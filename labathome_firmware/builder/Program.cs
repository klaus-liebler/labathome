using FirmwareBuilder.Common;
using FirmwareBuilder.Common.Esp32;

namespace Builder;

internal static class Program
{
	private const string AppName = "labathome";
	private const string AppVersion = "1.0";

	public static void Main(string[] args) =>
		BuildStepRunner.Run(args, a => new LabathomeBuildContext(a), typeof(Program));

	[BuildStep]
	public static void Info(IBuildContextEsp32 ctx) =>
		Esp32ConsoleReport.WriteBoardInfo(ctx, "board_info.json", () =>
		{
			try
			{
				var hw = ((LabathomeBuildContext)ctx).Hardware;
				return [("Board version", $"{hw.Padded} (Rev. {hw.SemVer})"), ("HAL directory", "main/hal/" + hw.HalDirectoryName), ("IDF target", hw.IdfTarget)];
			}
			catch (Exception ex)
			{
				return [("Board version", $"(unbekannt -- {ex.Message})")];
			}
		});

	[BuildStep]
	public static void GitStatus(IBuildContextEsp32 ctx) => BuilderConsoleReport.WriteGitStatus(ctx.Git);

	// Liest die echte MAC vom angeschlossenen Board und legt bei einem NEUEN Board das Board-Archiv an.
	// Die Hardware-Revision eines neuen Boards kommt aus "--boardVersion <50100|5.1>" (Default: 150300).
	// Fuer ein bereits bekanntes Board s. SetBoardVersion.
	[BuildStep]
	public static void PrepareContextWithRealHardware(IBuildContextEsp32 ctx)
	{
		var version = Cli.GetOptionalArgValue(ctx.Args, "--boardVersion") is { } v
			? LabathomeBuildContext.ParseBoardVersion(v)
			: LabathomeBuildContext.DefaultBoardVersion;
		Esp32BoardProvisioningService.PrepareContextWithRealHardware(
			ctx,
			createDefaultRecord: mac => LabathomeBuildContext.NewDefaultBoardRecord(mac, version),
			onBoardArchiveReady: recordPath => File.Copy(recordPath, ctx.BoardInfoJsonPath, overwrite: true));
	}

	// Aendert die Hardware-Revision des aktuellen Boards: SetBoardVersion --boardVersion 5.1
	[BuildStep]
	public static void SetBoardVersion(IBuildContextEsp32 ctx)
	{
		var version = LabathomeBuildContext.ParseBoardVersion(Cli.GetRequiredArgValue(ctx.Args, "--boardVersion"));
		var archiveRecord = Path.Combine(ctx.BoardArchiveDir, "board_info.json");
		var board = LabathomeBuildContext.LoadBoardRecord(archiveRecord) with { BoardVersion = version };
		LabathomeBuildContext.SaveBoardRecord(archiveRecord, board);
		File.Copy(archiveRecord, ctx.BoardInfoJsonPath, overwrite: true);
		var hw = new LabathomeHardware(version);
		Console.WriteLine($"Board-Version {hw.Padded} (Rev. {hw.SemVer}) -> HAL main/hal/{hw.HalDirectoryName}, IDF-Target {hw.IdfTarget}.");
	}

	[BuildStep]
	public static void GenerateBestBinaryBufferFiles(IBuildContextEsp32 ctx) => WsProtocolBuildService.Generate(
		ctx,
		sourceDirs: [ctx.BestBinaryBuffersSchemaDir, ctx.WebmanagerBestBinaryBuffersSchemaDir],
		cppOutputDir: LabathomeBuildContext.GeneratedWsProtocolCppDir,
		tsOutputDir: LabathomeBuildContext.GeneratedWsProtocolTsDir,
		tsPackageName: "@generated/wsprotocol_ts");

