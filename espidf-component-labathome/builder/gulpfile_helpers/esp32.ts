import { SerialPort, SlipDecoder, SlipEncoder } from "serialport";
import { SetOptions } from '@serialport/bindings-interface'
import { autoDetect } from '@serialport/bindings-cpp'
import { X02 } from "./utils";

export abstract class ESP32Type {
    constructor(protected loader:EspLoader){}
    protected _chipName="undefined"
    protected _hasEncryptionKey=false;
    protected _mac=new Uint8Array(6);
    public abstract updateChipInfo():void;
    public get chipName(){return this._chipName;}
    public get macAsUint8Array(){
        return this._mac;
    }

    public get hasEncryptionKey(){
        return this._hasEncryptionKey;
    }

    public get macAsNumber(){
        var ret=0;
        for (let index = 0; index < 6; index++) {
            ret+=this._mac[5-index]*Math.pow(256, index)
        }
        return ret; 
    }

    public get macAsHexString(){
        return "0x"+X02(this._mac[0])+X02(this._mac[1])+X02(this._mac[2])+X02(this._mac[3])+X02(this._mac[4])+X02(this._mac[5]);
    }

    public get comPort(){
        return this.loader.comPort;
    }
}

class EspLoader {

    static readonly resetAndBootToBootloader: SetOptions = { dtr: false, rts: true };
    static readonly releaseResetButKeepBootloader: SetOptions = { dtr: true, rts: false };
    static readonly freeRunningEPS32: SetOptions = { dtr: false, rts: false };
    static readonly CHIP_DETECT_MAGIC_REG_ADDR = 0x40001000;

    static readonly REQUEST = 0x00;
    static readonly RESPONSE = 0x01;
    static readonly ESP_SYNC = 0x08;
    static readonly ESP_WRITE_REG = 0x09;
    static readonly ESP_READ_REG = 0x0a;

    public get comPort(){
        return this.port;
    }

    private calculateChecksum(data: Buffer) {
        var checksum = 0xef;
        for (var i = 0; i < data.length; i++) {
            checksum ^= data[i];
        }
        return checksum;
    }

    public async sendCommandPacketWithSingleNumberValue(functionCode: number, data: number, needsChecksum = false): Promise<BootloaderReturn> {
        var b = Buffer.allocUnsafe(4);
        b.writeUint32LE(data, 0);
        return this.sendCommandPacket(functionCode, b, needsChecksum);
    }

    public async sendCommandPacket(commandCode: number, data: Buffer, needsChecksum = false): Promise<BootloaderReturn> {
        var b = Buffer.allocUnsafe(8 + data.byteLength);
        b.writeUInt8(EspLoader.REQUEST, 0);
        b.writeUInt8(commandCode, 1);
        b.writeUint16LE(data.byteLength, 2);
        b.writeUint32LE(needsChecksum ? this.calculateChecksum(data) : 0, 4);
        data.copy(b, 8);
        while (this.slipDecoder.read() != null) { ; }
        this.slipEncoder.write(b, undefined, ((error: Error | null | undefined) => {
            //console.debug(`${n()} Message out ${util.format(b)} and error ${error}`);
        }));
        let timeout = 10;
        while (timeout > 0) {
            var b1: Buffer = this.slipDecoder.read();
            if (b1) {
                if (b1.readUInt8(0) != EspLoader.RESPONSE) {
                    //console.error("b[0]!=RESPONSE");
                    return new BootloaderReturn(false, 0, null);
                }
                if (b1.readUInt8(1) != commandCode) {
                    //console.error("b[1]!=lastCommandCode");
                    return new BootloaderReturn(false, 0, null);
                }
                var receivedSize = b1.readUInt16LE(2);
                var receivedValue = b1.readUint32LE(4);
                var receivedPayload: Buffer | null = null;
                if (receivedSize > 0) {
                    receivedPayload = Buffer.allocUnsafe(receivedSize);
                    b1.copy(receivedPayload, 0, 8, 8 + receivedSize);
                }
                //console.debug(`${n()} Message in Received size=${receivedSize} value=${receivedValue}`);
                return new BootloaderReturn(true, receivedValue, receivedPayload);
            }
            await sleep(50);
            timeout--;
        }
        console.error("Timeout!")
        return new BootloaderReturn(false, 0, null);
    }
    private syncbuffer = Buffer.from([0x07, 0x07, 0x12, 0x20, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,]);

    public syncronize = async (retries: number) => {
        while (retries > 0) {
            var res = await this.sendCommandPacket(EspLoader.ESP_SYNC, this.syncbuffer);
            if (res.valid) break;
            retries--;
        }
        return retries == 0 ? false : true;
    };
    public readRegister(reg: number) {
        return this.sendCommandPacketWithSingleNumberValue(EspLoader.ESP_READ_REG, reg);
    }

