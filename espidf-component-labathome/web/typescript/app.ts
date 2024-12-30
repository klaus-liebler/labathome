import { TemplateResult, html, render } from "lit-html";
import "../style/app.css";

import { Ref, createRef, ref } from "lit-html/directives/ref.js";
import { HeaterExperimentController } from "./screen_controller/heater_experiment_controller";

import * as flatbuffers from "flatbuffers";
import { Chatbot } from "./chatbot";
import { DialogController, OkDialog } from "./dialog_controller/dialog_controller";
import { DevelopCFCController } from "./screen_controller/develop_cfc_controller";
import { runCarRace } from "./screen_controller/racinggame_controller";
import { DefaultScreenController, ScreenController } from "./screen_controller/screen_controller";
import { SystemController } from "./screen_controller/system_controller";
import { WifimanagerController } from "./screen_controller/wifimanager_controller";
import { Html, Severity, severity2class, severity2symbol } from "./utils/common";
import { LABBY_URL, WS_URL } from "./utils/constants";
import { IAppManagement, IWebsocketMessageListener } from "./utils/interfaces";
import RouterMenu, { IRouteHandler, Route } from "./utils/routermenu";


class Router2ContentAdapter implements IRouteHandler {
  constructor(public readonly child: ScreenController, public readonly app: AppController) { }
  handleRoute(params: RegExpMatchArray): void {
    console.log("Router2ContentAdapter->handleRoute")
    this.app.SetMain(this.child, params)
  }
}

function AddScreenControllers(app: AppController): void {
  app.AddScreenController("dashboard", new RegExp("^/$"), html`<span>&#127760;</span><span>Home</span>`, DefaultScreenController)
  app.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>🥽</span><span>Function Block</span>`, DevelopCFCController)
  app.AddScreenController("heater", new RegExp("^/heater$"), html`<span>🥽</span><span>Control Heater</span>`, HeaterExperimentController)
  app.AddScreenController("system", new RegExp("^/system$"), html`<span>🧰</span><span>System Settings</span>`, SystemController)
  //app.AddScreenController("properties", new RegExp("^/properties$"), html`<span>⌘</span><span>Properties</span>`, UsersettingsController)
  app.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>📶</span><span>Wifi Manager</span>`, WifimanagerController)

}

class BufferedMessage {
  constructor(public data: Uint8Array, public namespace: number, public maxLockingTimeMs: number) { }
}

class AppController implements IAppManagement {
  private routes: Array<Route> = []
  
  private namespace2listener = new Map<number, Array<IWebsocketMessageListener>>();
  private lockingNamespace:number|null=null;
  private socket: WebSocket | null = null;
  private messageBuffer = new Array<BufferedMessage>();
  private modalSpinner: Ref<HTMLDivElement> = createRef();
  private modalSpinnerTimeoutHandle: number = -1;


  private menu = new RouterMenu("/", this.routes);
  private mainContent: ScreenController = new DefaultScreenController(this);
  private mainRef: Ref<HTMLInputElement> = createRef();
  

  private dialog: Ref<HTMLDivElement> = createRef();
  private snackbarTimeout: number = -1;

  private screenControllers: ScreenController[];
  private chatbot: Chatbot;


  public ShowDialog(d: DialogController) {
    //this.dialog.value!.innerText="";
    render(d.Template(), this.dialog.value!)
    d.Show();
  }

  public RegisterWebsocketMessageNamespace(listener: IWebsocketMessageListener, ...namespaces: number[]): (() => void){
    namespaces.forEach((n) => {
      let arr = this.namespace2listener.get(n)
      if (!arr) {
        arr = []
        this.namespace2listener.set(n, arr)
      }
      arr.push(listener)
    })
    return () => {this.Unregister(listener)}
  }


  public Unregister(listener: IWebsocketMessageListener): void{
    console.info('unregister')
    this.namespace2listener.forEach((v) => {
      v.filter((l) => {
        l != listener
      })
    })
  }
  public WrapAndSend(namespace:number, b:flatbuffers.Builder, maxLockingTimeMs: number=0):void{
    console.log(`Send Request Namespace ${namespace} to server`)
    
    var m=new BufferedMessage(b.asUint8Array(), namespace, maxLockingTimeMs)
    if (!this.socket || this.socket.readyState != this.socket.OPEN) {
      console.info('sendWebsocketMessage --> not OPEN --> buffering')
      this.messageBuffer.push(m);
      return;
    }
    this.sendMessage(m);
  }

