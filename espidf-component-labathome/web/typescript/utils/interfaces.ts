
import * as flatbuffers from "flatbuffers";
import { DialogController } from "../dialog_controller/dialog_controller";
import { Severity } from "./common";
import { TemplateResult } from "lit-html";
import { ScreenController } from "../screen_controller/screen_controller";

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
export interface IScreenControllerHost {
   AddScreenController(url: string, urlPattern: RegExp, caption: TemplateResult<1>, controller:ScreenController);
}

export interface IAppManagement {
    RegisterWebsocketMessageNamespace(listener: IWebsocketMessageListener, namespace: number): (() => void);
    Unregister(listener: IWebsocketMessageListener): void;
    SendFinishedBuilder(namespace:number, b:flatbuffers.Builder, maxlockingTimeMs?: number);
    ShowSnackbar(severity: Severity, text: string): void;
    ShowDialog(dialogController: DialogController): void;
};