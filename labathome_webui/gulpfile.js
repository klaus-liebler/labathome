var gulp = require("gulp");

var source = require("vinyl-source-stream");
var ts = require("gulp-typescript");
var tsProject = ts.createProject("tsconfig.json");
const minify = require('gulp-minify');
var sourcemaps = require("gulp-sourcemaps");
var buffer = require("vinyl-buffer");
const { series, parallel } = require("gulp");
const sass = require('gulp-sass')(require('sass'));
const gulp_clean = require('gulp-clean');
const gzip = require('gulp-gzip');
const inlinesource = require('gulp-inline-source');
const browserify = require("browserify");
const tsify = require("tsify");


var paths = {
    pages: ["src/*.html"],
  };
  
function clean(cb)
{
    return gulp.src('{dist,dist_compressed}', {read: false, allowEmpty:true}).pipe(gulp_clean());
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
    .pipe(gulp.dest('./dist_compressed'));
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
