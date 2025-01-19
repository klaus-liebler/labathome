import { Ref, createRef, ref } from "lit-html/directives/ref.js";
import { Namespace, OneWeekIn15Minutes, OneWeekIn15MinutesData, RequestSchedulerDelete, RequestSchedulerList, RequestSchedulerOpen, RequestSchedulerRename, RequestSchedulerSave, RequestWrapper, Requests, ResponseSchedulerList, ResponseSchedulerListItem, ResponseSchedulerOpen, ResponseWrapper, Responses, Schedule, SunRandom, eSchedule, uSchedule } from "../../generated/flatbuffers/scheduler";
import { ScreenController } from "./screen_controller";
import * as flatbuffers from 'flatbuffers';
import { TemplateResult, html, render } from "lit-html";

import calendarPlus from '../../svgs/regular/calendar-plus.svg?raw'
import rotate from '../../svgs/solid/rotate.svg?raw'
import { unsafeSVG } from "lit-html/directives/unsafe-svg.js";
import { IAppManagement } from "../utils/interfaces";
import { DialogController, FilenameDialog } from "../dialog_controller/dialog_controller";
import { numberArray2HexString } from "../utils/common";

const weekdays = ["Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"];
const startHour=6;
enum MarkingMode{TOGGLE,ON,OFF};

export abstract class ScheduleItem {

    constructor(public readonly name:string, protected readonly type:string, protected readonly appManagement:IAppManagement) { }
    
    public OverallTemplate=()=>html`
    <tr>
        <td class="minwidth">${this.name}</td>
        <td class="minwidth">${this.type}</td>
        <td>${this.CoreEditTemplate()}</td>
    </tr>
    `
    protected abstract CoreEditTemplate:()=>TemplateResult<1>;
    public abstract OnResponseSchedulerOpen(m:ResponseSchedulerOpen):void;
    public abstract OnCreate():void;
    public abstract SaveToServer():void;
}

export class PredefinedSchedule extends ScheduleItem{
    public OnCreate(): void {
        throw new Error("PredefinedSchedule may not be created");
    }
    constructor(name:string, appManagement:IAppManagement){
        super(name, "Predefined", appManagement);
    }

    public OnResponseSchedulerOpen(m:ResponseSchedulerOpen):void{
        return;
    }

    protected CoreEditTemplate=()=>html``

    public SaveToServer():void{
        return
    }
}

export class SunRandomSchedule extends ScheduleItem implements iSunRandomDialogHandler{
    private offsetMinutes: number;
    private randomMinutes: number;
    constructor(name:string, appManagement:IAppManagement){
        super(name, "SunRandom", appManagement);
    }

    public SaveToServer():void{
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerSave, RequestSchedulerSave.createRequestSchedulerSave(b, 
                    Schedule.createSchedule(b,
                        b.createString(this.name), 
                        uSchedule.SunRandom,
                        SunRandom.createSunRandom(b,
                            this.offsetMinutes,
                            this.randomMinutes
                        )
                    )
                )
            ),
        );
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);
    }

    public OnResponseSchedulerOpen(m:ResponseSchedulerOpen):void{
       
        if(m.payload().scheduleType()!=uSchedule.SunRandom)
            return;
        if(m.payload().name()!=this.name){
            console.error("m.payload().name()!=this.name")
            return;
        }
        var rso = <SunRandom>m.payload().schedule(new SunRandom())
        this.offsetMinutes=rso.offsetMinutes();
        this.randomMinutes=rso.randomMinutes();
        this.appManagement.ShowDialog(new SunRandomScheduleEditorDialog(this.name, this.offsetMinutes, this.randomMinutes, this))
    }

    public OnCreate(): void {
        this.offsetMinutes=15
        this.randomMinutes=15
        this.SaveToServer()
    }

    public handleSunRandomDialog(ok: boolean, offsetMinutes: number, randomMinutes: number){
        console.info("In SunRandomSchedule: Dialog closed");
        if(!ok) return;
        this.offsetMinutes=offsetMinutes;
        this.randomMinutes=randomMinutes;
        this.SaveToServer()
    }

    private btnEditClicked() {
        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerOpen, RequestSchedulerOpen.createRequestSchedulerOpen(b, b.createString(this.name), eSchedule.SunRandom)));    
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);    
        
    }

    private btnDeleteClicked() {
        let b = new flatbuffers.Builder(256);   
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerDelete, RequestSchedulerDelete.createRequestSchedulerDelete(b, b.createString(this.name))));    
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);    
        
    }

    private handleRenameDialog(ok:boolean, newName:string){
        if(!ok) return;
        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerRename, RequestSchedulerRename.createRequestSchedulerRename(b, b.createString(this.name), b.createString(newName))));   
        this.appManagement.SendFinishedBuilder(Namespace.Value, b); 
        
    }

    private btnRenameClicked() {
        this.appManagement.ShowDialog(new FilenameDialog("Enter new name", (ok, value)=>this.handleRenameDialog(ok, value)));
    }

    protected CoreEditTemplate=()=>html`<button @click=${()=>this.btnEditClicked()}>Edit</button><button @click=${()=>this.btnDeleteClicked()}>Delete</button><button @click=${()=>this.btnRenameClicked()}>Rename</button>`
}