    public async readRegisters(baseAddress: number, len: number): Promise<Uint32Array> {
        var registers = new Uint32Array(len);
        for (let i = 0; i < len; i++) {
            var res = await this.readRegister(baseAddress + i * 4);
            if(!res.valid) return Promise.reject("readRegister failed")
            registers[i] = res.value;
        }
        //console.log(`ReadRegisters(baseAddress=${baseAddress}) => ${util.format(registers)}`)
        return registers;
    }

    private slipEncoder = new SlipEncoder({ bluetoothQuirk: true });//After analysis of the data of the original tool, I discovered, that each packet starts with a 0xC0 char (which is the package end char). the "bluetoothQuirk" option does exactly this...
    private slipDecoder = new SlipDecoder();
    private port: SerialPort;

    public promisifiedOpen = (port: SerialPort) => {
        return new Promise<boolean>((resolve, reject) => {
            port.open((err) => {
                if (err) {
                    resolve(false);
                }
                resolve(true);
            });
        });
    }

    constructor(comPort: string) {
        this.port = new SerialPort({
            path: comPort,
            baudRate: 115200,
            autoOpen: false,
        });
    }


    public async GetESP32Object(): Promise<ESP32Type | null> {

        this.slipEncoder.pipe(this.port);
        this.port.pipe(this.slipDecoder);

        this.port.on('error', (err) => {
            console.log('Error: ', err.message)
        });

        if (!await this.promisifiedOpen(this.port)) {
            console.error(`Port ${this.port.path} connot be opened`);
            this.port.close();
            return null;
        }
        this.port.set(EspLoader.resetAndBootToBootloader);
        await sleep(100);
        this.port.set(EspLoader.releaseResetButKeepBootloader);
        await sleep(100);
        this.port.set(EspLoader.freeRunningEPS32);

        if (!await this.syncronize(5)) {
            console.error("Sync was not successful");
            this.port.close();
            return null;
        }
        //console.info(`${n()} Sync was successful`);
        var res = await this.readRegister(EspLoader.CHIP_DETECT_MAGIC_REG_ADDR)

        if (!res.valid) {
            console.log("The Magic Register Address could not be read");
            return null;
        }
        //console.log("The Magic Register Address is " + res.value);
        let esp32type: ESP32Type | null = null;
        switch (res.value) {
            case 0x00f01d83: {
                //console.info("Detected ESP32");
                return new ESP32Classic(this);
            }
            case 0x6f51306f:
            case 0x7c41a06f: {
                console.error("ESP32C2ROM() not supported");
                return null;
            }
            case 0x6921506f:
            case 0x1b31506f:
            case 0x4881606f:
            case 0x4361606f: {

                console.error("ESP32C3ROM() not supported");
                return null;
            }
            case 0x2ce0806f: {
                console.error("ESP32C6ROM not supported");
                return null;
            }
            case 0x33f0206f:
            case 0x2421606f: {
                console.error("ESP32C61ROM not supported");
                return null;
            }
            case 0x1101406f:
            case 0x63e1406f: {
                console.error("ESP32C5ROM not supported");
                return null;
            }
            case 0xd7b73e80: {
                console.error("ESP32H2ROM not supported");
                return null;
            }
            case 0x09: {
                //console.info("Detected ESP32S3ROM");
                return new ESP32S3(this);
            }
            case 0x000007c6: {
                console.error("ESP32S2ROM not supported");
                return null;
            }
            case 0xfff0c101: {
                console.error("ESP8266ROM not supported");
                return null;
            }
            case 0x0:
            case 0x0addbad0:
            case 0x7039ad9: {
                console.error("ESP32P4ROM not supported");
                return null;
            }
            default:
                console.error("Unknown magic code --> not supported");
                return null;
        }
    }

    public async Close() {
        this.port.close();
    }

}

class ESP32Classic extends ESP32Type {
    constructor(loader:EspLoader){
        super(loader);
        this._chipName="ESP32"
    }
    static readonly EFUSE_BASE = 0x3ff5a000;
    static readonly MACFUSEADDR = ESP32Classic.EFUSE_BASE + 0x1;
    
    async updateChipInfo () {
        var efuses = await this.loader.readRegisters(ESP32Classic.MACFUSEADDR, 2);
        let mac0 = efuses[0];
        let mac1 = efuses[1];
        mac0 = mac0 >>> 0;
        mac1 = mac1 >>> 0;
        this._mac = new Uint8Array(6);
        this._mac[0] = (mac1 >> 8) & 0xff;
        this._mac[1] = mac1 & 0xff;
        this._mac[2] = (mac0 >> 24) & 0xff;
        this._mac[3] = (mac0 >> 16) & 0xff;
        this._mac[4] = (mac0 >> 8) & 0xff;
        this._mac[5] = mac0 & 0xff;
    }

}

