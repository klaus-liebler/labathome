import { AppManagement } from "./AppManagement";
import {$} from "./utils"
export class DialogController {
    

    private dialog = <HTMLDialogElement>document.getElementById('dialog')!;
    private dialogHeading = <HTMLHeadingElement>document.getElementById('dialog-heading')!;
    private dialogBody = <HTMLDivElement>document.getElementById('dialog-body')!;
    private dialogFooter = <HTMLElement>document.getElementById('dialog-footer')!;

    constructor(private appManagement:AppManagement) {
        
    }
    
    public init() {

        document.getElementById('dialog-close')!.onclick = (e) => {
            //this.dialog.close("cancelled");
        }
        //this.dialog.oncancel = (e) => {
            //this.dialog.close("cancelled");
        //}

        // close when clicking on backdrop
        this.dialog.onclick = (event) => {
            if (event.target === this.dialog) {
                //this.dialog.close('cancelled');
            }
        }
    }

    public showEnterFilenameDialog(priority: number, messageText: string, handler: (filename: string) => void) {
        this.prepareDialog();
        this.dialogHeading.innerText="Enter Filename";
        $.Html(this.dialogBody, "p", [], [], messageText);
        let fileInput= <HTMLInputElement>$.Html(this.dialogBody, "input", ["pattern", "^[A-Za-z0-9]{1,10}$"], []);
        this.dialogFooter.innerText="";
        $.Html(this.dialogFooter, "button", [], [], "OK").onclick=(e)=>{
            //this.dialog.close('OK');
            if(handler!=null) handler(fileInput.value);
        };
       // this.dialog.showModal();
    }

    public showOKDialog(priority: number, messageText:string, handler: ((a:string)=>any)|null) {
        this.prepareDialog();
        this.dialogHeading.innerText="Message";
        this.dialogBody.innerText=messageText;
        this.dialogFooter.innerText="";
        $.Html(this.dialogFooter, "button", ["type", "button"], [], "OK").onclick=(e)=>{
            //this.dialog.close('cancelled');
            if(handler!=null) handler("OK");
        };
        //this.dialog.showModal();
    }

    private prepareDialog()
    {
        this.dialogHeading.innerText="";
        this.dialogBody.innerText="";
        this.dialogFooter.innerText="";
    }

    public showFilelist(priority: number, files:string[], openhandler: (filename:string)=>any, deletehandler: (filename:string)=>any) {

        this.prepareDialog();
        this.dialogHeading.innerText="Please select a file to load"
        $.Html(this.dialogFooter, "button", ["type", "button"], [], "Cancel").onclick=(e)=>{
            //this.dialog.close("cancelled");
        };
        let table = <HTMLTableElement>$.Html(this.dialogBody, "table", [], []);
        let thead = <HTMLTableSectionElement>$.Html(table, "thead", [],[]);
        let tr_head = $.Html(thead, "tr", [], []);
        $.Html(tr_head, "th", [], [], "File Name");
        $.Html(tr_head, "th", [], [], "File Operation");
        let tbody= <HTMLTableSectionElement>$.Html(table, "tbody", [],[]);
        for(let filename of files){
            if(!filename.endsWith(".json")) continue;
            filename=filename.substring(0, filename.length-5);
            let tr = $.Html(tbody, "tr", [], []);
            $.Html(tr, "td", [], [], filename);
            let operationTd= $.Html(tr, "td", [], []);
            let openButton = $.Html(operationTd, "button", ["type", "button"], []);
            $.SvgIcon(openButton, "folder-open");
            openButton.onclick=(e)=>{
                //this.dialog.close("opened");
                openhandler(filename);
                
            };
            let deleteButton=$.Html(operationTd, "button", ["type", "button"], [], );
            $.SvgIcon(deleteButton, "bin2");
            deleteButton.onclick=(e)=>{
                //this.dialog.close("deleted");
                deletehandler(filename);
            }
        };
        //this.dialog.showModal();
        
    }









  
}