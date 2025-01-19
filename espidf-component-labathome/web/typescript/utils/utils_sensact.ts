
import * as flatbuffers from 'flatbuffers';
import { ApplicationId } from '../../generated/flatbuffers/application-id';
import { Command } from '../../generated/flatbuffers/command';

import { ISensactContext } from './interfaces';
import { RequestCommand } from '../../generated/flatbuffers/websensact/request-command';
import { Requests, Responses } from '../../generated/flatbuffers/webmanager';


export async function sendCommandMessage(id: ApplicationId, cmd: Command, payload: Uint8Array, ctx:ISensactContext) {
    let view = new DataView(payload.buffer, 0);
    let b = new flatbuffers.Builder(1024);
    ctx.WrapAndFinishAndSend(b, Requests.websensact_RequestCommand, RequestCommand.createRequestCommand(b, id, cmd, view.getBigUint64(0, true)), [Responses.websensact_ResponseCommand], 3000);
}