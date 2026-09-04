import { defineConfig} from 'vite'
// Geteiltes Plugin (s. npm-packages/@klaus-liebler/vite-firmware-single-file, gemeinsam mit
// factory_in_a_box/sensact) inlined JS+CSS in eine einzige dist/index.html, entfernt zusaetzliche
// Leerzeichen (inkl. Lit-Templates) und schreibt das Ergebnis direkt Brotli-komprimiert als
// "index.compressed.br" in den von builder/gulpfile.ts (buildAndCompressWebProject) uebergebenen
// outDir -- ersetzt damit sowohl das bisherige @klaus-liebler/vite-single-file (reines Inlining
// ohne Minify/Kompression) als auch den externen Zweit-Build in
// vite_helper.buildAndCompressWebProject.
import { singleFileFirmwareAssetPlugin } from "@klaus-liebler/vite-firmware-single-file"
import fs from "node:fs"
import path from "node:path"
import { visualizer } from 'rollup-plugin-visualizer'
export default defineConfig(({ command, mode, isSsrBuild, isPreview }) => {
  const isAnalyze = mode === 'analyze';
  return {
    plugins: [
      isAnalyze && visualizer({
        filename: './dist/stats.html',
        open: true,
        gzipSize: true,
        brotliSize: true,
        template: 'treemap',
      }),
      !isAnalyze && singleFileFirmwareAssetPlugin("index.compressed.br"),
    ].filter(Boolean),
    build: {
      //minify: false,
      cssCodeSplit: false,
    },
    esbuild: {
      //drop:["console", 'debugger'],
      legalComments: 'none',

    },
    server: {
      open: "https://protzklotz:5173",
      cors:true,
      https: {
        key: fs.readFileSync(path.join(process.env.USERPROFILE, "OneDrive - HSOS", "certificates", "testserver.pem.key")),
        cert: fs.readFileSync(path.join(process.env.USERPROFILE, "OneDrive - HSOS", "certificates", "testserver.pem.crt")),

      },

      proxy: {
        "/webmanager_ws": {
          target: "ws://labathome_2c31d0.local",
          ws: true,
        },
        "/files": {
          target: "http://labathome_2c31d0.local",
        }
      },

      proxy_local: {
        "/webmanager_ws": {
          target: "ws://localhost:3000",
          ws: true,
        },
        "/files": {
          target: "http://localhost:3000",
        },
        "/labathome": {
          target: "http://localhost:3001",
        }
      }
    }
  }
})
