
export class SerializeContext {
    public writeGUID(guid: Uint8Array) {
        for (let i = 0; i < 16; i++) {
            this.bufferDV.setUint8(this.bufferOffset + i, guid[i]);
        }
        this.bufferOffset += 16;
    }

    public consumeGUIDandCompare(compare:Uint8Array): boolean {
        let val=true;
        for (let i = 0; i < 16; i++) {
            if(compare[i]!=this.bufferDV.getUint8(this.bufferOffset + i)){
                val=false;
                break;
            }
        }
        this.bufferOffset += 16;
        return val;
    }
    private bufferDV: DataView;
    constructor(private buffer: ArrayBuffer, private bufferOffset: number = 0) {
        this.bufferDV = new DataView(buffer);
    }

    public funhash(start:number, end:number):number{
        end=Math.min(end, this.bufferDV.byteLength);
        for(var i = start, h = 0xdeadbeef; i < end; i++)
            h = Math.imul(h ^ this.bufferDV.getUint8(i), 2654435761);
        return (h ^ h >>> 16) >>> 0;
    }

    public overwriteU32(theNumber: number, offset:number): void {
        if(offset>this.bufferOffset-4) throw new Error("offset>this.bufferOffset-4");
        this.bufferDV.setUint32(offset, theNumber, true);
    }
    public writeS32(theNumber: number): void {
        this.bufferDV.setInt32(this.bufferOffset, theNumber, true);
        this.bufferOffset += 4;
    }

    public writeU32(theNumber: number): void {
        this.bufferDV.setUint32(this.bufferOffset, theNumber, true);
        this.bufferOffset += 4;
    }

    public writeF32(theNumber: number): void {
        this.bufferDV.setFloat32(this.bufferOffset, theNumber, true);
        this.bufferOffset += 4;
    }

    public readF32(): number {
        let val = this.bufferDV.getFloat32(this.bufferOffset, true);
        this.bufferOffset += 4;
        return val;
    }

    public readU32(): number {
        let val = this.bufferDV.getUint32(this.bufferOffset, true);
        this.bufferOffset += 4;
        return val;
    }

    public readS32(): number {
        let val = this.bufferDV.getInt32(this.bufferOffset, true);
        this.bufferOffset += 4;
        return val;
    }

    public getResult(): ArrayBuffer {
        return this.buffer.slice(0, this.bufferOffset);
    }
}
