import { AppManagement } from "./AppManagement";
import {$} from "./Utils"
export class DialogController {

    private dialog = <HTMLDialogElement>document.getElementById('dialog')!;
    private dialogHeading = <HTMLHeadingElement>document.getElementById('dialog-heading')!;
    private dialogBody = <HTMLDivElement>document.getElementById('dialog-body')!;
    private dialogFooter = <HTMLElement>document.getElementById('dialog-footer')!;

    constructor(private appManagement:AppManagement) {
        
    }
    
    public init() {

        document.getElementById('dialog-close')!.onclick = (e) => {
            this.dialog.close("cancelled");
        }
        this.dialog.oncancel = (e) => {
            this.dialog.close("cancelled");
        }

        // close when clicking on backdrop
        this.dialog.onclick = (event) => {
            if (event.target === this.dialog) {
                this.dialog.close('cancelled');
            }
        }
    }

    public showOKDialog(priority: number, handler: (a:string)=>any) {
        this.dialogFooter.innerText="";
        $.Html(this.dialogFooter, "button", ["type", "button"], [], "OK").onclick=(e)=>{
            handler("OK");
        };
        this.dialog.showModal();
    }

    private prepareDialog()
    {
        this.dialogHeading.innerText="";
        this.dialogBody.innerText="";
        this.dialogFooter.innerText="";
    }

    public showFilelist(priority: number, files:string[], handler: (a:string)=>any) {

        this.prepareDialog();
        this.dialogHeading.innerText="Please select a file to load"
        $.Html(this.dialogFooter, "button", ["type", "button"], [], "Cancel").onclick=(e)=>{
            this.dialog.close("cancelled");
        };
        
        let resultStr = '["Test01", "Test02", "Test03"]';
        let result:string[] = JSON.parse(resultStr);
        let table = <HTMLTableElement>$.Html(this.dialogBody, "table", [], []);
        let thead = <HTMLTableSectionElement>$.Html(table, "thead", [],[]);
        let tr_head = $.Html(thead, "tr", [], []);
        $.Html(tr_head, "th", [], [], "File Name");
        $.Html(tr_head, "th", [], [], "File Operation");
        let tbody= <HTMLTableSectionElement>$.Html(table, "tbody", [],[]);
        files.forEach((filename)=>{
            let tr = $.Html(tbody, "tr", [], []);
            $.Html(tr, "td", [], [], filename);
            let operationTd= $.Html(tr, "td", [], []);
            let openButton = $.Html(operationTd, "button", ["type", "button"], []);
            $.SvgIcon(openButton, "folder-open");
            let deleteButton=$.Html(operationTd, "button", ["type", "button"], [], );
            $.SvgIcon(deleteButton, "bin2");
        });
        this.dialog.showModal();
        
    }









  
}