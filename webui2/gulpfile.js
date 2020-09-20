var gulp = require("gulp");
var browserify = require("browserify");
var source = require("vinyl-source-stream");
var tsify = require("tsify");
const minify = require('gulp-minify');
var sourcemaps = require("gulp-sourcemaps");
var buffer = require("vinyl-buffer");
const { series, parallel } = require("gulp");
const sass = require('gulp-sass');
const gulp_clean = require('gulp-clean');
const gzip = require('gulp-gzip');
const inlinesource = require('gulp-inline-source');

sass.compiler = require('node-sass');

var paths = {
    pages: ["src/*.html"],
  };
  
function clean(cb)
{
    return gulp.src('{dist,inlined}', {read: false, allowEmpty:true}).pipe(gulp_clean());
}


function tsTranspileAndBundleAndMinify(cb)
{
    return browserify({
        basedir: ".",
        debug: true,
        entries: ["src/App.ts"],
        cache: {},
        packageCache: {},
      })
        .plugin(tsify)
        .bundle()
        .pipe(source("bundle.js"))
        .pipe(buffer())
        .pipe(sourcemaps.init({ loadMaps: true }))
        .pipe(minify())
        .pipe(sourcemaps.write("./"))
        .pipe(gulp.dest("dist"));
}

function cssTranspile(cb)
{
    return gulp.src('./src/app.scss')
    .pipe(sourcemaps.init())
    .pipe(sass({outputStyle: 'compressed'}).on('error', sass.logError))
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('./dist'));
}



function htmlCopy(cb)
{
    return gulp.src(paths.pages).pipe(gulp.dest("dist"));
}

function htmlInline(cb)
{
    return gulp.src('./dist/index.html')
    .pipe(inlinesource())
    .pipe(gzip())
    .pipe(gulp.dest('./inlined'));
}



exports.build =  series(
    clean,
    cssTranspile,
    tsTranspileAndBundleAndMinify,
    htmlCopy,
    htmlInline
  );
exports.default = exports.build;
exports.cssTranspile=cssTranspile;