  private sendMessage(m:BufferedMessage){
    const bufferLength = 4 + m.data.byteLength;
    const arrayBuffer = new ArrayBuffer(bufferLength);
    const dataView = new DataView(arrayBuffer);
    dataView.setUint32(0, m.namespace);
    const newData = new Uint8Array(arrayBuffer);
    newData.set(m.data, 4);
    
    if(m.maxLockingTimeMs==0){
      this.lockingNamespace=null;
    }else{
      this.lockingNamespace=m.namespace;
      this.setModal(true)
      this.modalSpinnerTimeoutHandle = <number>(<unknown>setTimeout(() => this.modalSpinnerTimeout(), m.maxLockingTimeMs)) //casting to make TypeScript happy
    }

    console.info(`sendWebsocketMessage --> OPEN --> send to server`)
    try {
      this.socket!.send(newData)
    } catch (error: any) {
      this.setModal(false)
      if (this.modalSpinnerTimeoutHandle) {
        clearTimeout(this.modalSpinnerTimeoutHandle)
      }
      this.ShowDialog(new OkDialog(Severity.ERROR, `Error while sending a request to server:${error}`))
    }
  }

  private onWebsocketData(arrayBuffer: ArrayBuffer) {
    if (arrayBuffer.byteLength == 4096) {
      //TODO: it is dumb idea to use the size of the message to test whether it is a raw timeseries message or not. But hopefully, Flatbuffer messages never get that big
      // this.timeseriesScreenController?.onTimeseriesMessage(data);
      // if(this.messagesToUnlock.includes(Responses.ResponseTimeseriesDummy)){
      //   clearTimeout(this.modalSpinnerTimeoutHandle);
      //   this.messagesToUnlock=[Responses.NONE];
      //   this.setModal(false);
      // }
      return
    }
    const dataView = new DataView(arrayBuffer);
    const namespace = dataView.getUint32(0);
    console.log(`A message of namespace ${namespace} with length ${arrayBuffer.byteLength} has arrived.`)
    if (this.lockingNamespace==namespace) {
      clearTimeout(this.modalSpinnerTimeoutHandle)
      this.lockingNamespace = null
      this.setModal(false)
    }
    const arr = new Uint8Array(arrayBuffer, 4);
    let bb = new flatbuffers.ByteBuffer(new Uint8Array(arrayBuffer, 4))
    //let messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb)
    this.namespace2listener.get(namespace)?.forEach((v) => {
      v.OnMessage(namespace, bb)
    })
  }


  public ShowSnackbar(severity: Severity, text: string, timeout = 3000) {
    if (this.snackbarTimeout >= 0) {
      clearInterval(this.snackbarTimeout);
    }
    var snackbar = document.getElementById("snackbar");
    snackbar.innerText = "";
    Html(snackbar, "span", [], [severity2class(severity)], severity2symbol(severity));
    Html(snackbar, "span", [], [], text);
    snackbar.style.visibility = "visible";
    snackbar.style.animation = "fadein 0.5s, fadeout 0.5s 2.5s";
    this.snackbarTimeout = <any>setTimeout(() => {
      snackbar.style.visibility = "hidden";
      snackbar.style.animation = "";
      this.snackbarTimeout = -1;
    }, timeout);
  }


  public SetMain(child: ScreenController, params: RegExpMatchArray) {
    this.mainContent.OnPausePublic();
    this.mainContent = child
    //this.mainRef.value!.innerText=""
    render(this.mainContent.Template(), this.mainRef.value!)
    this.mainContent.OnStartPublic();
    child.SetParameter(params)
  }

  public AddScreenController<T extends ScreenController>(url: string, urlPattern: RegExp, caption: TemplateResult<1>, type: { new(m: IAppManagement): T; }): ScreenController {
    let controllerObject = new type(this);
    var w = new Route(url, urlPattern, caption, new Router2ContentAdapter(controllerObject, this))
    this.routes.push(w)
    controllerObject.OnCreate();
    return controllerObject
  }



  private setModal(state: boolean) {
    this.modalSpinner.value!.style.display = state ? "flex" : "none";
  }

  private modalSpinnerTimeout() {
    this.setModal(false);
    this.ShowDialog(new OkDialog(Severity.ERROR, "Server did not respond"));
  }

