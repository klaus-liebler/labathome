/**
 * Parse RTTTL
 *
 * @param {string} rtttl - RTTTL String
 * @returns {object} - An object specifying frequency and duration for each note
 */
export function parse(rtttl:string) {

  const REQUIRED_SECTIONS_NUM = 3;
  const SECTIONS = rtttl.split(':');

  if (SECTIONS.length !== REQUIRED_SECTIONS_NUM) {
    throw new Error('Invalid RTTTL file.');
  }

  const NAME     = getName(SECTIONS[0]);
  const DEFAULTS = getDefaults(NAME, SECTIONS[1]);
  const MELODY   = getData(SECTIONS[2], DEFAULTS);

  return {
    name     : NAME,
    defaults : DEFAULTS,
    melody   : MELODY
  }
}

/**
 * Get ring tone name
 *
 * @param {string} name
 * @returns {string}
 */
export function getName(name:string) {

  const MAX_LENGTH = 10;

  if (name.length > MAX_LENGTH) {
    console.warn('Tune name should not exceed 10 characters.');
  }

  if (!name) {
    return 'Unknown';
  }

  return name;

}

/**
 * Get duration, octave and BPM
 *
 * @param {string} defaults
 * @returns {object}
 */

 class Defaults{
   constructor(public duration:number, public octave:number, public bpm:number){}
 }


export function getDefaults(name:string, defaults:string):Defaults {

  const VALUES:string[] = defaults.split(',');
  let defaultsObj = new Defaults(4,6,63);
  for(let kv of VALUES){
    if(kv.length==0) continue;
    const kv_arr = kv.split('=');

    if (kv_arr.length !== 2) {
      throw new Error('Invalid setting ' + kv);
    }
    console.log(name + " "+kv);
    const KEY = kv_arr[0].trim();
    const VAL_STR = kv_arr[1].trim();
    let parsedValue = parseInt(VAL_STR);

    const ALLOWED_DURATION = [1, 2, 4, 8, 16, 32];
    const ALLOWED_OCTAVE   = [4, 5, 6, 7];

    switch(KEY) {
      case 'd':
        if (ALLOWED_DURATION.indexOf(parsedValue) !== -1) {
          defaultsObj.duration=parsedValue;
        } else {
          throw new Error('Invalid duration ' + parsedValue);
        }
        break;
      case 'o':
        if (ALLOWED_OCTAVE.indexOf(parsedValue) === -1) {
          console.warn('Invalid octave ' + parsedValue);
        }
        defaultsObj.octave=parsedValue;
        break;
      case 'b':
        console.log(parsedValue);
        defaultsObj.bpm=parsedValue;
        break;
    }
  }

  return defaultsObj;

}



/**
 * Get the parsed melody data
 *
 * @param {string} melody
 * @param {object} defaults
 * @returns {Array}
 */
export function getData(melody:string, defaults:Defaults) {

  const NOTES       = melody.split(',');
  const msPerFullNote  = 4*60000 / defaults.bpm;

  return NOTES.map((note) => {

    const NOTE_REGEX = /(1|2|4|8|16|32|64)?((?:[a-g]|h|p)#?){1}(\.?)(4|5|6|7)?/;
    const NOTE_PARTS = note.match(NOTE_REGEX)!;

    const NOTE_DURATION = parseInt(NOTE_PARTS[1]) || defaults.duration;
    const NOTE          = NOTE_PARTS[2] === 'h' ? 'b' : NOTE_PARTS![2];
    const NOTE_DOTTED   = NOTE_PARTS[3] === '.';
    const NOTE_OCTAVE:number   = parseInt(NOTE_PARTS[4]) || defaults.octave;

    return {
      note: NOTE,
      duration: _calculateDuration(msPerFullNote, NOTE_DURATION!, NOTE_DOTTED),
      frequency: _calculateFrequency(NOTE, NOTE_OCTAVE)
    };
  });
}

/**
 * Calculate the frequency of a note
 *
 * @param {string} note
 * @param {number} octave
 * @returns {number}
 * @private
 */
function _calculateFrequency(note:string, octave:number) {

  if (note === 'p') {
    return 0;
  }

  const C4           = 261.63;
  const TWELFTH_ROOT = Math.pow(2, 1/12);
  const N            = _calculateSemitonesFromC4(note, octave);
  const FREQUENCY    = C4 * Math.pow(TWELFTH_ROOT, N);

  return Math.round(FREQUENCY * 1e1) / 1e1;
}

function _calculateSemitonesFromC4(note:string, octave:number) {

  const NOTE_ORDER          = ['c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#', 'a', 'a#', 'b'];
  const MIDDLE_OCTAVE       = 4;
  const SEMITONES_IN_OCTAVE = 12;

  const OCTAVE_JUMP = (octave - MIDDLE_OCTAVE) * SEMITONES_IN_OCTAVE;

  return NOTE_ORDER.indexOf(note) + OCTAVE_JUMP;

}

/**
 * Calculate the duration a note should be played
 *
 * @param {number} msPerFullNote
 * @param {number} noteDuration
 * @param {boolean} isDotted
 * @returns {number}
 * @private
 */
function _calculateDuration(msPerFullNote:number, noteDuration:number, isDotted:boolean) {
  const DURATION = msPerFullNote / noteDuration;
  const PROLONGED = isDotted ? (DURATION / 2) : 0;
  return Math.round(DURATION + PROLONGED);
}