
import * as flatbuffers from "flatbuffers";
import { DialogController } from "../dialog_controller/dialog_controller";
import { Severity } from "./common";

export interface IDialogController____ {
    ShowDialog(pHandler?: ((ok: boolean, value: string) => any)): void;
};

export interface IDialogBodyRenderer {
    Render(dialogBody: HTMLElement): HTMLInputElement | null;
}

export interface IHtmlRenderer {
    RenderStatic(c: HTMLElement): void;
}

export interface IWebsocketMessageListener {
    OnMessage(namespace:number, byteBuffer: flatbuffers.ByteBuffer): void;
  }

export interface IAppManagement {
    RegisterWebsocketMessageNamespace(listener: IWebsocketMessageListener, namespace: number): (() => void);
    Unregister(listener: IWebsocketMessageListener): void;
    WrapAndSend(namespace:number, b:flatbuffers.Builder, maxlockingTimeMs?: number);
    ShowSnackbar(severity: Severity, text: string): void;
    ShowDialog(dialogController: DialogController): void;
};