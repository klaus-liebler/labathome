"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.hello = void 0;
var world = 'world';
var p = __importStar(require("./rtttlparser"));
function hello(word) {
    if (word === void 0) { word = world; }
    return "Hello " + word + "! ";
}
exports.hello = hello;
//constexpr Note SONG_NEGATIVE[] = {{880, 300}, {440,300}, {880, 300}, {440,300},{0,0}}; //Zero-both-terminated
function parseATeam() {
    var rtttl = "ATeam:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
    var melody = p.parse(rtttl);
    var output = "constexpr Note " + melody.name + "[] = {";
    for (var _i = 0, _a = melody.melody; _i < _a.length; _i++) {
        var note = _a[_i];
        output += "{" + note.frequency.toFixed(0) + "," + note.duration + "},";
    }
    output += ("{0,0}};");
    console.log(output);
}
parseATeam();
//# sourceMappingURL=index.js.map