import { SerialPort, SlipDecoder, SlipEncoder } from "serialport";
import { SetOptions } from '@serialport/bindings-interface'
import { autoDetect } from '@serialport/bindings-cpp'
import * as util from 'node:util';
import * as fs from 'node:fs';
import { ESP32_HOSTNAME_TEMPLATE } from "../gulpfile_config";

interface ESP32Type {
    macAddr(loader:EspLoader): Promise<Uint8Array>;
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
            console.debug(`${n()} Message out ${util.format(b)} and error ${error}`);
        }));
        let timeout = 10;
        while (timeout > 0) {
            var b1: Buffer = this.slipDecoder.read();
            if (b1) {
                if (b1.readUInt8(0) != EspLoader.RESPONSE) {
                    console.error("b[0]!=RESPONSE");
                    return new BootloaderReturn(false, 0, null);
                }
                if (b1.readUInt8(1) != commandCode) {
                    console.error("b[1]!=lastCommandCode");
                    return new BootloaderReturn(false, 0, null);
                }
                var receivedSize = b1.readUInt16LE(2);
                var receivedValue = b1.readUint32LE(4);
                var receivedPayload: Buffer | null = null;
                if (receivedSize > 0) {
                    receivedPayload = Buffer.allocUnsafe(receivedSize);
                    b1.copy(receivedPayload, 0, 8, 8 + receivedSize);
                }
                console.debug(`${n()} Message in Received size=${receivedSize} value=${receivedValue}`);
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
    public readRegister(reg: number){
        return this.sendCommandPacketWithSingleNumberValue(EspLoader.ESP_READ_REG, reg);
    }

    public async readRegisters(baseAddress:number, len:number):Promise<Uint32Array>{
        var registers = new Uint32Array(len);
        for (let i = 0; i < len; i++) {
            var res = await this.readRegister(baseAddress+i*4);
            registers[i] = res.valid ? res.value : 0;
        }
        console.log(`ReadRegisters(baseAddress=${baseAddress}) => ${util.format(registers)}`)
        return registers;
    }

    private slipEncoder = new SlipEncoder({ bluetoothQuirk: true });//After analysis of the data of the original tool, I discovered, that each packet starts with a 0xC0 char (which is the package end char). the "bluetoothQuirk" option does exactly this...
    private slipDecoder = new SlipDecoder();
    private port:SerialPort;

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

    constructor(comPort: string){
        this.port = new SerialPort({
            path: comPort,
            baudRate: 115200,
            autoOpen: false,
        });
    }

    public async Init():Promise<BootloaderReturn>{
        
        this.slipEncoder.pipe(this.port);
        this.port.pipe(this.slipDecoder);
    
        this.port.on('error', (err) => {
            console.log('Error: ', err.message)
        });
    
        if (!await this.promisifiedOpen(this.port)) {
            console.error("Port is not open");
            this.port.close();
            return new BootloaderReturn(false, 0, null);
        }
        console.log(`${n()}  Port has been opened successfully.`);
        this.port.set(EspLoader.resetAndBootToBootloader);
        await sleep(100);
        this.port.set(EspLoader.releaseResetButKeepBootloader);
        await sleep(100);
        this.port.set(EspLoader.freeRunningEPS32);
    
        if (!await this.syncronize(5)) {
            console.error("Sync was not successful");
            this.port.close();
            return new BootloaderReturn(false, 0, null);
        }
        console.info(`${n()} Sync was successful`);
        var res = await this.readRegister(EspLoader.CHIP_DETECT_MAGIC_REG_ADDR)

        if (!res.valid) {
            console.log("The Magic Register Address could not be read");
            return new BootloaderReturn(false, 0, null);
        }
        console.log("The Magic Register Address is " + res.value);
        return res;
    }

    public async Close(){
        this.port.close();
    }

}

class ESP32orig implements ESP32Type {
    static readonly EFUSE_BASE = 0x3ff5a000;
    static readonly MACFUSEADDR = ESP32orig.EFUSE_BASE + 0x1;
    async macAddr(loader:EspLoader): Promise<Uint8Array> {
        var efuses = await loader.readRegisters(ESP32orig.MACFUSEADDR, 2);
        let mac0 = efuses[0];
        let mac1 = efuses[1];
        mac0 = mac0 >>> 0;
        mac1 = mac1 >>> 0;
        const mac = new Uint8Array(6);
        mac[0] = (mac1 >> 8) & 0xff;
        mac[1] = mac1 & 0xff;
        mac[2] = (mac0 >> 24) & 0xff;
        mac[3] = (mac0 >> 16) & 0xff;
        mac[4] = (mac0 >> 8) & 0xff;
        mac[5] = mac0 & 0xff;
        return mac;
    }
    
}

class ESP32S3 implements ESP32Type {
    static readonly EFUSE_BASE = 0x60007000;
    static readonly MACFUSEADDR = ESP32S3.EFUSE_BASE + 0x044;
    
    async macAddr(loader:EspLoader): Promise<Uint8Array> {
        var efuses = await loader.readRegisters(ESP32S3.MACFUSEADDR, 2);
        let macAddr = new Uint8Array(6);
        let mac0 = efuses[0];
        let mac1 = efuses[1];
        //let mac2 = efuses[2];
        //let mac3 = efuses[3];
        //valid only for ESP32S3
        macAddr[0] = (mac1 >> 8) & 0xff;
        macAddr[1] = mac1 & 0xff;
        macAddr[2] = (mac0 >> 24) & 0xff;
        macAddr[3] = (mac0 >> 16) & 0xff;
        macAddr[4] = (mac0 >> 8) & 0xff;
        macAddr[5] = mac0 & 0xff;
        return macAddr;
    }
}


const sleep = async (ms = 100) => new Promise((resolve) => setTimeout(resolve, ms));
const n = () => new Date().toLocaleTimeString();



class BootloaderReturn {
    constructor(public readonly valid: boolean, public readonly value: number, public readonly payload: Buffer | null) { }
}

export async function testopen(comPort: string) {
    try {
        const port = new SerialPort({
            path: comPort,
            baudRate: 115200,
          }, function (err) {
            if (err) {
              return console.log('Error: ', err.message)
            }
          });
          port.on('error', function(err) {
            console.log('Error: ', err.message)
          })
    } catch (error) {
        console.error(error);
    }
   
}

export async function getMac(comPort: string):Promise<Uint8Array> {
    const portInfo = await autoDetect().list();
    for (var i of portInfo) {
        console.log(`${i.path}; ${i.manufacturer}; ${i.serialNumber}; ${i.pnpId}; ${i.locationId}; ${i.productId}; ${i.vendorId};`);
    }
    var loader = new EspLoader(comPort);
    var res = await loader.Init();

    let esp32type: ESP32Type | null = null;
    switch (res.value) {
        case 0x00f01d83:
            esp32type = new ESP32orig();
            break;
        case 0x09:
            esp32type = new ESP32S3();
        default:
            console.error("No implementation for this ESP32 type available")
            return Promise.reject();
    }
    
    console.info(`Found a connected ${esp32type.constructor.name}`)

    var mac =  await esp32type.macAddr(loader);
    
    loader.Close();
    return mac;
}