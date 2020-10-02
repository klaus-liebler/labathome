export class SerializeContext {
    private bufferDV: DataView;
    constructor(private buffer: ArrayBuffer, private bufferOffset: number = 0) {
        this.bufferDV = new DataView(buffer);
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

    public getResult(): ArrayBuffer {
        return this.buffer.slice(0, this.bufferOffset);
    }
}
