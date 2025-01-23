import { defineConfig, loadEnv } from 'vite'
import { viteSingleFile } from "./vite-single-file/vite-single-file"
import fs from "node:fs"

// https://vitejs.dev/config/
export default defineConfig(({ command, mode, isSsrBuild, isPreview }) => {
  const env = loadEnv(mode, process.cwd(), '')
  return {
    plugins: [viteSingleFile(),],//removeViteModuleLoader=true for viteSingleFile had no effect on bundle size
    build: {
      //minify: true,
      cssCodeSplit: false,
    },
    define: {
      __BOARD_NAME__: JSON.stringify("BOARD_NAME"),
      __BOARD_VERSION__: 0,
      __BOARD_MAC__: 1,
      __APP_NAME__: JSON.stringify("Lab@Home build manually"),
      __APP_VERSION__: JSON.stringify("APP_VERSION"),
      __CREATION_DT__: 0,
      ENABLE_HEATER_EXPERIMENT: true,
      ENABLE_BRIGHTNESS_EXPERIMENT: true,
      ENABLE_FOURIER_EXPERIMENT: true,
      ENABLE_LABBY: true,
    },
    esbuild: {
      //drop:["console", 'debugger'],
      legalComments: 'none',

    },
    server: {
      open: "https://protzklotz:5173",
      https: {
        key: fs.readFileSync(process.env.USERPROFILE+"/netcase/certificates/testserver.pem.key"),
        cert: fs.readFileSync(process.env.USERPROFILE+"/netcase/certificates/testserver.pem.crt"),
      },

      proxy: {
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