	[BuildStep]
	public static void GenerateRuntimeConfig(IBuildContextEsp32 ctx)
	{
		var lctx = (LabathomeBuildContext)ctx;
		var hw = lctx.Hardware;
		var defines = new Dictionary<string, object>();
		var clientDefines = new Dictionary<string, object>();
		var now = DateTimeOffset.UtcNow;

		void SetShared(string key, object value)
		{
			defines[key] = value;
			clientDefines[key] = value;
		}

		defines["WEBMANAGER_AUTH_USERNAME"] = "admin";
		defines["WEBMANAGER_AUTH_PASSWORD"] = string.IsNullOrEmpty(ctx.WebAdminPassword)
			? $"{ctx.Hostname}_admin".ToLowerInvariant()
			: ctx.WebAdminPassword;

		SetShared("HOSTNAME", ctx.Hostname);
		SetShared("BOARD_NAME", ctx.BoardTypeName);
		SetShared("BOARD_VERSION", hw.BoardVersion);
		SetShared("BOARD_ROLES", "");
		SetShared("BOARD_MAC", ctx.ChipId.ToEsp32Mac48());
		SetShared("APP_NAME", AppName);
		SetShared("APP_VERSION", AppVersion);
		SetShared("CREATION_DT", now.ToUnixTimeSeconds());
		SetShared("CREATION_DT_STR", now.ToLocalTime().ToString("yyyy-MM-dd HH:mm:ss"));
		SetShared("GIT_SHORT_HASH", ctx.Git.CommitHash);
		SetShared("BANNER", AppName);

		// Nur fuer CMake (main/CMakeLists.txt): HAL-Verzeichnis und Pfade ausserhalb des Repos.
		defines["BOARD_HAL_DIR"] = hw.HalDirectoryName;
		defines["BOARD_DIRECTORY"] = ctx.BoardArchiveDir.Replace('\\', '/');
		defines["CERTS_DIR"] = ctx.CertsDir.Replace('\\', '/');

		RuntimeConfigWriter.CreateCppConfigurationHeader(LabathomeBuildContext.GeneratedRuntimeConfigCppDir, defines);
		RuntimeConfigWriter.CreateCMakeJsonConfigFile(LabathomeBuildContext.GeneratedCMakeDir, defines);
		RuntimeConfigWriter.CreateTypeScriptRuntimeConfig(LabathomeBuildContext.GeneratedRuntimeConfigTsDir, clientDefines);

		Console.WriteLine($"{defines.Count} Defines -> {LabathomeBuildContext.GeneratedRuntimeConfigCppDir}, {LabathomeBuildContext.GeneratedCMakeDir}; {clientDefines.Count} Defines -> {LabathomeBuildContext.GeneratedRuntimeConfigTsDir}");
	}

	[BuildStep]
	public static void GenerateCertificates(IBuildContextEsp32 ctx) => BoardCertificateService.EnsureBoardCertificate(
		ctx,
		rootCaCommonName: BuilderOptions.Current.Certificates.RootCaCommonName,
		hostname: ctx.Hostname,
		outputFileBaseName: "esp32");

	// Sprachausgabe per Google Cloud Text-to-Speech (Service-Account-JSON, s. appsettings.json ->
	// GoogleTtsCredentialsFile). Erzeugt nur fehlende Dateien: die allgemeinen Saetze nach sounds/de/, die
	// board-spezifische Begruessung "ready.mp3" (mit der MAC im Text) im Board-Archiv. Ohne Zugangsdaten
	// oder bei einem TTS-Fehler wird fuer ein fehlendes ready.mp3 das allgemeine "ok.mp3" als Platzhalter
	// verwendet (und beim naechsten Lauf durch die echte Ansage ersetzt).
	[BuildStep]
	public static void GenerateSounds(IBuildContextEsp32 ctx)
	{
		var mac6 = BoardPaths.Mac6Char(ctx.ChipId.ToEsp32Mac48());
		var readyDir = Path.Combine(ctx.BoardArchiveDir, "sounds", "de");
		var ready = Path.Combine(readyDir, "ready.mp3");
		var placeholderMarker = ready + ".placeholder";

		var credentials = BuilderOptions.Current.GoogleTtsCredentialsFile;
		if (!string.IsNullOrWhiteSpace(credentials) && !Path.IsPathRooted(credentials))
		{
			credentials = Path.Combine(ctx.RootDir, "builder", credentials);
		}
		var haveCredentials = !string.IsNullOrWhiteSpace(credentials) && File.Exists(credentials);

		// Ein frueher erzeugter Platzhalter darf die echte Ansage nicht blockieren
		if (File.Exists(placeholderMarker) && haveCredentials)
		{
			File.Delete(ready);
			File.Delete(placeholderMarker);
		}

		if (haveCredentials)
		{
			try
			{
				GoogleTextToSpeechService.SynthesizeMissing(credentials, CommonSentencesDe, Path.Combine(LabathomeBuildContext.SoundsDir, "de"));
				GoogleTextToSpeechService.SynthesizeMissing(credentials,
					[new SpeechSentence("ready", $"<speak>Willkommen! <lang xml:lang='en-US'>Lab@Home</lang><say-as interpret-as='characters'>{mac6}</say-as> ist bereit</speak>")],
					readyDir);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"WARNUNG: Sprachgenerierung fehlgeschlagen: {ex.Message}");
			}
		}
		else
		{
			Console.WriteLine("Keine Google-TTS-Zugangsdaten konfiguriert (appsettings.json: GoogleTtsCredentialsFile) -- keine Sprachgenerierung.");
		}

