export function EscapeToVariableName(n: string) {
    return n.toLocaleUpperCase().replaceAll(" ", "_");
}

export interface IStringBuilder {
    AppendLine(line: string): void;
}

export abstract class ConfigItem {
    constructor(public readonly displayName: string, protected key: string | null = null) { }
    abstract RenderCPPAccessor(codeBuilder: IStringBuilder, group: ConfigGroup): void;
    abstract RenderCPPConfig(codeBuilder: IStringBuilder, group: ConfigGroup): void;
    abstract RenderNvsPartitionGenerator(codeBuilder: IStringBuilder): void;
    public get Key() {
        return this.key ?? this.displayName;
    }
}

export class StringItem extends ConfigItem {
    constructor(displayName: string, public readonly def: string = "", public readonly regex: RegExp = /.*/, key: string | null = null) { super(displayName, key) }

    RenderCPPConfig(codeBuilder: IStringBuilder, group: ConfigGroup): void {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`{${gk}_${k}_KEY, webmanager::Setting::Setting_StringSetting},`);
    }

    RenderNvsPartitionGenerator(codeBuilder: IStringBuilder) {
        codeBuilder.AppendLine(`${this.Key},data,string,${this.def},`);

    }

    RenderCPPAccessor(codeBuilder: IStringBuilder, group: ConfigGroup) {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`\tconstexpr GroupAndStringSetting ${gk}_${k}{${gk}.groupKey, ${gk}_${k}_KEY};`);
    }
}

export class IntegerItem extends ConfigItem {

    constructor(displayName: string, public readonly def: number = 0, public readonly min: number = 0, public readonly max: number = Number.MAX_SAFE_INTEGER, public readonly step: number = 1, key: string | null = null) {
        super(displayName, key)
    }

    RenderCPPConfig(codeBuilder: IStringBuilder, group: ConfigGroup): void {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`{${gk}_${k}_KEY, webmanager::Setting::Setting_IntegerSetting},`);
    }

    RenderNvsPartitionGenerator(codeBuilder: IStringBuilder) {
        codeBuilder.AppendLine(`${this.Key},data,i32,${this.def},`);
    }

    RenderCPPAccessor(codeBuilder: IStringBuilder, group: ConfigGroup) {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`\tconstexpr GroupAndIntegerSetting ${gk}_${k}{${gk}.groupKey, ${gk}_${k}_KEY};`);
    }
}

export class BooleanItem extends ConfigItem {

    constructor(displayName: string, public readonly def: boolean = false, key: string | null = null) {
        super(displayName, key);
    }

    RenderCPPConfig(codeBuilder: IStringBuilder, group: ConfigGroup): void {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`{${gk}_${k}_KEY, webmanager::Setting::Setting_BooleanSetting},`);
    }

    RenderNvsPartitionGenerator(codeBuilder: IStringBuilder) {
        codeBuilder.AppendLine(`${this.Key},data,u8,${this.def ? "1" : "0"},`);
    }

    RenderCPPAccessor(codeBuilder: IStringBuilder, group: ConfigGroup) {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`\tconstexpr GroupAndBooleanSetting ${gk}_${k}{${gk}.groupKey, ${gk}_${k}_KEY};`);
    }
}

export class EnumItem extends ConfigItem {

    constructor(displayName: string, public readonly values: string[], key: string | null = null) {
        super(displayName, key);
    }

    RenderCPPConfig(codeBuilder: IStringBuilder, group: ConfigGroup): void {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`{${gk}_${k}_KEY, webmanager::Setting::Setting_EnumSetting},`);
    }

    RenderNvsPartitionGenerator(codeBuilder: IStringBuilder) {
        codeBuilder.AppendLine(`${this.Key},data,i32,0,`);
    }

    RenderCPPAccessor(codeBuilder: IStringBuilder, group: ConfigGroup) {
        let k = EscapeToVariableName(this.Key);
        let gk = EscapeToVariableName(group.Key);
        codeBuilder.AppendLine(`\tconstexpr GroupAndEnumSetting ${gk}_${k}{${gk}.groupKey, ${gk}_${k}_KEY};`);
    }
}

export class ConfigGroup {
    constructor(public readonly displayName: string, public items: ConfigItem[], private key: string | null = null) { }

    public get Key() {
        return this.key ?? this.displayName;
    }

    RenderCPPConfig(codeBuilder: IStringBuilder) {
        codeBuilder.AppendLine(`constexpr GroupCfg ${EscapeToVariableName(this.Key)} = {"${this.Key}", ${this.items.length}, { `);
    }
}