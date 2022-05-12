# lab@home

## Was steckt dahinter?
Das Projekt "lab@home" entstand zu Beginn der Corona-Pandemie. Als Lehrer in einer Hochschule der angewandten Wissenschaften war es dem Initiator ein Anliegen, seinen Studierenden auch 
während des Lockdowns Zugang zu Laborexperimenten in den Bereichen Steuerungstechnik, Regelungstechnik und Informationstechnik zu geben. Leider konnten die Studierenden nicht
mehr in die Hochschullabors, also musste das Labor zu den Studierenden. Nur leider wäre ein Container voll Material pro Student etwas zu aufwändig. Etwas kleineres und vor allem
kostengünstigeres musste her. Und damit waren die grundsätzlichen Anforderungen umrissen und der Initiator machte mich ans Design...

## Was gehört zu lab@home und was kann man damit machen?
Lab@home besteht aus
- einem Hardware-Design
- einer Softwarekomponente
- dieser Dokumentation
- der Beschreibung von Laborexperimenten (nicht veröffentlicht, auf Anfrage stelle ich das gerne zur Verfügung)

Der Entwickler bezeichne lab@home als eine Lehr- und Lernplattform für den Einsatz in Bildungseinrichtungen in den Disziplinen Steuerungstechnik, Regelungstechnik und Informationstechnik.
Ein besonderer Fokus liegt dabei auf der Gebäudeautomation

## Was sind die einzigartigen Merkmale von lab@home?
Lab@home
- ist quelloffen und soll unter der MIT-Lizenz jedem Nutzen stiften!
- kann komplett über den Browser bedient werden   
- ist preiswert und unproblematisch nachbaubar
- kann vielfältig eingesetzt werden

## Genug des Blabla - bitte technisch konkret!
Die Hardware von lab@home ist auf einer 10x10cm großen Platine aufgebaut und enthält die folgenden Komponenten (teilweise optionale Bestückung)
- ESP32-Mikrocontroller (2x240MHz, 32bit, 16MB Flash, 512kb Ram, Wifi, Bluetooth)
- Lokale Benutzerschnittstelle: 2 Taster, 1 Drehencoder, 1 IPS-Display (SPI-Schnittstelle,240x240 Pixel)
- Sensoren
    - Temperatur, Feuchte, Druck, Qualität, CO2 Helligkeit der Umgebung (div. Sensoren, teilweise I2C, teilweise analog)
    - PIR-Bewegungsmelder
    - Digitales MEMS-Mikrofon
    - Drucksensor XGZP6897D
    - Beschleunigungssensor und Gyroskop MPU6050
    - Temperatursensor am Heizwiderstand
    - optional UWB DWM1000
- Aktoren
  - Akustischer Aktor: 1 Lautsprecher incl. 1W-Verstärker
  - Optischer Aktor: Vier einzeln ansteuerbare RGB-LED (WS2812), sechs dimmbare Hochleistungs-LED (warmweiß
  - Thermischer Aktor: Hochlastwiderstand als Heizwiderstand
  - Mechanischer Aktor: Lüfter 6010
- Experimentierfeld für verbindungsprogrammierte Steuerungen
  - alle Anschlüsse auf Pin-Header geführt
  - 2 Relais mit je zwei Wechselkontakten
  - 1 Zeitrelais (softwaregesteuert)
  - 2 LED
  - 2 Taster
- Kommunikation
  - Drahtgebunden: RS485 (für Modbus o.ä.)
  - Drahtlos: Wifi, Bluetooth, optional GSM-Modul, optional LoRaWan-Modul
- Erweiterungsanschlüsse
  - Anschluss für externes 5V/24V-Gerät, positiv PWM-geschaltet, Sense-Anschluss z.B. für Drehzahlerfassung
  - Anschluss für zwei Servos (aus dem Modellbau)
  - quiic-Anschluss
  - 0-10V-Ausgang
  - 0-10V-Eingang
  - Anschluss für externes BME280-Modul
  - Anschluss für externes CCS811-Modul
  - Anschluss für externes BH1750-Modul
  - Anschluss für Audio (3,5mm Buchse)
- Stromversorgung
  - USB-C PD bzw. PPS (20V, 2A)
  - Klinkenbuchse 5,5mm x 2,5mm (24V, 2A)
- Programmierung
  - USB-C
  - OTA

Die Software von lab@home besteht aus der eigentlichen Firmware und einer Webapplikation. Viele Experimente können über die Webapplikation gesteuert werden.
Nach dem ersten Start öffnet lab@home einen WIFI-Accesspoint, über den die Webapplikation erreichbar ist und labathome auch in ein bestehendes WIFI-Netzwerk eingebunden werden kann. Dank der Webapplikation benötigt der Anwender keine lokale Software und die Nutzung ist unabhängig vom Betriebssystem möglich (viele meiner Studierenden verwenden Apple-Rechner oder haben nur Chromebooks oder Android-Tablets). Die Webapplikation hat diese Features:
- Programmierung von lab@home mit Funktionsblöcken in Anlehnung an IEC61131 und VDI3814
  - darauf ist der Entwickler besonders stolz!!
  - Speichern und Laden von Programmen möglich
  - Es ist möglich, eine Startapplikation festzulegen, die lab@home automatisch beim Einschalten lädt und ausführt
  - Integrierter Debugger
  - Integrierter Simulator
  - Technisches Detail: Das Funktionsblock-Diagramm wird von einem Java-Script-Compiler im Browser zu einem Binärcode compiliert und "online" zum Mikrocontroller gesendet. Dieser bringt dann das Programm unmittelbar zur Ausführung
- Oberfläche für das Experiment "Identikation einer Regelstrecke mit dem Sprungantwort-Versuch"
- Oberfläche für das Experiment "Parametriertung eines PID-Reglers mit der Einstellregeln nach Chien, Hrones und Reswick"
- Konfigurationsoberfläche für Wifi
- Konfigurationsoberfläche für diverse Systemeinstellungen (in Arbeit)
- Online-Aktualisierung der kompletten Systemsoftware (in Arbeit)
