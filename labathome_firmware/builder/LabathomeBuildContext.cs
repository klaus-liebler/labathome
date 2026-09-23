using System.Text.Json;
using FirmwareBuilder.Common;
using FirmwareBuilder.Common.Esp32;

namespace Builder;

// Physische Labathome-Hardware-Revision. Die Board-Version steht als Zahl im Board-Archiv
// (board_info.json, "boardVersion", z.B. 50100 = Rev. 5.1.0, 150300 = Rev. 15.3.0). Wichtig ist nur
// die ERSTE Zahl (Major): sie waehlt das HAL-Verzeichnis main/hal/<Major zweistellig> und den
// ESP-IDF-Target-Chip. Die Zahl wird sechsstellig mit fuehrenden Nullen aufgefuellt
// (50100 -> "050100"), damit Major 5 zu "05" wird -- als reine Zahl ginge die fuehrende Null verloren.
public sealed record LabathomeHardware(long BoardVersion)
{
	public string Padded => BoardVersion.ToString("D6");
	public int Major => int.Parse(Padded[..2]);
	public int Minor => int.Parse(Padded[2..4]);
	public int Patch => int.Parse(Padded[4..6]);
	public string HalDirectoryName => Padded[..2];
	public string SemVer => $"{Major}.{Minor}.{Patch}";

	// Rev. 5.x ist die alte Platine mit dem klassischen ESP32 (WROOM/WROVER, kein USB-OTG, 4 MB Flash);
	// alle neueren Revisionen nutzen den ESP32-S3.
	public string IdfTarget => Major <= 5 ? "esp32" : "esp32s3";
}

// Implementiert IBuildContextEsp32 auf Basis von AbstractBuildContext (geteilte Lib), analog zu
// SensactBuildContext. Board-Identitaets-Properties lesen bei JEDEM Zugriff frisch von der Platte.
public sealed class LabathomeBuildContext : AbstractBuildContext, IBuildContextEsp32
{
	public const string BoardTypeNameConst = "LABATHOME";
	public const long DefaultBoardVersion = 150300;

	// Projektlokales Ausgabeverzeichnis fuer generierte Dateien (wsprotocol_cpp/_ts, runtimeconfig_cpp/_ts,
	// cmake/, web/). Zuvor das repo-fremde c:\repos\generated.
	public static readonly string GeneratedRoot = Path.Combine(RootDirStatic, "generated");
	public static readonly string GeneratedWsProtocolCppDir = Path.Combine(GeneratedRoot, "wsprotocol_cpp");
	public static readonly string GeneratedWsProtocolTsDir = Path.Combine(GeneratedRoot, "wsprotocol_ts");
	public static readonly string GeneratedRuntimeConfigCppDir = Path.Combine(GeneratedRoot, "runtimeconfig_cpp");
	public static readonly string GeneratedRuntimeConfigTsDir = Path.Combine(GeneratedRoot, "runtimeconfig_ts");
	public static readonly string GeneratedCMakeDir = Path.Combine(GeneratedRoot, "cmake");
	private static readonly string _generatedWebDir = Path.Combine(GeneratedRoot, "web");

	// Wiederverwendbare, im Repo liegende Sounds (nicht board-spezifisch).
	public static readonly string SoundsDir = Path.Combine(RootDirStatic, "sounds");

	public LabathomeBuildContext(string[] args) : base(args)
	{
	}

	protected override IBuilderAppSettings Settings => BuilderOptions.Current;

	public string NpmPackagesDir => BuilderOptions.Current.NpmPackagesDir;
	public string WebmanagerBestBinaryBuffersSchemaDir => BuilderOptions.Current.WebmanagerBestBinaryBuffersSchemaDir;

	public override string WebGeneratedDir => _generatedWebDir;
	public override string FirmwareGeneratedDir => GeneratedRoot;

	public string IdfPath
	{
		get
		{
			var idfPath = Environment.GetEnvironmentVariable("IDF_PATH");
			if (string.IsNullOrWhiteSpace(idfPath))
			{
				throw new InvalidOperationException("IDF_PATH ist nicht gesetzt -- muss auf die ESP-IDF-Installationswurzel zeigen (enthaelt export.bat).");
			}
			if (!File.Exists(Path.Combine(idfPath, "export.bat")))
			{
				throw new InvalidOperationException($"IDF_PATH ist auf \"{idfPath}\" gesetzt, aber \"export.bat\" existiert dort nicht -- IDF_PATH korrigieren.");
			}
			return idfPath;
		}
	}

