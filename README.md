# lab@home

<a target="_blank" href="https://icons8.com/icon/cUjTrEUoGJ2Z/xing">Xing</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
<a target="_blank" href="https://icons8.com/icon/3tC9EQumUAuq/github">GitHub</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
<a target="_blank" href="https://icons8.com/icon/IuI5Yd3J3qcC/linkedin">LinkedIn</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>

## Was steckt dahinter?
Das Projekt "lab@home" entstand zu Beginn der Corona-Pandemie. Als Lehrer in einer Hochschule der angewandten Wissenschaften war es dem Initiator ein Anliegen, seinen Studierenden auch 
während des Lockdowns Zugang zu Laborexperimenten in den Bereichen Steuerungstechnik, Regelungstechnik und Informationstechnik zu geben. Leider konnten die Studierenden nicht
mehr in die Hochschullabors, also musste das Labor zu den Studierenden. Nur leider wäre ein Container voll Material pro Student etwas zu aufwändig. Etwas kleineres und vor allem
kostengünstigeres musste her. Und damit waren die grundsätzlichen Anforderungen umrissen und der Initiator machte mich ans Design...

![Alt text](labathome_pcb/labathome_pcb.png?raw=true "Rendering")

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
Die Hardware von lab@home ist auf einer 10x10cm großen Platine aufgebaut und enthält die folgenden Komponenten:
- Dual-Prozessor-Konzept
    - Applikation: ESP32-S3 (8M Flash, 2,5MB RAM, Wifi, Bluetooth)
    - I/O & Realtime: STM32G431 (128k Flash, 32k RAM)
- Speichererweiteung über microSD-Slot
- Kommunikation
  - Interne Busse: I2c & 1Wire
  - Extern: Wifi, Bluetooth, RS485 (z.B. für Modbus), CAN (z.B. für CANopen oder OBD2)
  - Optional durch Steckmodule: Ethernet (W5500), GSM (SIM7080G), LoRaWan (RFM95), 2,4GHz (NRF24L01)
- Benutzerinteraktion
  - Taster, Inkrementalencoder
  - IPS-Display 240x240 Pixel, 1,3Zoll (ST7789)
  - Vier RGB-LED (WS2812)
  - Lautsprecher, Mikrofon, Klinkenbuchsen für Aux-In, Line-Out, Kopfhörer (NAU88C22)
- Sensorik
  - Lufttemperatur, Temperatur des Heizwiderstandes (2x DS18B20)
  - Luftfeuchtigkeit (SHT20)
  - Umgebungshelligkeit (LDR)
  - Ausrichtungs und Beschleunigungssensor (LSM6DS3)
  - PIR-Bewegungsmelder
  - Drei universelle ADC-Eingänge, davon einer mit Spannungsteiler
  - Spannungs- und Strommessung
  - Eingänge für Hall-Encoder
  - Optional durch Steckmodule: CO2, Luftqualität, Distanzsensor, Farbsensor
- Aktorik
  - Heizwiderstand, Lüfter, weiße Power-LEDs (alle PWM-Ansteuerung)
  - Zwei universelle DAC-Ausgänge, davon einer mit 0-10V
  - Dreiphasiger Motortreiber (Brushless) (DRV8313)
  - Drei Schrittmotor-Treiber (TMC2209)
  - Anschlüsse für vier (optional sechs) Servos
- Stromversorgung
  - USB
  - USB PD oder PPS (max 20V, 5A, umgesetzt über das UCPD-Peripherial im STM32G431)
  - Hohlstecker 5,5mm
  - MC-Klemmen
  - 18650-Akku
  - Involviert in die Stromversorgung sind 
    - IP5306: Akkuladung, Boot-Converter auf 5V
    - TPS54202: Synchroner Abwärtswandler 8...24V -> 5V
    - TPS2116: Umschaltung zwischen Spannungsquellen
    - AMS1117: LDO 5V->3,3V

![Alt text](doc/schematic_block_diagram.png?raw=true "Blockdiagramm")

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

# Getting Started - Hardware
Die aktuellen Schaltpläne und Layouts im KiCad-Format befinden sich im Verzeichnis labathome_pcb

# Getting Started - Software

Die Software muss individuell für jedes Board compiliert werden, weil während des von Gulp gesteuerten Gesamt-Build-Prozesses diverse individuelle Dateien erzeugt und eincompiliert werden. Dazu gehören:
- LabAtHome meKicldet sich per Sprachausgabe betriebsbereit. Die Ausgabe enthält auch den Hostnamen der Platine, der sich wiederum aus der MAC-Adresse ableitet. Alle Sprachausgaben werden zum Build-Zeitpunkt über Google TTS erzeugt und in Form von MP3-Dateien im Flash abgelegt. Ach ja, LabAtHome kann natürlich MP3 wiedergeben!
- LabAtHome kommuniziert natürlich verschlüsselt! Dazu wird ein privater Schlüssel und ein Zertifikat für eben den Hostnamen erzeugt


Selbst kompilieren? Geht!
- Installiere das ESP-IDF!
- Installiere Node.js
- Ich gehe davon aus, dass alle Checkouts in c:/repos erfolgen. Viele Pfade sind in den Konfigurationsdateien absolut und auf diesen Pfad angepasst
- Checke dieses Repo https://github.com/klaus-liebler/labathome aus
- Checke das repo https://github.com/klaus-liebler/espidf-components aus
- Rufe "npm update" in C:\repos\labathome\espidf-component-labathome\web auf
- Rufe "npm update" in "C:\repos\labathome\espidf-component-labathome\builder" auf
- Rume "npm run gulp" in "C:\repos\labathome\espidf-component-labathome\builder" auf. Dieses erstellt individuelle Board-Files und die gesamte Web-Anwendung
- TBC
  
