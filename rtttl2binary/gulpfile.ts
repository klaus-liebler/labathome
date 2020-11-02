import * as  gulp from "gulp";
import rtttl from "./rtttl2binary";



function convert(cb:Function)
{
    return gulp.src('./data/*.*')
    .pipe(rtttl())
    .pipe(gulp.dest('./dist'));
}

exports.default = convert;

