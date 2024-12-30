import {TemplateResult, html, render} from 'lit-html';
import { IHtmlRenderer } from "./interfaces";
import { createRef, ref, Ref } from 'lit-html/directives/ref.js';

export interface IRouteHandler {
  handleRoute(params: RegExpMatchArray): void;
}

export class Route {
  constructor(public readonly url:string, public readonly urlPattern: RegExp, public readonly caption:TemplateResult<1>, public readonly handler: IRouteHandler) { }
}

export default class RouterMenu implements IHtmlRenderer {
  private currentUrl = "";

  private list:Ref<HTMLUListElement>=createRef();
 
  constructor(private readonly root="/", private routes:Array<Route>) {
   
    globalThis.window.onpopstate = (e:PopStateEvent) => {
      console.log(`onpopstate  ${globalThis.location} ${e.timeStamp}`)
      var newUrl = decodeURI(globalThis.location.pathname)
      newUrl = this.root != '/' ? newUrl.replace(this.root, '') : newUrl;
      if (this.currentUrl != newUrl) {
        this.check()
      }
      this.currentUrl = newUrl;
      //e.preventDefault();
    }
  }
  RenderStatic(container: HTMLElement): void {
    render(this.Template, container)
  }

  public ToggleHamburgerMenu(){
    
    console.info(window.getComputedStyle(document.body).display)
    if(window.getComputedStyle(document.body).display=="grid") return
    if (this.list.value!.style.display === "block") {
      this.list.value!.style.display = "none";
    } else {
      this.list.value!.style.display = "block";
    }
  }
  
  public check() {
    var fragment = decodeURI(globalThis.location.pathname)
    for(var r of this.routes){
        var match = fragment.match(r.urlPattern);
        if (match){
          console.log(`Match for ${r.url}`)
          r.handler.handleRoute(match);
          break;
        }
      }
  }

  private navigation_anchor_clicked(e:MouseEvent, url:string){
    e.preventDefault();
    this.ToggleHamburgerMenu()
    console.log(`New URL push ${url}`)
    window.history.pushState(null, "", url);
    this.check();
  }
 

  public readonly Template= ()=> html`
  <ul ${ref(this.list)}>
    ${this.routes.map((item:Route) => html`<li><a @click=${(e:MouseEvent)=>this.navigation_anchor_clicked(e, item.url)} href=${item.url}>${item.caption}</a></li>`)}
  </ul>`




}