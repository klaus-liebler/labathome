"use strict";
var Transform = require('stream').Transform;
module.exports = function () {
    // Monkey patch Transform or create your own subclass,
    // implementing `_transform()` and optionally `_flush()`
    var transformStream = new Transform({ objectMode: true });
    /**
     * @param {Buffer|string} file
     * @param {string=} encoding - ignored if file contains a Buffer
     * @param {function(Error, object)} callback - Call this function (optionally with an
     *          error argument and data) when you are done processing the supplied chunk.
     */
    transformStream._transform = function (file, encoding, callback) {
        var error = null, output = file;
        callback(error, output);
    };
    return transformStream;
};
//# sourceMappingURL=rtttl2binary2.js.map