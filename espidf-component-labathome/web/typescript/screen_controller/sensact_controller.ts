import BuildApps from "../../generated/sensact/sensactapps";
import { ScreenController } from "./screen_controller";
import { TemplateResult, html, render } from "lit-html";
import { Ref, createRef, ref } from "lit-html/directives/ref.js";
import { ApplicationGroup, SensactApplication } from "../utils/sensactapps_base";
import * as flatbuffers from 'flatbuffers';

import bed from '../../svgs/solid/bed.svg?raw'
import lightbulb from '../../svgs/solid/lightbulb.svg?raw'
import { unsafeSVG } from "lit-html/directives/unsafe-svg.js";
import { GetLevelFromApplicationId, GetRoomFromApplicationId, GetTechnologyFromApplicationId } from "../utils/sensact";

import "../utils/extensions";
import { Namespace, NotifyStatus, Requests, ResponseCommand, Responses, ResponseStatus, ResponseWrapper } from "../../generated/flatbuffers/sensact";
import { ISensactContext } from "../utils/interfaces_sensact";

export class SensactController extends ScreenController implements ISensactContext {
    WrapAndFinishAndSend(b: flatbuffers.Builder, message_type: Requests, message: flatbuffers.Offset, messagesToUnlock?: Array<Responses>, maxWaitingTimeMs?: number) {
        b.finish(message);
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, maxWaitingTimeMs);
    }

    private groups: Array<ApplicationGroup>;

    private btnSortTechnology() {
        var tech2apps = new Map<string, Array<SensactApplication>>();
        for (const app of this.sensactApps.values()) {
            var k = GetTechnologyFromApplicationId(app.applicationId);
            var arr = tech2apps.getOrAdd(k, () => new Array<SensactApplication>());
            arr.push(app);
        }
        var sortedMap = new Map([...tech2apps.entries()].sort((a, b) => a[0].localeCompare(b[0])));
        this.groups = [];
        sortedMap.forEach((v, k) => {
            this.groups.push(new ApplicationGroup(k, this.appManagement, v, k));
        });
        this.execTemplates();
    }

    private btnSortRooms() {
        var level_room2apps = new Map<string, Array<SensactApplication>>();
        for (const app of this.sensactApps.values()) {
            var room_level = GetRoomFromApplicationId(app.applicationId) + "_" + GetLevelFromApplicationId(app.applicationId);
            var arr = level_room2apps.getOrAdd(room_level, () => new Array<SensactApplication>());
            arr.push(app);
        }
        var sortedMap = new Map([...level_room2apps.entries()].sort((a, b) => a[0].localeCompare(b[0])));
        this.groups = [];
        sortedMap.forEach((v, k) => {
            this.groups.push(new ApplicationGroup(k, this.appManagement, v, k));
        });
        this.execTemplates();
    }

    private execTemplates() {
        var templates = new Array<TemplateResult<1>>();
        this.groups.forEach((group) => {
            templates.push(group.Template());
        });
        render(templates, this.mainElement.value!);
    }

    private mainElement: Ref<HTMLElement> = createRef();
    public Template = () => html`
    <h1>Sensact Controls</h1>
        
    <div class="buttons">
        <button class="withsvg" @click=${() => this.btnSortRooms()}>${unsafeSVG(bed)}<span>Sort Rooms<span></button>
        <button class="withsvg" @click=${() => this.btnSortTechnology()}>${unsafeSVG(lightbulb)}<span>Sort Tech<span></button>
    </div>
    <section ${ref(this.mainElement)}></section>`;

    private sensactApps: Map<number, SensactApplication>;

    OnMessage(namespace: number, bb: flatbuffers.ByteBuffer): void {
        if (namespace != Namespace.Value) return;
        var messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb);
        switch (messageWrapper.responseType()) {
            case Responses.ResponseCommand:
                this.onResponseCommand(<ResponseCommand>messageWrapper.response(new ResponseCommand()));
                break;
            case Responses.NotifyStatus:
                this.onNotifyStatus(<NotifyStatus>messageWrapper.response(new NotifyStatus()));
                break;
            case Responses.ResponseStatus:
                this.onResponseStatus(<ResponseStatus>messageWrapper.response(new ResponseStatus()));
                break;
            default:
                break;
        }
        this.execTemplates();
    }

    private onResponseCommand(m: ResponseCommand) {
        console.log("Command confirmed");
    }

    private onNotifyStatus(m: NotifyStatus) {
        var app = this.sensactApps.get(m.id());
        if (!app) {
            console.warn(`Unknown app with id ${m.id()}`);
            return;
        }
        app.UpdateState(m.status());
    }

    private onResponseStatus(m: ResponseStatus) {
        for (var i = 0; i < m.statesLength(); i++) {
            var app = this.sensactApps.get(m.states(i).id());
            if (!app) {
                console.warn(`Unknown app with id ${m.states(i).id()}`);
                continue;
            }
            app.UpdateState(m.states(i).status());
        }
    }

    OnCreate(): void {
        this.appManagement.RegisterWebsocketMessageNamespace(this, Namespace.Value);
        this.sensactApps = new Map<number, SensactApplication>(BuildApps(this.appManagement).map(v => [v.applicationId, v]));
    }

    private onStart_or_onRestart() {
        this.btnSortRooms();
    }

    OnFirstStart(): void {
        this.onStart_or_onRestart();
    }

    OnRestart(): void {
        this.onStart_or_onRestart();
    }

    OnPause(): void {
    }
}
