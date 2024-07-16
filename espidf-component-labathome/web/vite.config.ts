import { defineConfig } from 'vite'
import {viteSingleFile} from "./vite_plugin/vite-plugin-singlefile"
import svg from 'vite-plugin-svgo'
import fs from "node:fs"
import { minifyTemplates } from 'esbuild-minify-templates';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [viteSingleFile(), ],//removeViteModuleLoader=true for viteSingleFile had no effect on bundle size
  build:{
    minify:true,
    cssCodeSplit:false,
  },
  esbuild:{
    //drop:["console", 'debugger'],
    legalComments:'none',
    
  },
  server:{
    open:"https://protzklotz:5173",
    https:{
      key: fs.readFileSync('../certificates/testserver.pem.key'),
      cert: fs.readFileSync('../certificates/testserver.pem.crt'),
    },
    proxy:{
      "/webmanager_ws":{
        target:"ws://localhost:3000",
        ws:true,
      }
    }
  }
})