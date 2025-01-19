import { RequestGetUserSettings, RequestSetUserSettings, RequestWrapper, Requests, ResponseGetUserSettings, ResponseSetUserSettings, ResponseWrapper, Responses } from "../../generated/flatbuffers/webmanager";
import { ScreenController } from "./screen_controller";
import * as flatbuffers from 'flatbuffers';
import us from "../../generated/usersettings/usersettings"
import { BooleanItemRT, ConfigGroup, ConfigItemRT, EnumItemRT, IntegerItemRT, StringItemRT, ValueUpdater } from "../utils/usersettings_base";
import { TemplateResult, html, render } from "lit-html";
import { Severity } from "../utils/common";
import { Ref, createRef, ref } from "lit-html/directives/ref.js";
import { IAppManagement } from "../utils/interfaces";


class ConfigGroupRT{

    private panelOpen=false;
    private dataDirty=false;
    public spanArrowContainer:Ref<HTMLElement> = createRef();
    public divPanel:Ref<HTMLTableSectionElement> = createRef();
    public btnOpenClose:Ref<HTMLElement> = createRef();
    public btnSave:Ref<HTMLButtonElement> = createRef();
    public btnUpdate:Ref<HTMLButtonElement> = createRef();
    public btnReset:Ref<HTMLButtonElement> = createRef();
    constructor(private readonly groupCfg: ConfigGroup, private readonly appManagement:IAppManagement, private readonly itemKey2configItemRT:Map<string, ConfigItemRT>){}

    private Template=(itemTemplates:Array<TemplateResult<1>>)=>{
        return html`
    <div class="accordion">
        <button ${ref(this.btnOpenClose)} @click=${(e)=>this.onBtnOpenCloseClicked(e)} style="display: flex; ">
            <span ${ref(this.spanArrowContainer)}>▶</span>
            <span style="flex-grow: 1;">${this.groupCfg.displayName}</span>
            <input ${ref(this.btnSave)} @click=${(e:MouseEvent)=>this.onBtnSaveClicked(e)} disabled type="button" value="💾 Save Changes" />
            <input ${ref(this.btnUpdate)} @click=${(e:MouseEvent)=>this.onBtnUpdateClicked(e)} type="button" value=" ⟳ Fetch Values from Server" />
            <input ${ref(this.btnReset)} type="button" value=" 🗑 Reset Values" />
        </button>
        <div ${ref(this.divPanel)}>
            <table>
                <thead>
                    <tr><th>Name</th><th></th><th style='width: 100%;'>Value</th></tr>
                </thead>
                <tbody>${itemTemplates}</tbody>
            </table>
            
        </div>
    </div>
    `}
    private sendRequestGetUserSettings() {
        let b = new flatbuffers.Builder(256);
        this.appManagement.WrapAndFinishAndSend(b,
            Requests.RequestGetUserSettings,
            RequestGetUserSettings.createRequestGetUserSettings(b, b.createString(this.groupCfg.Key)),
            [Responses.ResponseGetUserSettings]
        );
    }

    private sendRequestSetUserSettings() {
        let b = new flatbuffers.Builder(1024);
        let vectorOfSettings:number[]=[];
        for(let v of this.itemKey2configItemRT!.values()){
            if(!v.HasAChangedValue()) continue;
            vectorOfSettings.push(v.WriteToFlatbufferBufferAndReturnSettingWrapperOffset(b));
        }
        this.appManagement.WrapAndFinishAndSend(b,
            Requests.RequestSetUserSettings,
            RequestSetUserSettings.createRequestSetUserSettings(b, b.createString(this.groupCfg.Key),
                ResponseGetUserSettings.createSettingsVector(b, vectorOfSettings)
            ),
            [Responses.ResponseSetUserSettings]
        );
    }

    private onBtnOpenCloseClicked(e:MouseEvent){
        this.panelOpen=!this.panelOpen;
        this.btnOpenClose.value!.classList.toggle("active");
        if (this.panelOpen) {
            this.divPanel.value!.style.display = "block";
            this.spanArrowContainer.value!.textContent="▼";
            this.sendRequestGetUserSettings();
        } else {
            this.divPanel.value!.style.display = "none";
            this.spanArrowContainer.value!.textContent="▶";
        }
        e.stopPropagation();
    }

    private onBtnSaveClicked(e:MouseEvent){
        this.sendRequestSetUserSettings();
        e.stopPropagation();
    }

    private onBtnUpdateClicked(e:MouseEvent){
        this.sendRequestGetUserSettings();
        e.stopPropagation()
    }
    public BuildRtAndRender(templates:Array<TemplateResult<1>>, updater:ValueUpdater) {
        var itemTemplates:Array<TemplateResult<1>>=[];
        for(const item of this.groupCfg.items){
            var configItemRt = item.BuildConfigItemRt(this.groupCfg.displayName, updater)
            itemTemplates.push(configItemRt.OverallTemplate());
            this.itemKey2configItemRT.set(item.Key, configItemRt);
        }
        templates.push(this.Template(itemTemplates));
        return this.divPanel.value!;
    }
}

export class UsersettingsController extends ScreenController implements ValueUpdater{
    private mainElement:Ref<HTMLElement>= createRef();
    public Template = () => html`<h1>User Settings</h1><section ${ref(this.mainElement)}></section>`

    UpdateSaveButton(groupKey:string){
        let group=this.groupKey2itemKey2configItemRT.get(groupKey);
        let atLeastOneHasChanged=false;
        for(let v of group!.values()){
            if(v.HasAChangedValue()){
                atLeastOneHasChanged=true;
                break;
            }
        }
        let gc=this.groupKey2configGroupRT.get(groupKey)!;
        gc.btnSave.value!.disabled=!atLeastOneHasChanged;
    }