export class OneWeekIn15MinutesSchedule extends ScheduleItem implements iWeeklyScheduleDialogHandler{
    private value:Array<number>=[]
    
    constructor(name:string, appManagement:IAppManagement){
        super(name, "OneWeekIn15Minutes", appManagement);
    }
    
       

    public OnResponseSchedulerOpen(m:ResponseSchedulerOpen|null):void{
        if(m.payload().scheduleType()!=uSchedule.OneWeekIn15Minutes)
            return;
        if(m.payload().name()!=this.name){
            console.error("m.payload().name()!=this.name")
            return;
        }
        this.value = new Array<number>(84);
        var rso = <OneWeekIn15Minutes>m.payload().schedule(new OneWeekIn15Minutes())
        for(var i=0;i<84;i++){this.value[i]=rso.data()!.v(i)!}
        this.appManagement.ShowDialog(new WeeklyScheduleDialog(`Weekly Schedule ${this.name}`, this.value, this, null));
    }

    public OnCreate(): void {
        this.value = new Array<number>(84).fill(0xFF)
        this.SaveToServer()
    }

    public handleWeeklyScheduleDialog(ok: boolean, referenceHandle: any, value: Array<number>) {
        if(!ok) return
        console.info(`In OneWeekIn15MinutesSchedule: WeeklyScheduleDialog closed with ok, data = ${numberArray2HexString(value)}`)
        this.value=value
        this.SaveToServer()
    }

    public SaveToServer():void{
        let b = new flatbuffers.Builder(1024)
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerSave, 
            RequestSchedulerSave.createRequestSchedulerSave(b, Schedule.createSchedule(b,
                b.createString(this.name), 
                uSchedule.OneWeekIn15Minutes,
                OneWeekIn15Minutes.createOneWeekIn15Minutes(b,
                    OneWeekIn15MinutesData.createOneWeekIn15MinutesData(b, this.value)
                    )
                )
            )
        )
        );
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);
    }

    private btnEditClicked(e:MouseEvent){

        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerOpen, RequestSchedulerOpen.createRequestSchedulerOpen(b, b.createString(this.name), eSchedule.OneWeekIn15Minutes)));   
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);     
        
    }

    private btnDeleteClicked(_e:MouseEvent) {
        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerDelete, RequestSchedulerDelete.createRequestSchedulerDelete(b, b.createString(this.name))));    
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);     
        
    }

    private handleRenameDialog(ok:boolean, newName:string){
        if(!ok) return;
        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerRename, RequestSchedulerRename.createRequestSchedulerRename(b, b.createString(this.name), b.createString(newName))));  
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);   
        
    }

    private btnRenameClicked(_e:MouseEvent) {
        this.appManagement.ShowDialog(new FilenameDialog("Enter new name", (ok, value)=>this.handleRenameDialog(ok, value)))
    }

    protected CoreEditTemplate=()=>html`<button @click=${(e:MouseEvent)=>this.btnEditClicked(e)}>Edit</button><button @click=${(e:MouseEvent)=>this.btnDeleteClicked(e)}>Delete</button><button @click=${(e:MouseEvent)=>this.btnRenameClicked(e)}>Rename</button>`
}

export class CreateNewScheduleDialog extends DialogController {

