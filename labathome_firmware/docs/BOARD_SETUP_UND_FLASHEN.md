# Board-Inbetriebnahme und Flashen

Kurzreferenz für den C#-Build-Orchestrator (`builder/`), der `idf.py`/`esptool` steuert und dabei
Board-Identität, Zertifikate, Sprachausgaben, Web-App und Firmware generiert. Hintergrund zur
Umstellung von Gulp auf diesen Builder: [MIGRATION_TO_BESTBINARYBUFFERS.md](MIGRATION_TO_BESTBINARYBUFFERS.md).

Alle Befehle unten werden im Verzeichnis `labathome_firmware/` ausgeführt (PowerShell oder
ESP-IDF-Konsole).

## Voraussetzungen (einmalig pro Rechner)

- .NET SDK 10 installiert (`dotnet --version`)
- ESP-IDF installiert; `IDF_PATH` zeigt auf das ESP-IDF-Installationsverzeichnis (das Verzeichnis,
  das `export.bat` enthält)
- **Die Konsole, in der `dotnet run` gestartet wird, muss eine bereits aktivierte
  ESP-IDF-Umgebung sein** (z.B. die vom ESP-IDF-Installer angelegte Verknüpfung
  "ESP-IDF X.Y PowerShell/CMD", oder manuell vorher `& "$Env:IDF_PATH\export.ps1"` aufrufen).
  Grund: `PrepareContextWithRealHardware` und `FlashFirmware` rufen `esptool`/`espefuse` direkt
  auf (ohne selbst `export.bat` dazwischenzuschalten) und brauchen sie daher schon auf `PATH`. Der
  Schritt `BuildFirmware` aktiviert die Umgebung zwar zusätzlich selbst noch einmal für seinen
  `idf.py`-Aufruf, das ersetzt aber nicht die aktivierte Konsole für die anderen Schritte.
- `builder/appsettings.json` existiert und ist korrekt ausgefüllt. Sie wird beim ersten
  `dotnet build`/`dotnet run` automatisch aus `builder/appsettings.json.template` angelegt
  (gitignored, weil rechnerspezifisch) und muss dann einmalig angepasst werden:
  - `BoardStorage:BoardsDir` -- Archiv-Verzeichnis, in dem pro Board (nach MAC) Zertifikate,
    Sprachausgabe und `board_info.json` abgelegt werden
  - `Certificates:CertsDir` -- Ablage für die selbstsignierte Root-CA (wird beim ersten Board
    automatisch erzeugt, danach wiederverwendet)
  - `NpmPackagesDir`, `WebmanagerBestBinaryBuffersSchemaDir` -- Pfade zu den Nachbar-Repos
    (s. Haupt-README, Abschnitt "Checke diverse Repositories aus")
  - `GoogleTtsCredentialsFile` -- optional; ohne gültige Google-TTS-Zugangsdaten wird für die
    Begrüßungsansage "ready.mp3" automatisch ein Platzhalter (`ok.mp3`) verwendet
- Board per USB angeschlossen, bevor ein Schritt mit Hardwarezugriff aufgerufen wird

## Neues Board in Betrieb nehmen

Ein "neues" Board ist eines, dessen MAC-Adresse noch nicht im Board-Archiv (`BoardsDir`) bekannt
ist. Einmaliger Komplettlauf über alle Schritte (Hardware erkennen und Board-Archiv anlegen,
Protokoll-/Runtime-Konfiguration generieren, Zertifikat erzeugen, Sprachausgabe erzeugen, Web-App
bauen, Firmware bauen, Firmware flashen):

```powershell
dotnet run --project builder -- Pipeline
```

Für die ältere Platinen-Revision mit klassischem ESP32 (Rev. 5.x) muss die Board-Version explizit
mitgegeben werden, da sonst der Default (Rev. 15.3, ESP32-S3) angenommen wird:

```powershell
dotnet run --project builder -- Pipeline --boardVersion 5.1
```

`--boardVersion` akzeptiert `5.1`, `5.1.0` oder die sechsstellig gepolsterte Zahl `050100`. Die
erste Ziffer wählt HAL-Verzeichnis (`main/hal/05` bzw. `main/hal/15`) und ESP-IDF-Target
(`esp32` bzw. `esp32s3`) -- Details dazu in [LabathomeBuildContext.cs](../builder/LabathomeBuildContext.cs).

