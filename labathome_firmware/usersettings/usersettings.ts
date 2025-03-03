
//Dieser Import bzw der im gleichen Verzeichnis liegende "usersettings_import_adapter" sorgt dafür, dass die 
//Items/Group die im Kontext benötigte Funktionalität besitzen
//Diese Datei hier wird im Rahmen des Build-Prozesses an verschiedene Stellen kopiert und der Import-Adapter sorgt für das Folgende
//- Entwicklungszeit: Die Items/Groups dienen dem Intellisense und der Code-Generierung
//- Laufzeit WebUi: Die Items/Groups sorgen dafür, dass eine passende HTML-UI aufgbaut wird und Synchronisationsnachrichten mit dem Server ausgetauscht werden können
import { StringItem, ConfigGroup, IntegerItem, BooleanItem, EnumItem } from './usersettings_import_adapter';
//Hier legt der Nutzer die Einstellungen fest für das aktuelle Projekt


export function Build(boardName:string, boardVersion:number, boardRoles:Array<string>): ConfigGroup[] {
    return [
        new ConfigGroup("Group1",
            [
                new StringItem("Name G1I1 String", "foo", /.*/, "G1_1_S"),
                new StringItem("Name G1I2 String", "BAR", /.*/, "G1_2_S"),
            ]),
        new ConfigGroup("Group2",
            [
                new StringItem("G2I1 String", "BAR", /.*/, "G2_1_S"),
                new IntegerItem("G2I2 Int", 1, 0, 100000, 1, "G2_2_I"),
                new BooleanItem("G2I3 Bool", true, "G2_3_B"),
                new EnumItem("G2I4 Enum", ["Hund", "Katze", "Maus", "Ente"], "G2_4_E"),
            ]),
        new ConfigGroup("wifimanager", [
            new StringItem("ssid", "Woeste123", /.*/),
            new StringItem("password", "wrong", /.*/),
        ])
    ];
}
