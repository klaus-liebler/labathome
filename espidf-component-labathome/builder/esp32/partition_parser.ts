import { Hash, createHash } from 'node:crypto'

const MAX_PARTITION_LENGTH = 0xC00;   // 3K for partition data (96 entries) leaves 1K in a 4K sector for signature
const MD5_PARTITION_BEGIN = 0xEBEB;  // The first 2 bytes are like magic numbers for MD5 sum
const PARTITION_TABLE_SIZE  = 0x1000;  // Size of partition table

const MIN_PARTITION_SUBTYPE_APP_OTA = 0x10
const NUM_PARTITION_SUBTYPE_APP_OTA = 16

const SECURE_V1 = 'v1'
const SECURE_V2 = 'v2'

function md5(content:Buffer):Hash {  
    return createHash('md5').update(content);
  }

enum ALIGNMENT {
    APP_TYPE = 0x10000,
    DATA_TYPE = 0x1000,
}

enum APP_SUBTYPE {
    'factory' = 0x00,
    'test' = 0x20,
};

enum DATA_SUBTYPE {
    'ota' = 0x00,
    'phy' = 0x01,
    'nvs' = 0x02,
    'coredump' = 0x03,
    'nvs_keys' = 0x04,
    'efuse' = 0x05,
    'undefined' = 0x06,
    'esphttpd' = 0x80,
    'fat' = 0x81,
    'spiffs' = 0x82,
    'timeseries' = 253,
};


export class PartitionInfo {
    static MAGIC_BYTES = 0x50AA;
    constructor(public readonly label: string, public readonly isData: boolean, public readonly subtype: number, public readonly offset: number, public readonly length: number, public readonly encrypted:boolean) { }

    static fromBinary(buf:Buffer):PartitionInfo{
        var magicActual = buf.readUInt16LE(0)
        if(magicActual!=this.MAGIC_BYTES){
            throw Error(`buf.readUInt16LE(0) ${magicActual.toString(16)}!=this.MAGIC_BYTES${this.MAGIC_BYTES.toString(16)}`);
        }
        let type= buf.readUint8(2);
        let subtype= buf.readUint8(3);
        let offset = buf.readUInt32LE(4);
        let length=buf.readUint32LE(8);
        let label = buf.toString('latin1', 12, 28)
        let l=buf.readUint32LE(28);
        return new PartitionInfo(label, type!=0, subtype, offset, length, l!=0);
    }
    public toString():string{
        let subtype_str = this.isData?DATA_SUBTYPE[this.subtype]??this.subtype:APP_SUBTYPE[this.subtype]??this.subtype;
        return `{label:"${this.label}", isdata:${this.isData}, subtype:${subtype_str}, offset:0x${this.offset.toString(16)}, size:0x${this.length.toString(16)}, encrypted:${this.encrypted},}`
    }
}

export function parsePartitions(binaryPartitionInfo: Buffer):Array<PartitionInfo> {
    let ret:Array<PartitionInfo>=[];
    var md5= createHash('md5');
    if(binaryPartitionInfo.byteLength%32!=0){
        console.error('Partition table length must be a multiple of 32 bytes');
        return ret;
    }
    var o=0;
    for(o=0;o<binaryPartitionInfo.byteLength;o+=32){
        var data = binaryPartitionInfo.subarray(o, o+32);
        if(data.readUInt16LE(0)==MD5_PARTITION_BEGIN){
            console.info(`Found magic partition begin marker on offset ${o}, checking MD5 hash`);
            let hash = md5.digest();
            if(data.subarray(16, 32).compare(hash)==0){
                console.info("MD5 check passed");
            }else{
                console.info(`MD5 check failed ${data.subarray(16, 32).toString('hex')} vs ${hash.toString('hex')}`);
            }
            break;
        }
        md5.update(data);
        ret.push(PartitionInfo.fromBinary(data));
    }
    var errorInEndMarker=false;

    for(var oo=o+32;oo<o+64;oo+=4){
        var val=binaryPartitionInfo.readUInt32LE(oo)
        if(val!=0xFFFFFFFF){
            console.warn(`End Marker at position ${oo} contains ${val}`);
            errorInEndMarker=true;
        }
    }
    if(!errorInEndMarker){
        console.info(`End Marker successfully parsed`);
    }
    return ret;
}