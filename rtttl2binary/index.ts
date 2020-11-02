const world = 'world';

import * as p from './rtttlparser'


export function hello(word: string = world): string {
  return `Hello ${word}! `;
}
//constexpr Note SONG_NEGATIVE[] = {{880, 300}, {440,300}, {880, 300}, {440,300},{0,0}}; //Zero-both-terminated
function parseATeam(){
    const rtttl="ATeam:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
    let melody =p.parse(rtttl);
    let output = `constexpr Note ${melody.name}[] = {`
    for (const note of melody.melody) {
        output+=`{${note.frequency.toFixed(0)},${note.duration}},`;
    }
    output+=("{0,0}};");
    console.log(output);
    
}

parseATeam();