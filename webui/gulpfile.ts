import * as gulp from 'gulp';
import * as browserify from 'browserify';
// @ts-ignore
import * as  source from 'vinyl-source-stream';
// @ts-ignore
import * as  tsify from 'tsify';
import * as  uglify from 'gulp-uglify';
import * as  sourcemaps from 'gulp-sourcemaps';
import * as  buffer from 'vinyl-buffer';
// @ts-ignore
import inlinesource from 'gulp-inline-source';

var paths = {
    pages: ['src/*.html']
};


gulp.task('copy-html', function () {
    return gulp.src(paths.pages)
        .pipe(gulp.dest('dist'));
});

gulp.task('compileTypescript', function(){
    return browserify({
        basedir: '.',
        debug: true,
        entries: ['src/main.ts'],
        cache: {},
        packageCache: {}
    }).plugin(tsify)
        .bundle()
        .pipe(source('bundle.js'))
        .pipe(buffer())
        .pipe(sourcemaps.init({loadMaps: true}))
        .pipe(uglify())
        .pipe(sourcemaps.write('./'))
        .pipe(gulp.dest('dist'));
});


gulp.task('default', gulp.series('copy-html', 'compileTypescript'));
