import * as s from "@klaus-liebler/websocket_file_testserver"
import {ChatbotHandler} from "@klaus-liebler/websocket_file_testserver/handlers/chatbot"
import { FunctionblockHandler } from "@klaus-liebler/websocket_file_testserver/handlers/functionblock"
import { HeaterExperimentHandler } from "@klaus-liebler/websocket_file_testserver/handlers/heaterexperiment"
import { SystemHandler } from "@klaus-liebler/websocket_file_testserver/handlers/system"
import { WebmanagerHandler } from "@klaus-liebler/websocket_file_testserver/handlers/webmanager"
import path from "node:path"

s.StartServers(path.join(process.env.USERPROFILE, "netcase", "certificates"), [
    new ChatbotHandler(),
    new FunctionblockHandler("./files/spiffs"),
    new HeaterExperimentHandler(),
    new SystemHandler(),
    new WebmanagerHandler()
])