	public BoardRecord Board => LoadBoardRecord(BoardInfoJsonPath);

	public static BoardRecord LoadBoardRecord(string path)
	{
		if (!File.Exists(path))
		{
			throw new InvalidOperationException(
				$"Keine board_info.json unter {path} -- noch kein Board verbunden gewesen (s. PrepareContextWithRealHardware).");
		}
		return JsonSerializer.Deserialize<BoardRecord>(File.ReadAllText(path))
			?? throw new InvalidOperationException($"{path} konnte nicht gelesen werden.");
	}

	private long BoardMac => Board.Mac ?? throw new InvalidOperationException($"{BoardInfoJsonPath}: Feld \"mac\" fehlt.");

	public LabathomeHardware Hardware => new(Board.BoardVersion ?? DefaultBoardVersion);

	public override ChipId ChipId => ChipId.FromEsp32Mac48(BoardMac);
	public override string BoardUid => BoardPaths.BoardDirectoryName(BoardMac);
	public override string BoardArchiveDir => BoardArchiveContext.BoardArchiveDir(BuilderOptions.Current.BoardStorage, BoardUid);

	public override string? WebAdminPassword => Board.WebAdminPassword;
	public override IReadOnlyDictionary<string, string> BoardSettings => Board.BoardSettings ?? [];
	public bool FlashEncryptionKeyBurnedAndActivated => Board.FlashEncryptionKeyBurnedAndActivated ?? false;

	public override string BoardTypeName => BoardTypeNameConst;
	public override string BoardTypeVersion => Hardware.SemVer;

	public override string Hostname => BoardSettings.GetValueOrDefault(BoardSettingsKeys.OverrideHostname) is { Length: > 0 } name
		? name
		: $"labathome_{BoardPaths.Mac6Char(BoardMac)}";

	// --- ESP-IDF-Target-abhaengige Pfade ---
	// Der S3 behaelt die historischen Namen (build/, sdkconfig, partitions.csv), damit die
	// VS-Code-ESP-IDF-Erweiterung und "idf.py" ohne Builder unveraendert weiter funktionieren. Jedes
	// andere Target bekommt ein eigenes Build-Verzeichnis und eigene sdkconfig, damit ein Wechsel
	// zwischen Boards keinen Full-Clean/set-target erzwingt.
	public string IdfTarget => Hardware.IdfTarget;
	private string Suffix => IdfTarget == "esp32s3" ? "" : "_" + IdfTarget;
	public string BuildDir => Path.Combine(RootDir, "build" + Suffix);
	public string SdkconfigPath => Path.Combine(RootDir, IdfTarget == "esp32s3" ? "sdkconfig" : $"sdkconfig.{IdfTarget}");
	public string PartitionsCsvPath => Path.Combine(RootDir, IdfTarget == "esp32s3" ? "partitions.csv" : $"partitions_{IdfTarget}.csv");

	// --- Board-Archiv (genutzt von PrepareContextWithRealHardware) ---
	public static BoardRecord NewDefaultBoardRecord(long mac, long boardVersion)
	{
		var now = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
		return new BoardRecord(
			FirstConnectedAtEpoch: now,
			LastConnectedAtEpoch: now,
			WebAdminPassword: null,
			BoardSettings: [],
			Mac: mac,
			BoardVersion: boardVersion,
			FlashEncryptionKeyBurnedAndActivated: false);
	}

	// Akzeptiert "50100", "050100" oder "5.1"/"5.1.0".
	public static long ParseBoardVersion(string text)
	{
		text = text.Trim();
		if (text.Contains('.'))
		{
			var parts = text.Split('.').Select(int.Parse).ToArray();
			if (parts.Length is < 1 or > 3 || parts.Any(p => p is < 0 or > 99))
			{
				throw new ArgumentException($"Ungueltige Board-Version \"{text}\" (erwartet Major[.Minor[.Patch]], je 0..99).");
			}
			return parts[0] * 10000L + (parts.Length > 1 ? parts[1] : 0) * 100L + (parts.Length > 2 ? parts[2] : 0);
		}
		return long.Parse(text);
	}

	public static void SaveBoardRecord(string path, BoardRecord record) =>
		File.WriteAllText(path, JsonSerializer.Serialize(record, new JsonSerializerOptions { WriteIndented = true }));
}