    protected inputName: Ref<HTMLInputElement> = createRef();
    protected selectScheduleType: Ref<HTMLSelectElement> = createRef();
    private isValid=false;

    constructor(private alreadyUsedScheduleNames: Array<string>, private m:IAppManagement, protected handler: ((ok: boolean, scheduleItem: ScheduleItem|null) => any) | undefined) {
        super();
        
    }

    protected cancelHandler() {
        this.dialog.value!.close('Cancel');
        this.handler?.(false, null);
    }

    protected okHandler() {
        if(!this.isValid) return;
        this.dialog.value!.close('Ok');
        switch(this.selectScheduleType.value!.selectedIndex){
            case 0:
                this.handler?.(true, new OneWeekIn15MinutesSchedule(this.inputName.value!.value, this.m));
                break;
            case 1:
                this.handler?.(true, new SunRandomSchedule(this.inputName.value!.value, this.m));
                break;
        }
        
    }

    protected backdropClickedHandler(e: MouseEvent) {
        if (e.target === this.dialog) {
            this.cancelHandler();
        }
    }

    private validate(){
        if (this.inputName.value!.validity.patternMismatch) {
            this.inputName.value!.setCustomValidity("Already used names may not be used again");
            this.inputName.value!.reportValidity();
            this.isValid=false;
        }
        else if(this.inputName.value!.validity.tooShort){
            this.inputName.value!.setCustomValidity("Too short!");
            this.inputName.value!.reportValidity();
            this.isValid=false;
        } else {
            this.inputName.value!.setCustomValidity("");
            this.isValid=true;
        }
    }

    public Template() {
        const escapedWords = this.alreadyUsedScheduleNames.map(word => {
            // Escape special characters in each word
            return word.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
        });
    
        // Join the escaped words with '|' and wrap in a negative lookahead
        const pattern = `^(?!.*\\b(?:${escapedWords.join('|')})\\b).*$`
      
        return html`
    <dialog @cancel=${() => this.cancelHandler()} @click=${(e: MouseEvent) => this.backdropClickedHandler(e)} ${ref(this.dialog)}>
        <header>
            <span>Create New Schedule</span>
            <button @click=${() => this.cancelHandler()} type="button">&times;</button>
        </header>
        <main>
            <table>
                <tr><td>Name</td><td><input ${ref(this.inputName)} @input=${()=>this.validate()} type="text" placeholder="Enter Name" required minlength=3 pattern=${pattern} /></td></tr>
                <tr><td>Type</td><td><select ${ref(this.selectScheduleType)}><option>WeeklyQuarterHour</option><option>SunRandom</option></select></td></tr>
            </table>
        </main>
        <footer><input @click=${() => this.okHandler()} type="button" value="OK"></input><input @click=${() => this.cancelHandler()} type="button" value="Cancel"></input></footer>
    </dialog>`;
    }

}

export class SunRandomScheduleEditorDialog extends DialogController {

    protected inputOffset: Ref<HTMLInputElement> = createRef();
    protected inputRandom: Ref<HTMLInputElement> = createRef();

    constructor(protected nameOfSchedule: string, private offsetMinutes: number, private randomMinutes: number, protected handler: iSunRandomDialogHandler) {
        super();
    }

    protected cancelHandler() {
        this.dialog.value!.close('Cancel');
        this.handler.handleSunRandomDialog(false, 0, 0);
    }

    protected okHandler() {
        this.dialog.value!.close('Ok');
        this.handler.handleSunRandomDialog(true, this.inputOffset.value!.valueAsNumber, this.inputRandom.value!.valueAsNumber);
    }

    protected backdropClickedHandler(e: MouseEvent) {
        if (e.target === this.dialog) {
            this.cancelHandler();
        }
    }

    public Template() {
        return html`
    <dialog @cancel=${() => this.cancelHandler()} @click=${(e: MouseEvent) => this.backdropClickedHandler(e)} ${ref(this.dialog)}>
        <header>
            <span>Edit SunRandom Schedule "${this.nameOfSchedule}"</span>
            <button @click=${() => this.cancelHandler} type="button">&times;</button>
        </header>
        <main>
            <label><input ${ref(this.inputOffset)} type="number" value=${this.offsetMinutes} />Offset [minutes]</label>
            <label><input ${ref(this.inputRandom)} type="number" value=${this.randomMinutes} />Random [minutes]</label>
        </main>
        <footer><input @click=${() => this.okHandler()} type="button" value="OK"></input><input @click=${() => this.cancelHandler()} type="button" value="Cancel"></input></footer>
    </dialog>`;
    }

}