    UpdateString(groupName:string, i:StringItemRT, v:string): void {
        console.log(`${i.displayName}=${v}`);
        this.UpdateSaveButton(groupName);
       
    }
    UpdateInteger(groupName:string, i:IntegerItemRT, v: number): void {
        console.log(`${i.displayName}=${v}`);
        this.UpdateSaveButton(groupName);
    }

    UpdateBoolean(groupName:string, i: BooleanItemRT, value: boolean): void {
        console.log(`${i.displayName}=${value}`);
        this.UpdateSaveButton(groupName);
    }
    UpdateEnum(groupName:string, i: EnumItemRT, selectedIndex: number): void {
        console.log(`${i.displayName}=${selectedIndex}`);
        this.UpdateSaveButton(groupName);
    }

    private groupKey2itemKey2configItemRT = new Map<string, Map<string,ConfigItemRT>>();
    private groupKey2configGroupRT = new Map<string, ConfigGroupRT>();
    private cfg:ConfigGroup[]=[];

    public onMessage(messageWrapper: ResponseWrapper): void {
        switch (messageWrapper.responseType()) {
            case Responses.ResponseGetUserSettings:
                this.onResponseGetUserSettings(messageWrapper);
                break;
            case Responses.ResponseSetUserSettings:
                this.onResponseSetUserSettings(messageWrapper);
                break;
            default:
                break;
        }
    }
    public onResponseSetUserSettings(messageWrapper: ResponseWrapper): void{
        let resp = <ResponseSetUserSettings>messageWrapper.response(new ResponseSetUserSettings());
        let groupRtMap=this.groupKey2itemKey2configItemRT.get(resp.groupKey()!);
        if(!groupRtMap){
            this.appManagement.showOKDialog(Severity.WARN, `Received settings for unknown group index ${resp.groupKey()}`);
            return;
        }
        groupRtMap.forEach((v,_k,_m)=>{v.Flag=false});
        let unknownKeys:string[]=[];
        for (let i = 0; i < resp.settingKeysLength(); i++) {
            let key = resp.settingKeys(i);
            let itemRt = groupRtMap.get(key);
            if(!itemRt){
                unknownKeys.push(key);
                continue;
            }
            itemRt.ConfirmSuccessfulWrite();
            itemRt.Flag=true;
        }
        let nonStoredEntryKeys:string[]=[];
        groupRtMap.forEach((v,_k,_m)=>{
            if(!v.Flag){
                nonStoredEntryKeys.push(v.displayName);
            }
        });
        if(unknownKeys.length!=0 || nonStoredEntryKeys.length!=0){
            this.appManagement.showOKDialog(Severity.WARN, `The following errors occured while receiving data for ${resp.groupKey()}: Unknown names: ${unknownKeys.join(", ")}; No successful storage for: ${nonStoredEntryKeys.join(", ")};`);
        }
        groupRtMap.forEach((v,_k,_m)=>{v.Flag=false});
    }
    
    public onResponseGetUserSettings(messageWrapper: ResponseWrapper): void{
        let resp = <ResponseGetUserSettings>messageWrapper.response(new ResponseGetUserSettings());
        let itemKey2item=this.groupKey2itemKey2configItemRT.get(resp.groupKey()!);
        if(!itemKey2item){
            this.appManagement.showOKDialog(Severity.WARN, `Received settings for unknown group index ${resp.groupKey()}`);
            return;
        }
        itemKey2item.forEach((v,_k,_m)=>{v.Flag=false});
        let unknownKeys:string[]=[];
        for (let i = 0; i < resp.settingsLength(); i++) {
            let itemKey = resp.settings(i)!.settingKey()!;
            
            let itemRt = itemKey2item.get(itemKey)!;
            if(!itemRt){
                unknownKeys.push(itemKey);
                continue;
            }
            itemRt.ReadFlatbuffersObjectAndSetValueInDom(resp.settings(i)!);
            itemRt.Flag=true;
        }
        let nonUpdatedEntries:string[]=[];
        itemKey2item.forEach((v,_k,_m)=>{
            if(!v.Flag){
                nonUpdatedEntries.push(v.displayName);
                v.NoDataFromServerAvailable();
            }
        });
        if(unknownKeys.length!=0 || nonUpdatedEntries.length!=0){
            this.appManagement.showOKDialog(Severity.WARN, `The following errors occured while receiving data for ${resp.groupKey()}: Unknown keys: ${unknownKeys.join(", ")}; No updates for: ${nonUpdatedEntries.join(", ")};`);
        }
    }
    
    
    


    onCreate(): void {
        this.appManagement.registerWebsocketMessageTypes(this, Responses.ResponseGetUserSettings, Responses.ResponseSetUserSettings);
        this.cfg = us.Build();
       
    }

    private onStart_or_onRestart(){
        var templates:Array<TemplateResult<1>>=[]
        this.cfg.forEach((groupCfg, _groupIndex)=>{
            let itemDisplayName2configItemRT= new Map<string, ConfigItemRT>();
            var groupRT = new ConfigGroupRT(groupCfg, this.appManagement, itemDisplayName2configItemRT);
            groupRT.BuildRtAndRender(templates, this);
            this.groupKey2configGroupRT.set(groupCfg.Key, groupRT);
            this.groupKey2itemKey2configItemRT.set(groupCfg.Key, itemDisplayName2configItemRT);
        });
        render(templates, this.mainElement.value!)
    }
   
    onFirstStart(): void {
        this.onStart_or_onRestart();
    }
    onRestart(): void {
        this.onStart_or_onRestart();
    }
    onPause(): void {
    }

}