  public log(text: string) {
    console.log(text)
  }

  


  private easteregg() {
    document.body.innerText = "";
    document.body.innerHTML = "<canvas id='c'>"
    var el = <HTMLCanvasElement>document.getElementById("c")!
    runCarRace(el);
  }

  
  public Startup(screenControllersBuilder: (app: AppController) => void) {
    screenControllersBuilder(this);
    const Template = html`
            <header style="background-image: url('data:image/svg+xml;charset=utf-8;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnICB3aWR0aD0nNTU5JyBoZWlnaHQ9JzY3LjEnIHZpZXdCb3g9JzAgMCAxMDAwIDEyMCc+PHJlY3QgZmlsbD0nI0ZGRkZGRicgd2lkdGg9JzEwMDAnIGhlaWdodD0nMTIwJy8+PGcgIGZpbGw9J25vbmUnIHN0cm9rZT0nI0M5RUVENicgc3Ryb2tlLXdpZHRoPScxNCcgc3Ryb2tlLW9wYWNpdHk9JzEnPjxwYXRoIGQ9J00tNTAwIDc1YzAgMCAxMjUtMzAgMjUwLTMwUzAgNzUgMCA3NXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjxwYXRoIGQ9J00tNTAwIDQ1YzAgMCAxMjUtMzAgMjUwLTMwUzAgNDUgMCA0NXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjxwYXRoIGQ9J00tNTAwIDEwNWMwIDAgMTI1LTMwIDI1MC0zMFMwIDEwNSAwIDEwNXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjxwYXRoIGQ9J00tNTAwIDE1YzAgMCAxMjUtMzAgMjUwLTMwUzAgMTUgMCAxNXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjxwYXRoIGQ9J00tNTAwLTE1YzAgMCAxMjUtMzAgMjUwLTMwUzAtMTUgMC0xNXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjxwYXRoIGQ9J00tNTAwIDEzNWMwIDAgMTI1LTMwIDI1MC0zMFMwIDEzNSAwIDEzNXMxMjUgMzAgMjUwIDMwczI1MC0zMCAyNTAtMzBzMTI1LTMwIDI1MC0zMHMyNTAgMzAgMjUwIDMwczEyNSAzMCAyNTAgMzBzMjUwLTMwIDI1MC0zMCcvPjwvZz48L3N2Zz4=');"><span @click=${() => this.easteregg()}> Lab@Home WebUI</span></header>
            <nav>${this.menu.Template()}<a href="javascript:void(0);" @click=${() => this.menu.ToggleHamburgerMenu()}><i>≡</i></a></nav>
            <main ${ref(this.mainRef)}></main>
            <footer>Klaus Liebler, &copy;2024 ${import.meta.env.__APP_NAME__}</footer>
            <div ${ref(this.modalSpinner)} class="modal"><span class="loader"></span></div>
            <div id="snackbar">Some text some message..</div>
            <div ${ref(this.dialog)}></div>
            ${this.chatbot.Template()}`
    render(Template, document.body);
    console.log(`Connecting to ${WS_URL}`)
    this.socket = new WebSocket(WS_URL)
    this.socket.binaryType = 'arraybuffer'
    this.socket.onopen = (_event) => {
      console.log(`Websocket is connected.`)
      if (this.messageBuffer.length>0) {
        console.log(`There are ${this.messageBuffer.length} messages in buffer.`)
        for(const m of this.messageBuffer){
          this.sendMessage(m);

        }
      }
      this.messageBuffer=new Array<BufferedMessage>()
    }
    this.socket.onerror = (event: Event) => {
      var message = `Websocket error ${event}`
      console.error(message)
      this.ShowSnackbar(Severity.ERROR, message)
    }
    this.socket.onmessage = (event: MessageEvent<any>) => {
      this.onWebsocketData(event.data)
    }
    this.socket.onclose = (event) => {
      if (event.code == 1000) {
        console.info('The Websocket connection has been closed normally. But why????')
        return
      }
      var message = `Websocket has been closed ${event}`
      console.error(message)
      this.ShowSnackbar(Severity.ERROR, message)
    }
    this.menu.check();
    //this.chatbot.Setup();

  }

  constructor() {
    this.chatbot = new Chatbot();
    
  }
}

let app: AppController;
document.addEventListener("DOMContentLoaded", (e) => {
  app = new AppController();
  app.Startup(AddScreenControllers);
});