enum KeyPurpose{
    USER_EMPTY=0,
    RESERVED=1,
    XTS_AES_256_KEY_1=2,
    XTS_AES_256_KEY_2=3,
    XTS_AES_128_KEY=4,
    HMAC_DOWN_ALL=5,
    HMAC_DOWN_JTAG=6,
    HMAC_DOWN_DIGITAL_SIGNATURE=7,
    HMAC_UP=8,
    SECURE_BOOT_DIGEST0=9,
    SECURE_BOOT_DIGEST1=10,
    SECURE_BOOT_DIGEST2=11,
}

class ESP32S3 extends ESP32Type {
    constructor(loader:EspLoader){
        super(loader);
        this._chipName="ESP32S3"
    }
    static readonly EFUSE_BASE = 0x6000_7000;
    static readonly EFUSE_RD_REG_BASE           = ESP32S3.EFUSE_BASE + 0x030  //BLOCK0 read base address
    static readonly EFUSE_BLOCK1_ADDR           = ESP32S3.EFUSE_BASE + 0x44;
    static readonly EFUSE_BLOCK2_ADDR           = ESP32S3.EFUSE_BASE + 0x5C;
    static readonly EFUSE_RD_REPEAT_DATA0_REG   = ESP32S3.EFUSE_BASE + 0x030;
    static readonly EFUSE_RD_REPEAT_DATA1_REG   = ESP32S3.EFUSE_BASE + 0x034;
    
    static readonly MACFUSEADDR                 = ESP32S3.EFUSE_BLOCK1_ADDR;

    async updateChipInfo () {
        var efuses = await this.loader.readRegisters(ESP32S3.MACFUSEADDR, 2);
        let mac0 = efuses[0];
        let mac1 = efuses[1];
        //let mac2 = efuses[2];
        //let mac3 = efuses[3];
        //valid only for ESP32S3
        this._mac[0] = (mac1 >> 8) & 0xff;
        this._mac[1] = mac1 & 0xff;
        this._mac[2] = (mac0 >> 24) & 0xff;
        this._mac[3] = (mac0 >> 16) & 0xff;
        this._mac[4] = (mac0 >> 8) & 0xff;
        this._mac[5] = mac0 & 0xff;
        var data_regs_efuses = await this.loader.readRegisters(ESP32S3.EFUSE_RD_REPEAT_DATA1_REG, 2);
        const purposes=[((data_regs_efuses[0]>>24)& 0xF) as KeyPurpose, ((data_regs_efuses[0]>>28)& 0xF) as KeyPurpose, ((data_regs_efuses[1]>>0)& 0xF) as KeyPurpose,((data_regs_efuses[1]>>4)& 0xF) as KeyPurpose,((data_regs_efuses[1]>>8)& 0xF) as KeyPurpose,((data_regs_efuses[1]>>12)& 0xF) as KeyPurpose,];
        const SPI_BOOT_CRYPT_CNT = (data_regs_efuses[0]>>18)& 0x7
        if(purposes[0]==KeyPurpose.XTS_AES_256_KEY_1 && purposes[1]==KeyPurpose.XTS_AES_256_KEY_2) this._hasEncryptionKey=true;
        else if(!(purposes[0]==KeyPurpose.USER_EMPTY && purposes[1]==KeyPurpose.USER_EMPTY)){
            throw Error("Unexpected key purposes");
        }
        if(this.hasEncryptionKey && !(SPI_BOOT_CRYPT_CNT==0b1 || SPI_BOOT_CRYPT_CNT==0b11 || SPI_BOOT_CRYPT_CNT==0b111)){
            throw Error(`Encryption Key is XTS_AES_256, but SPI_BOOT_CRYPT_CNT has no odd number of ones, but 0b${SPI_BOOT_CRYPT_CNT.toString(2)}`);	
        }
    }
}


const sleep = async (ms = 100) => new Promise((resolve) => setTimeout(resolve, ms));
const n = () => new Date().toLocaleTimeString();



class BootloaderReturn {
    constructor(public readonly valid: boolean, public readonly value: number, public readonly payload: Buffer | null) { }
}


export async function GetESP32ObjectFromSpecificPort(comPort: string): Promise<ESP32Type|null> {
    var loader = new EspLoader(comPort);
    var res = await loader.GetESP32Object();
    if (!res) {
        return null;
    }
    await res.updateChipInfo();
    loader.Close();
    return res;
}

interface KLPortResult{
    friendlyName:string;
    locationId:string;
    manufacturer:string;
    path:string;
    pnpId:string;
    productId:string;
    serialNumber:string;
    vendorId:string;
}

export async function GetESP32Object(): Promise<ESP32Type|null> {
    const portInfo = await autoDetect().list() as Array<KLPortResult>;
    let ret:ESP32Type|null;
    for (var pi of portInfo) {
        if(pi.productId!="1001" || pi.vendorId!="303A"){
            continue;
        }
        console.log(`Checking Port '${pi.friendlyName}'`);
        ret=await GetESP32ObjectFromSpecificPort(pi.path)
        if(ret){
            return ret;
        }
    }
    return null;
}