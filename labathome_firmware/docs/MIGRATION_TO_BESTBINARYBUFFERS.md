# Migration auf BestBinaryBuffers + C#-Build-System

**Status: noch nicht begonnen.** Dieses Dokument beschreibt, was in `sensact_firmware`
bereits passiert ist, warum das hier relevant ist, und was konkret zu tun ist, um
`labathome_firmware` auf denselben Stand zu bringen.

## Warum das jetzt dringend ist

`labathome_firmware` bindet `espidf-component-webmanager` als geteilte Komponente
ein (`CMakeLists.txt`: `EXTRA_COMPONENT_DIRS "../../espidf-component-webmanager"`,
zeigt auf `C:\repos\espidf-component-webmanager`). Dieselbe Komponente wird auch von
`sensact_firmware` genutzt. Dort wurde sie im August 2026 komplett von Flatbuffers auf
[BestBinaryBuffers](https://github.com/klaus-liebler/best_binary_buffers) umgestellt
(eigenes Repo, Schema als annotierter C#-Code statt `.fbs`) -- **inklusive einer
Breaking Change am Plugin-Interface selbst**. Das heisst: der aktuelle Checkout von
`espidf-component-webmanager` kompiliert mit `labathome_firmware`s Plugins (Stand
dieses Dokuments) **nicht mehr**.

Ausserdem: die generierten Flatbuffers-Header, auf die `labathome_firmware`s Plugins
sich beziehen (`generated/flatbuffers_cpp/ns03functionblock_generated.h`,
`ns04heaterexperiment_generated.h`, sowie client-seitig `@generated/flatbuffers_ts`),
wurden im Zuge dieser Migration aus `C:\repos\generated\` **geloescht** (waren in
`sensact_firmware` bereits vollstaendig ungenutzt). Ein Build von `labathome_firmware`
schlaegt dadurch aktuell schon beim Includieren fehl, nicht erst beim Linken gegen die
neue Interface-Signatur.

## Was sich am Plugin-Interface geaendert hat

`espidf-component-webmanager/cpp/webmanager_interfaces.hh`:

```diff
- virtual esp_err_t WrapAndSendAsync(uint32_t namespaceId, flatbuffers::FlatBufferBuilder& b) = 0;
+ virtual esp_err_t SendRawAsync(const uint8_t* data, size_t len) = 0;
```

```diff
- virtual eMessageReceiverResult ProvideWebsocketMessage(iWebmanagerCallback *callback,
-     httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t *buf) = 0;
+ virtual eMessageReceiverResult ProvideWebsocketMessage(iWebmanagerCallback *callback,
+     httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint16_t namespaceId, uint16_t messageTypeId,
+     const uint8_t *frame, size_t frameLen) = 0;
```

`frame` zeigt jetzt auf den **kompletten** Frame inkl. 4-Byte-Kopf (2x `uint16`:
`namespaceId`, `messageTypeId`) -- kein Vorab-Slicing mehr durch den Dispatcher. Die
generierten `WsProtocol::<namespace>::<Message>::Decode(frame, frameLen, out)`-
Funktionen ueberspringen die ersten 4 Bytes selbst.

## Konkret zu aendernde Dateien in diesem Repo

- **`main/webmanager_plugins/functionblock_plugin.hh`** und
  **`heaterexperiment_plugin.hh`**: `ProvideWebsocketMessage`-Signatur anpassen (s.o.),
  `#include "flatbuffers/flatbuffers.h"` + `#include "../generated/flatbuffers_cpp/..."`
  durch `#include "wsprotocol_cpp/ws_protocol.hh"` ersetzen,
  `flatbuffers::GetRoot<...>`/`request_type()`-Switch durch
  `if (namespaceId != WsProtocol::<ns>::NAMESPACE_ID) return NOT_FOR_ME;` +
  `switch (messageTypeId)` gegen die generierten `TYPE_ID`-Konstanten ersetzen,
  `callback->WrapAndSendAsync(ns, b)` durch `WsProtocol::<ns>::<Response>::Payload`
  befuellen + `Encode(...)` in einen Stack-Puffer + `callback->SendRawAsync(buf, len)`
  ersetzen. Die Schemas fuer beide Namespaces existieren **bereits** (wurden im Zuge
  der sensact-Migration mit generiert, auch wenn sensact selbst keinen Server dafuer
  hat): `C:\repos\espidf-component-webmanager\ws-protocol\functionblock.cs` /
  `heaterexperiment.cs`. `RequestFbdRun`/`ResponseFbdRun`/`RequestDebugData`/
  `ResponseDebugData` bzw. `RequestHeater`/`ResponseHeater` sind 1:1 gegen die
  bisherigen Flatbuffers-Felder benannt -- die Migration ist reines Feld-fuer-Feld-
  Nachvollziehen, keine Schema-Neugestaltung noetig. Referenzimplementierung fuer genau
  dieses Muster (Request/Response, kein Array/Union): `systeminfo_plugin.hh` in
  `espidf-component-webmanager/cpp/webmanager_plugins/` (bereits migriert).
  `ResponseDebugData`s `Bools`/`Integers`/`Floats`/`Colors`-Arrays sind im neuen Schema
  `UniformPackedArrayField`s aus Einzelfeld-Wrapper-Structs (`BoolValue{Value:bool}`
  usw., s. Kommentar in `functionblock.cs`) -- Referenz fuer das Codegen-Muster:
  `AppendResponseSystemDataPartitionsPartitionInfoElement` in `systeminfo_plugin.hh`
  (analoges Array-Feld, andere Elementstruktur).
- **`main/main.cc`**: `webmanager::M::GetSingleton()->Begin(...)`-Aufruf pruefen -- die
  Begin-Signatur hat in sensact_firmware neue optionale Parameter bekommen
  (`apFallbackTimeout_us`, `auth_username`/`auth_password`), s.
  `C:\repos\sensact\firmware\sensact_firmware\main\main.cc` als Referenz.
- **`web/`**: `@klaus-liebler/web-components` importiert(e) `Flowchart.ts`,
  `develop_cfc_controller.ts` (functionblock) und `heater_experiment_controller.ts`
  (heaterexperiment) aus `@generated/flatbuffers_ts/functionblock` bzw.
  `/heaterexperiment`. **Diese drei Dateien wurden im Zuge dieser Aufraeumaktion aus
  `web-components` entfernt** (waren nur fuer labathome_firmware relevant, s. dortiges
  `docs/plan_v2/03-wifimanager-review.md`, Abschnitt "ws-protocol-Migration" ->
  "Uebergangsregel"). Fuer `labathome_firmware`s eigenen `web/`-Client heisst das: die
  alten Dateiversionen aus der Git-Historie von `npm-packages`
  (`@klaus-liebler/web-components`) holen (letzter Commit, der sie noch enthielt, vor
  ihrer Loeschung) und als **lokale** Kopien in `labathome_firmware/web/` uebernehmen
  (nicht mehr aus dem geteilten Paket importieren), dann auf
  `@generated/wsprotocol_ts/ws-protocol.ts` umstellen -- Transport-Interface
  (`IMessageListener`/`IAppManagement.SendFrame`/`RegisterNamespace`) in
  `web-components/typescript/utils/interfaces.ts` ist bereits auf den neuen,
  einheitlichen Mechanismus umgestellt (kein Flatbuffers-Praefix-Bau mehr durch
  `AppController` selbst).

## Empfehlung: C#-Build-System uebernehmen

Neben der reinen Protokoll-Migration lohnt es sich, gleich den alten Node/Gulp-
Orchestrator (`builder/gulpfile.ts`) durch das neue C#-Pendant zu ersetzen -- in
`sensact_firmware` als `builder/` (vormals `builder_cs/`, dort umbenannt sobald der
alte TS-Orchestrator obsolet war). Vorteile/Vorgehen: s.
`C:\repos\sensact\firmware\sensact_firmware\docs\plan_v2\02-builder-migration-csharp.md`
fuer die Migrationsentscheidung, `builder/Program.cs` fuer die Phasen-CLI
(`GenerateWsProtocolFiles`, `BuildFirmware`, `FlashFirmware`, ...),
`builder/Phases/GenerateWsProtocolFiles.cs` fuer die Codegen-Phase, die auch hier
direkt wiederverwendbar sein sollte (liest `*.cs`-Schema-Dateien aus einem oder
mehreren `ws-protocol/`-Verzeichnissen -- fuer labathome waere das vermutlich
`labathome_firmware/ws-protocol/` + weiterhin `espidf-component-webmanager/ws-protocol/`
fuer die geteilten Namespaces).

## Wo generierte Dateien jetzt liegen

`sensact_firmware` schreibt seine generierten Dateien projektlokal nach
`sensact_firmware/generated/` (nicht mehr nach `C:\repos\generated\`, s. dortiges
`builder/Paths.cs`). `labathome_firmware`s `CMakeLists.txt` (`GENERATED_DIR`) und
`web/package.json` (`file:`-Abhaengigkeiten auf `@generated/...`) zeigen aktuell noch
auf `C:\repos\generated\` -- das existiert vorerst fuer die von `labathome_firmware`
noch benoetigten Teile weiter (`cmake/config.json`, `web/index.compressed.br`), sollte
aber im Zuge dieser Migration ebenfalls auf einen projektlokalen
`labathome_firmware/generated/`-Ordner umgestellt werden (gleiches Muster wie in
`sensact_firmware`).