export interface iWeeklyScheduleDialogHandler{
    handleWeeklyScheduleDialog(ok: boolean, referenceHandle:any, value: Array<number>);
}

export interface iSunRandomDialogHandler{
    handleSunRandomDialog(ok: boolean, offsetMinutes: number, randomMinutes: number);
}

export class WeeklyScheduleDialog extends DialogController {
    
    
    constructor(private header:string, private initialValue: Array<number>|null, private handler:iWeeklyScheduleDialogHandler, private referenceHandle:any){
        super()
    }

    private isSelecting = false;
    private markingMode:MarkingMode=MarkingMode.ON
    private tblBody:Ref<HTMLTableSectionElement>= createRef();

    protected okHandler() {
        
        var arr = new Array<number>(84);
        for(const d of [6,5,4,3,2,1,0]){
            for(var hour=0;hour<24;hour+=2){
                var value=0;
                for(var quarterhour=0;quarterhour<8;quarterhour++){
                    var sourceMarked=this.isSelected(d, ((hour+24-startHour)*4+quarterhour)%(4*24));
                    value=value | (sourceMarked?1:0);
                    value<<=1 //MSB ist dann immer die erste viertelstunde im 2h-Interval-byte
                }
                value>>=1
                arr[d*12+(hour/2)]=value;
            }
        }
        this.initialValue=arr;
        console.log(`Save clicked. Value=${numberArray2HexString(arr)}`)
        this.dialog.value!.close("Ok");
        this.handler.handleWeeklyScheduleDialog(true, this.referenceHandle, arr);
    }
   
    public Show(){
        if(this.initialValue){
            console.log(`Dialog opened. Value=${numberArray2HexString(this.initialValue)}`)
            for(var d=0;d<7;d++){
                for(var hour=0;hour<24;hour+=2){
                    var value=this.initialValue[d*12+(hour/2)];
                    for(var quarterhour=0;quarterhour<8;quarterhour++){
                        var shouldBeMarked=value & (0b10000000>>quarterhour)
                        this.setSelected(d, ((hour+24-startHour)*4+quarterhour)%(4*24), shouldBeMarked>0);
                    }
                }
            }
        }else{
            this.setAll(false);
        }
        var radios= <NodeListOf<HTMLInputElement>>document.getElementsByName("mode")
        radios[1].checked=true;
        this.dialog.value!.showModal();
    }

    private tdMousedown(e: MouseEvent) {
        //console.log(`mousedown @ ${(<HTMLElement>e.target).innerText}`)
        this.isSelecting = true;
        this.tdMouseenter(e)
        e.preventDefault(); // Verhindert die Textauswahl
    }

    

    private tdMouseenter(e: MouseEvent) {
        //console.log(`mouseenter @ ${(<HTMLElement>e.target).innerText}`)
        if (!this.isSelecting) return;
        switch(this.markingMode){
            case MarkingMode.TOGGLE:(<HTMLElement>e.target).classList.toggle('selected');break;
            case MarkingMode.ON:(<HTMLElement>e.target).classList.add('selected');break;
            case MarkingMode.OFF:(<HTMLElement>e.target).classList.remove('selected');break;
        }
    }

    private tdMouseup(e: MouseEvent) {
        //console.log(`mouseup @ ${(<HTMLElement>e.target).innerText}`)
        this.isSelecting = false;
    }

    private rdoChange(e:MouseEvent){
        var value:string =(<HTMLInputElement>e.target).value
        this.markingMode=MarkingMode[value];
    }

    private copy(sourceDay:number, destinationDays:Array<number>){
        
        for(var fifteen_minutes_slot=0;fifteen_minutes_slot<4*24;fifteen_minutes_slot++){
            var sourceMarked=this.isSelected(sourceDay, fifteen_minutes_slot);
            for(const d of destinationDays){
                this.setSelected(d, fifteen_minutes_slot, sourceMarked);
            }
        }
    }

