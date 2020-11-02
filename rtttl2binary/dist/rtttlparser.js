"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getData = exports.getDefaults = exports.getName = exports.parse = void 0;
/**
 * Parse RTTTL
 *
 * @param {string} rtttl - RTTTL String
 * @returns {object} - An object specifying frequency and duration for each note
 */
function parse(rtttl) {
    var REQUIRED_SECTIONS_NUM = 3;
    var SECTIONS = rtttl.split(':');
    if (SECTIONS.length !== REQUIRED_SECTIONS_NUM) {
        throw new Error('Invalid RTTTL file.');
    }
    var NAME = getName(SECTIONS[0]);
    var DEFAULTS = getDefaults(SECTIONS[1]);
    var MELODY = getData(SECTIONS[2], DEFAULTS);
    return {
        name: NAME,
        defaults: DEFAULTS,
        melody: MELODY
    };
}
exports.parse = parse;
/**
 * Get ring tone name
 *
 * @param {string} name
 * @returns {string}
 */
function getName(name) {
    var MAX_LENGTH = 10;
    if (name.length > MAX_LENGTH) {
        console.warn('Tune name should not exceed 10 characters.');
    }
    if (!name) {
        return 'Unknown';
    }
    return name;
}
exports.getName = getName;
/**
 * Get duration, octave and BPM
 *
 * @param {string} defaults
 * @returns {object}
 */
var Defaults = /** @class */ (function () {
    function Defaults(duration, octave, bpm) {
        this.duration = duration;
        this.octave = octave;
        this.bpm = bpm;
    }
    return Defaults;
}());
function getDefaults(defaults) {
    var VALUES = defaults.split(',');
    var defaultsObj = new Defaults(4, 6, 63);
    for (var _i = 0, VALUES_1 = VALUES; _i < VALUES_1.length; _i++) {
        var kv = VALUES_1[_i];
        if (kv.length == 0)
            continue;
        var kv_arr = kv.split('=');
        if (kv_arr.length !== 2) {
            throw new Error('Invalid setting ' + kv);
        }
        var KEY = kv_arr[0];
        var VAL_STR = kv_arr[1];
        var parsedValue = parseInt(VAL_STR);
        var ALLOWED_DURATION = [1, 2, 4, 8, 16, 32];
        var ALLOWED_OCTAVE = [4, 5, 6, 7];
        switch (KEY) {
            case 'd':
                if (ALLOWED_DURATION.indexOf(parsedValue) !== -1) {
                    defaultsObj.duration = parsedValue;
                }
                else {
                    throw new Error('Invalid duration ' + parsedValue);
                }
            case 'o':
                if (ALLOWED_OCTAVE.indexOf(parsedValue) === -1) {
                    console.warn('Invalid octave ' + parsedValue);
                }
                defaultsObj.octave = parsedValue;
            case 'b':
                defaultsObj.bpm = parsedValue;
        }
    }
    return defaultsObj;
}
exports.getDefaults = getDefaults;
/**
 * Get the parsed melody data
 *
 * @param {string} melody
 * @param {object} defaults
 * @returns {Array}
 */
function getData(melody, defaults) {
    var NOTES = melody.split(',');
    var BEAT_EVERY = 60000 / defaults.bpm;
    return NOTES.map(function (note) {
        var NOTE_REGEX = /(1|2|4|8|16|32|64)?((?:[a-g]|h|p)#?){1}(\.?)(4|5|6|7)?/;
        var NOTE_PARTS = note.match(NOTE_REGEX);
        var NOTE_DURATION = parseInt(NOTE_PARTS[1]) || defaults.duration;
        var NOTE = NOTE_PARTS[2] === 'h' ? 'b' : NOTE_PARTS[2];
        var NOTE_DOTTED = NOTE_PARTS[3] === '.';
        var NOTE_OCTAVE = parseInt(NOTE_PARTS[4]) || defaults.octave;
        return {
            note: NOTE,
            duration: _calculateDuration(BEAT_EVERY, NOTE_DURATION, NOTE_DOTTED),
            frequency: _calculateFrequency(NOTE, NOTE_OCTAVE)
        };
    });
}
exports.getData = getData;
/**
 * Calculate the frequency of a note
 *
 * @param {string} note
 * @param {number} octave
 * @returns {number}
 * @private
 */
function _calculateFrequency(note, octave) {
    if (note === 'p') {
        return 0;
    }
    var C4 = 261.63;
    var TWELFTH_ROOT = Math.pow(2, 1 / 12);
    var N = _calculateSemitonesFromC4(note, octave);
    var FREQUENCY = C4 * Math.pow(TWELFTH_ROOT, N);
    return Math.round(FREQUENCY * 1e1) / 1e1;
}
function _calculateSemitonesFromC4(note, octave) {
    var NOTE_ORDER = ['c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#', 'a', 'a#', 'b'];
    var MIDDLE_OCTAVE = 4;
    var SEMITONES_IN_OCTAVE = 12;
    var OCTAVE_JUMP = (octave - MIDDLE_OCTAVE) * SEMITONES_IN_OCTAVE;
    return NOTE_ORDER.indexOf(note) + OCTAVE_JUMP;
}
/**
 * Calculate the duration a note should be played
 *
 * @param {number} beatEvery
 * @param {number} noteDuration
 * @param {boolean} isDotted
 * @returns {number}
 * @private
 */
function _calculateDuration(beatEvery, noteDuration, isDotted) {
    var DURATION = (beatEvery * 4) / noteDuration;
    var PROLONGED = isDotted ? (DURATION / 2) : 0;
    return DURATION + PROLONGED;
}
//# sourceMappingURL=rtttlparser.js.map