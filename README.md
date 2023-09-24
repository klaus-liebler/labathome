# lab@home

<a target="_blank" href="https://icons8.com/icon/cUjTrEUoGJ2Z/xing">Xing</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
<a target="_blank" href="https://icons8.com/icon/3tC9EQumUAuq/github">GitHub</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>
<a target="_blank" href="https://icons8.com/icon/IuI5Yd3J3qcC/linkedin">LinkedIn</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>

## Was steckt dahinter?
Das Projekt "lab@home" entstand zu Beginn der Corona-Pandemie. Als Lehrer in einer Hochschule der angewandten Wissenschaften war es dem Initiator ein Anliegen, seinen Studierenden auch 
während des Lockdowns Zugang zu Laborexperimenten in den Bereichen Steuerungstechnik, Regelungstechnik und Informationstechnik zu geben. Leider konnten die Studierenden nicht
mehr in die Hochschullabors, also musste das Labor zu den Studierenden. Nur leider wäre ein Container voll Material pro Student etwas zu aufwändig. Etwas kleineres und vor allem
kostengünstigeres musste her. Und damit waren die grundsätzlichen Anforderungen umrissen und der Initiator machte mich ans Design...

![Alt text](labathome_pcb/pcbV10.png?raw=true "Rendering")

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

# Getting Started - Hardware
Die aktuellen Schaltpläne und Layouts im KiCad-Format befinden sich im Verzeichnis pcbV10 (neu) bzw pcb (alt). Die Hardware wurde so designed, dass sie bei JLCPBC kostengünstig gefertigt werden kann.
Im Verzeichnis pcb-frontplate gibt es ein KiCad-Projekt für eine Frontplattendesign, das als 10x10cm große Platine (mit vielen Fräsungen und Bedruckungen, aber komplett ohne Leiterbahnen) sehr preiswert bei JLCPCB mitbestellt werden kann.
Bei Bedarf stelle ich ein Aufbauvideo zur Verfügung, das die Montage der THT-Bauteile und der mechanischen Bauteile zeigt.
Nach dem Aufbau ist darauf zu achten, dass die sechs Schiebeschalter auf eine sinnvolle Position gestellt werden.
- "5V FAN 24V": Stellt ein, ob die externen Anschlüsse FAN1 und FAN2 mit 5V oder mit 20/24V versorgt werden (Empfehlung: zur Sicherheit auf "5V)
- "TERM ON" in Richtung "ON": Stellt ein, ob die Terminierung des RS485 ein- oder ausgeschaltet ist.
- "PROG 5V POWER": Stellt ein, ob die 5V-Schiene der Platine (aus der letztlich die komplette Elektronik versorgt wird) aus dem USB-C-Programmieranschluss oder dem USB-C-PD-Power-Anschluss versorgt werden (Empfehlung "PROG")
- "SNS MOV": Stellt ein, ob ein gemeinsam genutzter Pin mit dem PIR-Movement-Sensor oder dem Sensor-Signal "FS" des Fan1-Anschlusses verbunden ist (Empfehlung in Richung "MOV")
- "LED HTR": Stellt ein, ob ein gemeinsam genutzter Pin den Heizwiderstand (HTR) oder die Hochleistungs-LED ansteuert.
- "REL_ROT": Stellt ein, ob ein gemeinsam genutzter Pin mit dem A1-Anschluss des Zeitrelais oder mit dem Rotary-Encoder verbunden ist (Empfehlung in Richtung ROT)

# Getting Started - Software compilieren
Selbst kompilieren? Geht!
- ESP-IDF installieren
- Ich gehe davon aus, dass alle Checkouts in c:/repos erfolgen. Viele Pfade sind in den Konfigurationsdateien absolut und auf diesen Pfad angepasst
- Dieses repo https://github.com/klaus-liebler/labathome auschecken
- Das repo https://github.com/klaus-liebler/espidf-components auschecken
- Das repo https://github.com/klaus-liebler/wifimanager_webui auschecken

- Im Verzeichnis c:/repos/labathome/labathome_webui zunächst "npm update" und dann "gulp" aufrufen (Ich gehe davon aus, dass eine Node.js-Umgebung installiert ist...)
- Im Verzeichnis c:/repos/wifimanager_webui zunächst "npm update" und dann "gulp" aufrufen (Ich gehe davon aus, dass eine Node.js-Umgebung installiert ist...)
- Pfade anpassen (Falls Pfade anders gewählt)
- In c:/repos/labathome/labathome_firmware/CMakeLists.txt die Zeile set(EXTRA_COMPONENT_DIRS "../../espidf-components") ggf. auf den Speicherort der eben ausgecheckten espidf-components anpassen
- In c:/repos/espidf-components/wifimanager/CMakeLists.txt die Zeile EMBED_FILES ../../wifimanager_webui/dist_compressed/wifimanager.html.gz ggf anpassen
- im verzeichnis labathome_firmware menuconfig starten und Einstellungen tätigen (Stand 2022-05-12: Gegenwärtig werden die Einstellungen nicht beachtet, Einstellungen sind also nicht erforderlich)
- Compilieren starten
- Wenn Sie diesen Prozess erfolgreich gemeistert haben, ist das Flashen wohl auch kein Problem mehr

Um den Prozess zu vereinfachen, stelle ich auf Anfrage fertige Binaries zur Verfügung

# Getting Started Variante 1 - Software in Betrieb nehmen 
- Ergebnis der Compilierung sind mehrere binäre Dateien, die jetzt in bestimmte Speicherbereiche im Flash-Speicher des Mikrocontrollers geschrieben werden müssen.
- Laden Sie sich zunächst die Software "Flash Download Tools" von https://www.espressif.com/en/support/download/other-tools herunter.
- Entpacken und starten Sie die Software (Doppelklick auf flash_download_tool_x.y.z)
- Wählen Sie bei "chipType" die Option "ESP32" und bei "workMode" die Option "develop"
- Stellen Sie in der nun erscheinenden Hauptoberfläche die folgenden Dateien und Speicherorte ein (achten Sie bei den Speicherorten auf die exakte Schreibweise und vergessen Sie keine "0"). Bei "COM" nutzen Sie den Drop-Down, um den richtigen Anschluss auszuwählen.
![flash_download_tool_setting](https://user-images.githubusercontent.com/7479349/168019275-0f23d562-bf44-4dae-b733-d4ca9e6b8fbd.png)
- Klicken Sie auf "Start"
- Warten Sie, bis der Prozess abgeschlossen ist
- Schließen Sie die Software und betätigen den Reset-Knopf von Lab@home (8-Uhr-Position)
- Verbinden Sie sich mit dem WLAN-Netz labathome-xyz (xyz ist eine einzigartige Seriennummer; bitte notieren!!!) und dem Passwort labathome
- Offnen Sie im Webbrowser die Seite http://192.168.210.0/wifimanager und verbinden Sie lab@home mit ihrem WLAN-Netzwerk
- Wenn Ihr WLAN-Router das unterstützt, sollte Ihr lab@home jetzt unter http://labathome-xyz (Seriennummer!) mit seiner Hauptoberfläche erreichbar sein

# Getting Started Variante 2 - Software rein webbasiert in Betrieb nehmen 
- Schließen Sie auf der Lab@Home-Platine z.B. mit einer Büroklammer zwei Anschlüsse kurz (siehe Bild)
 ![20230924_175428](https://github.com/klaus-liebler/labathome/assets/7479349/4d02154b-1920-4702-956e-c5fe77882c1a)

- Schließen Sie Lab@Home an den Computer an (erst kurzschließen, dann anschließen!)
- Gehen Sie auf [https://klaus-liebler.github.io/labathome/](https://klaus-liebler.github.io/labathome/). Das ist ein komplett webbasiertes Flash-Werkzeug, das das Einspielen einer neuen Firmware aus dem Browser heraus ermöglicht
- Klicken Sie auf "Flash Firmware"
- Wählen Sie den COM-Port von Lab@Home aus. Bei mir sieht das so aus
  ![20230924_7479349](https://github.com/klaus-liebler/labathome/assets/7479349/dc9fadc8-a65a-4f9a-81fb-0cd4b16af6b5)

- Wählen Sie aus, welche Firmware Sie installieren möchten (GA-Master: labathome_firmware_modbus)
- Wenn die Installation erfolgreich beendet ist, können Sie das Browserfenster schließen. Trennen Sie dann Lab@Home vom Computer und entfernen Sie den "Kurzschluss". Jetzt können Sie Lab@Home wieder anschließen und verwenden. Viel Spaß damit!
  
