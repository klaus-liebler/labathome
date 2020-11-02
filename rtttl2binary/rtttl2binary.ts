import * as s from  'stream';
import File from 'vinyl';
import * as p from  "./rtttlparser";

class RTTTL2BinaryTransform extends s.Transform {
  private counter = 0;
  private code = "";
  private melodyNames:string[] = [];
  constructor(opts?: s.TransformOptions){
    super(opts);
  }

  _transform(chunk: File, encoding: BufferEncoding, callback: s.TransformCallback): void{
    this.counter++;
    let error = null;
    let melody = p.parse(chunk.contents!.toString());
    let validMelodyName =melody.name.replace(" ", "_");
    



    this.code += `constexpr Note ${validMelodyName}[] = {`
    for (const note of melody.melody) {
      this.code+=`{${note.frequency.toFixed(0)},${note.duration}},`;
    }
    this.code+=("{0,0}};\n");
    this.melodyNames.push(validMelodyName);
    callback(null, null);
  }
  _flush(callback: s.TransformCallback): void{

    /*
const Note *SONGS[] = {0, SONG_NEGATIVE, SONG_POSITIVE, ATeam};
import { StringNumberTuple } from "../Utils";
export default function(){ return [
    new StringNumberTuple("No Song", 0),
    new StringNumberTuple("Negative", 1),
    new StringNumberTuple("Positive", 2),
    new StringNumberTuple("LabathomeJingle", 3),
];
};
*/

    let tsCode = `import { StringNumberTuple } from "../Utils";\nexport default function(){ return [\n\tnew StringNumberTuple("No Song", 0),\n`;
    let index=1;
    this.code+="const Note *SONGS[] = {0, ";
    for(const name of this.melodyNames){
      this.code+=`${name}, `;
      tsCode+=`\tnew StringNumberTuple("${name}", ${index}),\n`;
      index++;
    }
    this.code+="};\n";
    tsCode+=`];\n};\n`;
    this.push(new File({path: 'Songs.ts', contents:Buffer.from(tsCode)}));
    this.push(new File({path: 'songs.hh', contents:Buffer.from(this.code)}));
    callback(null, null);
  }
}


export default ()=>{return new RTTTL2BinaryTransform({objectMode:true});}