    private setAll(selected:boolean) {
        
        for(var d=0;d<weekdays.length;d++){
            for(var fifteen_minutes_slot=0;fifteen_minutes_slot<4*24;fifteen_minutes_slot++){
                this.setSelected(d, fifteen_minutes_slot, selected);
            }
        }
    }

    private isSelected(day_zero_based:number, fifteen_minutes_slot:number){
        const r=this.tblBody.value!.rows[day_zero_based]
        return r.cells[fifteen_minutes_slot+1].classList.contains("selected")
    }

    private setSelected(day_zero_based:number, fifteen_minutes_slot:number, value:boolean){
        const r=this.tblBody.value!.rows[day_zero_based]
        if(value){
            r.cells[fifteen_minutes_slot+1].classList.add("selected");
        }else{
            r.cells[fifteen_minutes_slot+1].classList.remove("selected");
        }
    }

    protected cancelHandler() {
        this.dialog.value!.close('Cancel')
        this.handler.handleWeeklyScheduleDialog(false, this.referenceHandle, null);
    }

    

    
    

    public Template = () => {
        const weekdayTemplate = (day_name, day_index) => html`${[...Array(96)].map((name, num) =>
            html`<td @mousedown=${(e: MouseEvent) => this.tdMousedown(e)} @mouseenter=${(e: MouseEvent) => this.tdMouseenter(e)} @mouseup=${(e: MouseEvent) => this.tdMouseup(e)}></td>`
        )}`
        const rowTemplates = [];
        weekdays.map((day_name, day_index) =>  {
            rowTemplates.push(html`<tr><td>${day_name}</td>${weekdayTemplate(day_name, day_index)}</tr>`)
        })


        return html`
    <dialog ${ref(this.dialog)}>
        <header>
            <span>${this.header}</span>
            <button @click=${()=>this.dialog.value!.close("cancelled")} type="button">&times;</button>
        </header>
        <main>

                <fieldset>
                    <legend>Marking Mode</legend>
                    <label><input @change=${(e:MouseEvent) => this.rdoChange(e)} type="radio" name="mode" value="TOGGLE" ?checked=${this.markingMode == MarkingMode.TOGGLE} />Toggle</label>
                    <label><input @change=${(e:MouseEvent) => this.rdoChange(e)} type="radio" name="mode" value="ON" ?checked=${this.markingMode == MarkingMode.ON}/>On</label>
                    <label><input @change=${(e:MouseEvent) => this.rdoChange(e)} type="radio" name="mode" value="OFF" ?checked=${this.markingMode == MarkingMode.OFF}/>Off</label>
                </fieldset>
                <fieldset>
                    <legend>Comfort Copy</legend>
                    <input @click=${(e:MouseEvent) => this.copy(0, [1,2,3,4])} type="button" value="Mo➔Di-Fr" />
                    <input @click=${(e:MouseEvent) => this.copy(0, [1,2,3,4,5,6])} type="button" value="Mo➔Di-So" />
                    <input @click=${(e:MouseEvent) => this.copy(5, [6])} type="button" value="Sa➔So" />
                </fieldset>
                <fieldset>
                    <legend>Comfort Fill</legend>
                    <input @click=${(e:MouseEvent) => this.setAll(true)} type="button" value="Fill All" />
                    <input @click=${(e:MouseEvent) => this.setAll(false)} type="button" value="Clear All" />
                </fieldset>

            <table class="weekschedule">
                <thead>
                <tr>
                    <th></th>
                    ${[...Array(24)].map((v,i) => html`<th colspan=4>${(i+startHour)%24}:00</th>`)}
                </tr>
                </thead>
                <tbody ${ref(this.tblBody)}>
                ${rowTemplates}
                </tbody>
            </table>
        </main>
        <footer><input @click=${() => this.okHandler()} type="button" value="OK"></input><input @click=${() => this.cancelHandler()} type="button" value="Cancel"></input></footer>
    </dialog>
    `}
}

export class SchedulerScreenController extends ScreenController {
    private btnNew() {
        this.appManagement.ShowDialog(new CreateNewScheduleDialog(Array.from(this.name2item.keys()), this.appManagement, (ok, item)=>{
            if(!ok) return;
            item.OnCreate()
            this.name2item.set(item.name, item);
            var itemTemplates:Array<TemplateResult<1>>=[];
            for(const i of this.name2item.values()){
                itemTemplates.push(i.OverallTemplate())
            }
            render(itemTemplates, this.tBodySchedules!.value);
            
        }));
    }

