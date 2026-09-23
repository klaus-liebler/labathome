// Konfiguration ueber appsettings.json -> typsichere POCO-Klasse. BoardStorage/Certificates sind
// zentralisiert in FirmwareBuilder.Common (BoardsDirectoryOptions/CertificateAuthorityOptions) --
// derselbe verschachtelte Aufbau wie im STM32-Referenzprojekt (firmware_factory_control_unit/
// builder/BuilderSettings.cs). appsettings.json selbst ist gitignored (persoenliche
// Maschinenpfade), getrackt ist nur appsettings.json.template (s. builder.csproj,
// EnsureAppSettings-Target).
using FirmwareBuilder.Common;

namespace Builder;

public sealed class BuilderOptions : IBuilderAppSettings
{
	public BoardsDirectoryOptions BoardStorage { get; set; } = new();

	public CertificateAuthorityOptions Certificates { get; set; } = new();

	IBoardsDirectoryOptions IBuilderAppSettings.BoardStorage => BoardStorage;
	ICertificateAuthorityOptions IBuilderAppSettings.Certificates => Certificates;

	// Eigenstaendiges Repo (Remote klaus-liebler/npm-packages), von mehreren generierten npm-Projekten
	// per relativem "file:"-Pfad referenziert (s. AbstractBuildContext.RelativeFileDependency). Maschinenabhaengig
	// (jeder Entwickler kann seine Repos woanders auschecken) -- deshalb appsettings.json statt
	// hartkodiert in Paths.cs.
	public string NpmPackagesDir { get; set; } = "";

	// Zweite BestBinaryBuffers-Schema-Quelle: die anderen 10 Namespaces liegen im
	// Nachbar-Repo espidf-component-webmanager. Ebenfalls maschinenabhaengig.
	public string WebmanagerBestBinaryBuffersSchemaDir { get; set; } = "";

	// Google-Cloud-Service-Account-JSON fuer Text-to-Speech (relativ zum builder/-Verzeichnis oder absolut;
	// gitignored, s. .gitignore). Leer/fehlend => keine Sprachgenerierung, Platzhalter-Sound.
	public string GoogleTtsCredentialsFile { get; set; } = "";

	private static readonly Lazy<BuilderOptions> LazyCurrent = new(Load);

	public static BuilderOptions Current => LazyCurrent.Value;

	private static BuilderOptions Load() => BuilderAppSettings.LoadFromAppBase<BuilderOptions>();
}