Nach erfolgreichem Lauf zeigt die Konsole Board-Version, HAL-Verzeichnis, IDF-Target und
Board-Verzeichnis an. Sobald die Firmware startet, meldet sich das Board per Sprachausgabe
(inkl. Hostname) und ist über den angezeigten Hostnamen im Netz erreichbar.

## Bereits bekanntes Board (erneut) flashen

Board per USB anschließen (dieselbe Platine wie beim ersten Mal, also dieselbe MAC) und denselben
Befehl wie oben erneut aufrufen:

```powershell
dotnet run --project builder -- Pipeline
```

`PrepareContextWithRealHardware` erkennt das Board an seiner MAC im Archiv, aktualisiert nur den
Zeitstempel "zuletzt verbunden" und übernimmt automatisch die zuvor gespeicherte Board-Version --
`--boardVersion` wird für ein bereits bekanntes Board ignoriert (wirkt nur beim Neuanlegen).

### Nur neu flashen, ohne alles neu zu generieren

Wenn Zertifikat/Sounds/Web-App/Runtime-Config schon aktuell sind und sich nur der C++-Code
geändert hat, reicht:

```powershell
dotnet run --project builder -- BuildFirmware
dotnet run --project builder -- FlashFirmware
```

Diese beiden Schritte arbeiten mit dem zuletzt verwendeten Board aus dem `board_info.json` im
Repo-Root (angelegt/aktualisiert von `PrepareContextWithRealHardware`) -- ohne erneuten
Hardwarezugriff für `BuildFirmware`, aber `FlashFirmware` braucht das Board angeschlossen.

Optional kann beim Flashen die NVS-Partition (WLAN-/Usersettings) mitgelöscht werden:

```powershell
dotnet run --project builder -- FlashFirmware --resetNVSPartition
```

### Nur bauen, ohne Board (z.B. CI, oder Board gerade nicht angeschlossen)

```powershell
dotnet run --project builder -- PipelineBuildOnly
```

Baut für das zuletzt verwendete Board (aus `board_info.json`), ohne Hardwarezugriff und ohne zu
flashen.

## Weitere nützliche Einzelschritte

Alle Schritte lassen sich auch einzeln aufrufen (`dotnet run --project builder -- <Schritt>
[Argumente]`), z.B. zum gezielten Nacharbeiten:

| Schritt | Zweck |
| --- | --- |
| `Info` | Zeigt Board-Version, HAL-Verzeichnis, IDF-Target und (falls Board angeschlossen) ob die MAC des Live-Boards zum Archiv-Eintrag passt |
| `GitStatus` | Zeigt aktuellen Git-Branch/Commit/Dirty-Status |
| `SetBoardVersion --boardVersion <5.1\|15.3\|...>` | Ändert nachträglich die Hardware-Revision des zuletzt verwendeten Boards, ohne Hardwarezugriff |
| `GenerateBestBinaryBufferFiles` | Generiert C++/TypeScript aus den BestBinaryBuffers-Schemas |
| `GenerateRuntimeConfig` | Generiert Runtime-Defines (Hostname, Board-Infos, Zugangsdaten, ...) für C++, CMake und TypeScript |
| `GenerateCertificates` | Erzeugt (lazy) Root-CA und Board-Zertifikat |
| `GenerateSounds` | Erzeugt (lazy, per Google TTS) Sprachausgaben, inkl. Board-spezifischer Begrüßung |
| `BuildWebApp` | Baut die Web-Oberfläche |
| `BuildFirmware` | Ruft `idf.py build` für das aktuelle Board (Target/HAL/sdkconfig passend) auf |
| `FlashFirmware [--resetNVSPartition]` | Flasht die zuletzt gebaute Firmware per `esptool` |

**Hinweis Flash-Verschlüsselung:** Ist im Board-Archiv `flashEncryptionKeyBurnedAndActivated:
true` vermerkt, bricht `FlashFirmware` bewusst mit einer `NotImplementedException` ab -- der
verschlüsselte Flash-Pfad ist (noch) nicht implementiert.
