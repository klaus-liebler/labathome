import http from "node:http"
import * as fs from "node:fs"
import * as flatbuffers from "flatbuffers"
import * as weso  from "ws"
import * as path from "node:path"

import { handleFunctionblock } from "./handlers/functionblock"
import * as functionblock from "./generated/flatbuffers/functionblock"


const PORT = 3000;
const wss = new weso.WebSocketServer({ noServer: true });


function processRequest(buffer: Buffer, ws: weso.WebSocket) {
    var b_req = new flatbuffers.ByteBuffer(new Uint8Array(buffer));
    let ns = new DataView(buffer.buffer).getUint32(0);
    console.log(`Received buffer length ${buffer.byteLength} for Namespace ${ns}`);
    if(ns==functionblock.Namespace.Value){
        handleFunctionblock(b_req, ws)
    }


    
}

//let hostCert =fs.readFileSync("./../certificates/testserver.pem.crt").toString();
//let hostPrivateKey = fs.readFileSync("./../certificates/testserver.pem.prvtkey").toString();





let server = http.createServer((req, res) => {
//let server = https.createServer({key: hostPrivateKey, cert: hostCert}, (req, res) => {
    console.log(`Request received for '${req.url}'`);
    //var local_path = new URL(req.url).pathname;

    if (req.method=="POST" && req.url.startsWith("/files")) {
        const req_body_chunks: any[] = [];
        req.on("data", (chunk: any) => req_body_chunks.push(chunk));
        req.on("end", () => {
            var p = path.join("./", req.url)
            fs.mkdirSync(path.dirname(p.toString()), { recursive: true });
            fs.writeFileSync(p, Buffer.concat(req_body_chunks), {});
            res.writeHead(200, {'Content-Type': 'text/html'});
            res.write('<p>OK</p>');
            res.end();
            //process(res, req.url!, Buffer.concat(req_body_chunks));
        });
    }
    else if (req.method=="GET" && req.url.startsWith("/files")) {
        if(req.url.endsWith("/")){
            res.writeHead(200, {'Content-Type': 'application/json'});
            var ret={files:new Array<string>(), dirs:new Array<string>()}
            for(const f of fs.readdirSync(path.join("./", req.url.slice(0,-1)), {withFileTypes:true})){
                if(f.isFile()) ret.files.push(f.name);
                if(f.isDirectory()) ret.dirs.push(f.name);
            }
            res.write(JSON.stringify(ret));
            res.end();
        }else{
            try{
                var p = path.join("./", req.url)
                var b =fs.readFileSync(p);
                res.writeHead(200, {'Content-Type': 'application/octet-stream'});
                res.write(b);
                res.end();
            }
            catch (error) {
                res.writeHead(500);
                res.write("Resource not available");
                res.end();
            } 
        }
    } else {
        console.log(`Request unknwon '${req.url}'`);
        res.writeHead(404);
        res.end("Not found");
    }
});

server.on('upgrade', (req, sock, head) => {
    if (req.url == '/webmanager_ws') {
        console.info("Handle upgrade to websocket");
        wss.handleUpgrade(req, sock, head, ws => wss.emit('connection', ws, req));
    } else {
        sock.destroy();
    }
});

var messageChanger = 0;

wss.on('connection', (ws:weso.WebSocket) => {
    console.info("Handle connection");
    ws.on('error', console.error);

    ws.on('message', (data: Buffer, isBinary: boolean) => {
        
        processRequest(data, ws);
    });
});

server.on("error",(e)=>{
    console.error(e);
})

server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
    const interval = setInterval(() => {
        let b = new flatbuffers.Builder(1024);
        let mw: number;
        switch (messageChanger) {
            case 0:
                
                break;
            default:
                break;
        }
        messageChanger++;
        messageChanger %= 3;

    }, 1000);

});




