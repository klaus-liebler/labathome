import { Html } from "@klaus-liebler/web-components/typescript/utils/common";

export class MenuManager{

    constructor(private menus:Array<Menu>){}

    public Render(subcontainer: HTMLDivElement){
        this.menus.forEach(m=>m.Introduce(this))

        subcontainer.onclick = (e) => {
            if ((<HTMLElement>e.target).classList.contains("dropbtn")) return;
            this.CloseAllButOne()
        }

        let toolbar = Html(subcontainer, "div", [], ["develop-toolbar"]);
        this.menus.forEach(m=>{m.Render(toolbar)});
    }

    public CloseAllButOne(menuNotToCloseOrNull:Menu|null=null){
        this.menus.filter(m=>(!menuNotToCloseOrNull || m!=menuNotToCloseOrNull)).forEach(m=>{m.Hide()});
    }


}

export class Menu {
    private mm!:MenuManager;
    public Introduce(mm:MenuManager){
        this.mm=mm;
    }

    private DropContent!:HTMLDivElement;
    constructor(private caption:string, private menuItems:Array<MenuItem>) {
        
    }

    public ToggleShow(){
        this.DropContent.style.display=="none"?this.DropContent.style.display="block":this.DropContent.style.display="none"
    }

    public Hide(){
        this.DropContent.style.display="none"
    }

    public Render(toolbar:HTMLElement){
        this.menuItems.forEach(m=>m.Introduce(this.mm, this))
        let menu = Html(toolbar, "div", [], ["dropdown"]);
        let menuDropBtn = <HTMLButtonElement>Html(menu, "button", [], ["dropbtn"], `${this.caption} ▼` );
        menuDropBtn.onclick = (_e) => {
            this.mm.CloseAllButOne(this);
            this.ToggleShow()
        };
        this.DropContent = Html(menu, "div", [], ["dropdown-content"]) as HTMLDivElement;
        this.menuItems.forEach(mi=>{mi.Render(this.DropContent)});

        this.DropContent.onclick = (_e) => { this.ToggleShow() };
        this.Hide();
    }
}

export class MenuItem {
    private mm!:MenuManager;
    //@ts-ignore
    private m!:Menu;
    public Introduce(mm:MenuManager,m:Menu){
        this.mm=mm;
        this.m=m
    }
    constructor(private caption:string, private clickedHandler:()=>void) {
        
    }

    public Render(menuFileDropContent:HTMLElement){
        Html(menuFileDropContent, "a", ["href", "#"], [], this.caption).onclick = (e) => {
            this.mm.CloseAllButOne()
            this.clickedHandler();
            e.preventDefault();
        }
    }
}