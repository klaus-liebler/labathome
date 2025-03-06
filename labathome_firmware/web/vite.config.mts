import { defineConfig} from 'vite'
import { viteSingleFile } from "@klaus-liebler/vite-single-file"
import fs from "node:fs"

// https://vitejs.dev/config/
export default defineConfig(({ command, mode, isSsrBuild, isPreview }) => {
  return {
    plugins: [viteSingleFile(),],//removeViteModuleLoader=true for viteSingleFile had no effect on bundle size
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
        key: fs.readFileSync(process.env.USERPROFILE+"/netcase/certificates/testserver.pem.key"),
        cert: fs.readFileSync(process.env.USERPROFILE+"/netcase/certificates/testserver.pem.crt"),

      },

      proxy_not_working_idea: {
        "/webmanager_ws": {
          target: "wss://labathome_6550c0",
          ws: true,
        },
        "/files/*": {
          target: "https://labathome_6550c0",
        }
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
