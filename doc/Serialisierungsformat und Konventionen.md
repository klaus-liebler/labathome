Alle Variablen im System werden über ihren Typ (bool, s32, f32, color, string) und ihre Adresse - ein u32 - identifiziert

Für Bools und Ints gilt
0x0000_0000 bis 0x0000_FFFF: Konstanten mit dem Jeweiligen Wert
0x0001_0000 bis 0x0001_FFFF: Hardware-Inputs 
0x0010_0000 bis 0x0010_FFFF: Hardware-Outputs

0xFFFF_0000 bis 0xFFFF_FFFF: Normale Zwischenvariablen
HIer gilt:
//Unbeschaltete Outputs schreiben in Offset 0. 
//Unbeschaltete Inputs lesen vom Offset 1
//Echte Speicheradressen gibt es dann ab Offset 2

Große Frage ist: Wie schlau und auf das Board angepasst ist die Webapplikation? --> Am besten gar nicht angepasst! 

Die Webapplikation weiß nichts von den Speicheradressen auf dem Board
Der Datensatz erhält 
Version der Datenstruktur u32 (vorerst immer 0)
timestamp u64
KEINE maxIndizes - das kann das Board problemlos selbst rechnen
Für jeden Operator zunächst den TypIndex (u32) und den globalen Index (u32) des Browsers. Dann Adressen u32 für Input und dann Adressen u32 für Output
Danach mögliche KOnfigurationsparameter

Problem: Es muss absolut sicher gestellt sein, dass sind die Länge der Konfiguration eines TypIndex niemals ändert!!!

Der Operator muss sich selbst in ein u32-Array serialisieren können, sofern er die AdressMaps bekommts




Variablen werden FÜR JEDEN DATENTYP UNABHÄNGIG mit einer 32bit ID identifiziert
Für Bool gilt
- Index 0 liefert immer False (dies ist auch der Standardwert für "nicht angeschlossen)
- Index 1 liefert immer True
- Index 2...1023 ist Hardware-Input
- Index 1024...2047 ist Hardware-Output
- Index 2048...3071 ist Kommunikations-Input
- Index 3072...4095 ist Kommunikations-Output
- Variablen mit Index 16384..32767 sind "Zwischenvariablen"
- Variablen mit Index >=32768 sind reserviert (ggf für Retained?)
Für Integer gilt
- Index 0 liefert immer 0
- Index 1 liefert immer 1
- tbc