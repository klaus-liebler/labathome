"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
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
var s = __importStar(require("stream"));
var RTTTL2BinaryTransform = /** @class */ (function (_super) {
    __extends(RTTTL2BinaryTransform, _super);
    function RTTTL2BinaryTransform(opts) {
        var _this = _super.call(this, opts) || this;
        _this.counter = 0;
        return _this;
    }
    RTTTL2BinaryTransform.prototype._transform = function (chunk, encoding, callback) {
        this.counter++;
        var error = null;
        var buffer = chunk;
        var output = "Counter is " + this.counter + ", FileContent is " + buffer.toString();
        console.log(output);
        callback(error, null);
    };
    RTTTL2BinaryTransform.prototype._flush = function (callback) {
        console.log("flush");
        callback(null, null);
    };
    return RTTTL2BinaryTransform;
}(s.Transform));
exports.default = (function () { return new RTTTL2BinaryTransform({ objectMode: true }); });
//# sourceMappingURL=rtttl2binary.js.map