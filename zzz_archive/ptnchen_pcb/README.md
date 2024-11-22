# smopla_PTN2 - "Das kleine Peteh Ennchen"

Der kleine Bruder von lab@home. Ermöglicht das Experimentieren mit PT1...PTn-Gliedern, die intern als RC-Kombination (entkoppelt über Operationsverstärker) ausgeführt sind.
Features:
- Drei hintereinander geschaltete RC-Glieder, die bekanntlich PT1-Verhalten aufweisen; bilden eine Regelstrecke
- WIFI-fähiger Microcontroller (ESP32)
- Webbasierte Schnittstelle
- bald: Schnittstelle zu WinFACT Boris über WiFi
- Anregung über mechanischen Schalter oder softwaregesteurert über den Microcontroller
- Jedes PT1-Glied kann mit einstellbarer Intensität gestört werden
- Technisch: Anregungsspannung und Spannungen am Ausgang der RC-Glieder über Laborstecker herausgeführt und auf Analog-Digital-Wandler geführt
- Fachlich: Spannungen sind über externe Messinstrumente messbar oder stehen auf der Weboberfläche zur Verfügung oder stehen in einem BORIS-Funktionsbaustein zur Verfügung
- Softwareseitig kann der Regelkreis geöffnet oder geschlossen werden. Es stehen verschiedene Reglertypen und Parametersets zur Verfügung
- Die Verstärker zur Entkopplung der PT1-Glieder lassen sich mechanisch aktivieren und deaktivieren (dann keine Rückkopplungsfreiheit)
- Farbige LEDs zeigen den Zustand des Gesamtsystems und der einzelnen PT1-Glieder an
- Spannungsversorgung, Programmierung und bei Bedarf auch Kommunikation über USB-C

Ein Bild der Baugruppe sagt mehr, als tausend Worte

![Alt text](smopla_PTN2.png?raw=true "Rendering")