		if (!File.Exists(ready))
		{
			Directory.CreateDirectory(readyDir);
			File.Copy(Path.Combine(LabathomeBuildContext.SoundsDir, "de", "ok.mp3"), ready);
			File.WriteAllText(placeholderMarker, "ready.mp3 ist nur ein Platzhalter (ok.mp3); wird bei verfuegbarem TTS ersetzt.");
			Console.WriteLine($"WARNUNG: kein board-spezifisches ready.mp3 -- Platzhalter ok.mp3 nach {ready} kopiert.");
		}
	}

	private static readonly SpeechSentence[] CommonSentencesDe =
	[
		new("resistor_hot", "<speak>Achtung! Der Heizwiderstand wird zu heiß!</speak>"),
		new("boring", "<speak>Mir wird langweilig</speak>"),
		new("ok", "<speak>Alles ok</speak>"),
		new("nok", "<speak>Nicht ok</speak>"),
		new("alarm_co2", "<speak>CO2 Alarm</speak>"),
		new("alarm_temperature", "<speak>Temperaturalarm</speak>"),
	];

	[BuildStep]
	public static void BuildWebApp(IBuildContextEsp32 ctx) => WebAppBuildService.Run(
		ctx,
		["--outDir", ctx.WebGeneratedDir, "--sourcemap", "true", "--emptyOutDir"],
		Path.Combine(ctx.WebGeneratedDir, "index.compressed.br"));

	// idf.py mit eigenem Build-Verzeichnis + eigener sdkconfig je Target (s. LabathomeBuildContext.BuildDir).
	// -DIDF_TARGET wird immer mitgegeben, weil sdkconfig.defaults bewusst KEIN CONFIG_IDF_TARGET enthaelt
	// (target-spezifisches steht in sdkconfig.defaults.<target>, das ESP-IDF automatisch dazunimmt).
	[BuildStep]
	public static void BuildFirmware(IBuildContextEsp32 ctx)
	{
		var lctx = (LabathomeBuildContext)ctx;
		var exportBat = Path.Combine(ctx.IdfPath, "export.bat");
		var cmd = $"\"{exportBat}\" && idf.py -B \"{lctx.BuildDir}\" -DIDF_TARGET={lctx.IdfTarget} -DSDKCONFIG=\"{lctx.SdkconfigPath}\" build";
		Console.WriteLine($"Baue Firmware fuer {lctx.IdfTarget} (HAL {lctx.Hardware.HalDirectoryName}) -> {lctx.BuildDir}");
		ProcessRunner.RunInheritShellCommand(cmd, ctx.RootDir);
		Console.WriteLine("Firmware gebaut.");
	}

	[BuildStep]
	public static void FlashFirmware(IBuildContextEsp32 ctx) => Esp32FlashService.Run(ctx);

	[BuildStep]
	public static void Pipeline(IBuildContextEsp32 ctx)
	{
		BuildStepRunner.Invoke(ctx, PrepareContextWithRealHardware);
		BuildStepRunner.Invoke(ctx, GenerateBestBinaryBufferFiles);
		BuildStepRunner.Invoke(ctx, GenerateRuntimeConfig);
		BuildStepRunner.Invoke(ctx, GenerateCertificates);
		BuildStepRunner.Invoke(ctx, GenerateSounds);
		BuildStepRunner.Invoke(ctx, BuildWebApp);
		BuildStepRunner.Invoke(ctx, BuildFirmware);
		BuildStepRunner.Invoke(ctx, FlashFirmware);
	}

	// Wie Pipeline, aber fuer das zuletzt verwendete Board (board_info.json im Repo-Root) und ohne
	// Hardwarezugriff/Flashen.
	[BuildStep]
	public static void PipelineBuildOnly(IBuildContextEsp32 ctx)
	{
		BuildStepRunner.Invoke(ctx, GenerateBestBinaryBufferFiles);
		BuildStepRunner.Invoke(ctx, GenerateRuntimeConfig);
		BuildStepRunner.Invoke(ctx, GenerateCertificates);
		BuildStepRunner.Invoke(ctx, GenerateSounds);
		BuildStepRunner.Invoke(ctx, BuildWebApp);
		BuildStepRunner.Invoke(ctx, BuildFirmware);
	}
}
