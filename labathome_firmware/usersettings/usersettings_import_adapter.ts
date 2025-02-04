


export abstract class ConfigItem {
    constructor(public readonly displayName: string, protected readonly key: string | null = null) { }
}

export class StringItem extends ConfigItem {
    constructor(public readonly displayName: string, public readonly def: string = "", public readonly regex: RegExp = /.*/, key: string | null = null) {super(displayName, key) }


}

export class IntegerItem extends ConfigItem{
    constructor(public readonly displayName: string, public readonly def: number = 0, public readonly min: number = 0, public readonly max: number = Number.MAX_SAFE_INTEGER, public readonly step: number = 1, key: string | null = null) { super(displayName, key)}
}

export class BooleanItem extends ConfigItem{
    constructor(public readonly displayName: string, public readonly def: boolean = false, key: string | null = null) {super(displayName, key)}
}

export class EnumItem extends ConfigItem{
    constructor(public readonly displayName: string, public readonly values: string[], key: string | null = null) { super(displayName, key)}
}

export class ConfigGroup{
    constructor(public readonly displayName: string, public items: ConfigItem[], private key: string | null = null) {}
}