    private btnUpdate(){
        this.onStart_or_onRestart_or_onUpdateClicked();
    }

    private tBodySchedules:Ref<HTMLTableSectionElement>= createRef();
    private name2item:Map<string, ScheduleItem>=new Map<string, ScheduleItem>();

    public Template = () =>{
        return html`
    <h1>Schedule Definitions</h1>
        
        <div class="buttons">
            <button class="withsvg" @click=${() => this.btnNew()}>${unsafeSVG(calendarPlus)}<span>New<span></button>
            <button class="withsvg" @click=${() => this.btnUpdate()}>${unsafeSVG(rotate)}<span>Update List<span></button>
        </div>
        

        <table class="schedules">
            <thead>
            <tr>
                <th>Name</th>
                <th>Type</th>
                <th>Edit</th>
            </tr>
            </thead>
            <tbody ${ref(this.tBodySchedules)}>
            </tbody>
        </table>
        `}

    //jede Tabellenzeile hat einen Button "Rename" und einen Button "Delete"
    //im Property-Speicher des ESP32 wird abgelegt, welche Bezeichnung zu welcher internen Nummer gehört
    //Das Anlegen eines Eintrages findet ausschließlich über die Automatische Nummerierung statt
    //in der Tabelle wird auch die interne Speichernummer angezeigt



    public sendRequestDeleteSchedule(name: string) {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerDelete, RequestSchedulerDelete.createRequestSchedulerDelete(b, b.createString(name))));
        this.appManagement.SendFinishedBuilder(Namespace.Value, b); 
        
    }

    OnMessage(namespace:number, bb: flatbuffers.ByteBuffer): void {
         if(namespace!=Namespace.Value) return;
        let messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb);

        switch (messageWrapper.responseType()) {
            case Responses.ResponseSchedulerList:{
                var itemTemplates:Array<TemplateResult<1>>=[];
                var list = <ResponseSchedulerList>messageWrapper.response(new ResponseSchedulerList())
                for(var i=0;i<list.itemsLength();i++){
                    var item = list.items(i);
                    this.processItem(item, itemTemplates);
                }
                render(itemTemplates, this.tBodySchedules!.value);
                break;
            }
            case Responses.ResponseSchedulerOpen:{
                var open = <ResponseSchedulerOpen>messageWrapper.response(new ResponseSchedulerOpen())
                if(!this.name2item.has(open.payload().name())){
                    console.warn(`OpenMessage for ${open.payload().name()}, but this is not contained in [${Array.from(this.name2item.keys()).join(",")}].`)
                }else{
                    var o =this.name2item.get(open.payload().name())
                    o.OnResponseSchedulerOpen(open);
                }
                break;
            }
            default:
                break;
        }
        
    }
    processItem(item: ResponseSchedulerListItem, itemTemplates: TemplateResult<1>[]) {
        var elem:ScheduleItem=null;
        switch (item.type()) {
            case eSchedule.OneWeekIn15Minutes:{
                elem= new OneWeekIn15MinutesSchedule(item.name(), this.appManagement)
                break;
            }
            case eSchedule.SunRandom:{
                elem= new SunRandomSchedule(item.name(), this.appManagement)
                break;
            }
            case eSchedule.Predefined:{
                elem= new PredefinedSchedule(item.name(), this.appManagement)
                break;
            }
            default:
                return;
        }
        this.name2item.set(item.name(), elem);
        itemTemplates.push(elem.OverallTemplate())
    }

    private onStart_or_onRestart_or_onUpdateClicked(){
        let b = new flatbuffers.Builder(256);
        b.finish(RequestWrapper.createRequestWrapper(b, Requests.RequestSchedulerList, RequestSchedulerList.createRequestSchedulerList(b)));
        this.appManagement.SendFinishedBuilder(Namespace.Value, b);     
        
    }
    
    OnCreate(): void {
        this.appManagement.RegisterWebsocketMessageNamespace(this, Namespace.Value);
    }


    OnFirstStart(): void {
        this.onStart_or_onRestart_or_onUpdateClicked()
    }
    OnRestart(): void {
        this.onStart_or_onRestart_or_onUpdateClicked()
    }
    OnPause(): void {
    }

}
