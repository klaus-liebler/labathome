export class Utils
{
    public static EventCoordinatesInSVG(evt:MouseEvent, element:Element, positionRatio:number=1):Location2D {
        let rect = element.getBoundingClientRect();
        return {x: (evt.clientX - rect.left)/positionRatio, y:(evt.clientY - rect.top)/positionRatio}
    }
}

export interface Location2D {
    x: number;
    y: number;
}

export interface KeyValueTuple {
    key: string;
    value: any;
}

export class StringNumberTuple{
    public constructor(public s:string, public n:number){}
}


export class $
{
    public static readonly SVGNS = "http://www.w3.org/2000/svg";
    public static readonly XLINKNS = "http://www.w3.org/1999/xlink";
    public static readonly HTMLNS = "http://www.w3.org/1999/xhtml";

    public static Svg(parent: Element, type:string,  attributes:string[], classes?: string[]):SVGElement {
        return  parent.appendChild(<SVGElement>$.Elem($.SVGNS, type, attributes, classes));
    }

    //<svg class="icon icon-wastebasket"><use xlink:href="#icon-wastebasket"></use></svg>
    public static SvgIcon(parent: Element, iconname:string):SVGSVGElement
    {
        let svg = <SVGSVGElement>$.Svg(parent, "svg", [], ["icon", "icon-"+iconname]);
        let use =$.Svg(svg, "use", [], []);
        use.setAttributeNS(this.XLINKNS, "href", "#icon-"+iconname);
        parent.appendChild(svg);
        return svg;
    }

    public static ColorNumColor2ColorDomString(num:number):string {
        let str = num.toString(16);
        while (str.length < 6) str = "0" + num;
        return "#"+str;
    }

    public static ColorDomString2ColorNum(colorString: string):number {
        return parseInt(colorString.substring(1), 16);
    }

    public static Html(parent: Element, type:string,  attributes:string[], classes?: string[], textContent?:string):HTMLElement {
        return parent.appendChild(<HTMLElement>$.Elem($.HTMLNS, type, attributes, classes, textContent));
    }

    public static HtmlAsFirstChild(parent: Element, type:string,  attributes:string[], classes?: string[], textContent?:string):HTMLElement {
        if(parent.firstChild)
            return parent.insertBefore(<HTMLElement>$.Elem($.HTMLNS, type, attributes, classes, textContent), parent.firstChild);
        else
            return parent.appendChild(<HTMLElement>$.Elem($.HTMLNS, type, attributes, classes, textContent));
    }

    private static Elem(ns:string, type:string, attributes:string[], classes?: string[], textContent?:string):Element
    {
        let element = document.createElementNS(ns, type);
        if(classes)
        {
            for (const clazz of classes) {
                element.classList.add(clazz);
            }
        }
        let i:number;
        for(i=0;i<attributes.length;i+=2)
        {
            element.setAttribute(attributes[i], attributes[i+1]);
        }
        if(textContent)
        {
            element.textContent=textContent;
        }
        return element;
    }

    public static InputIntegerNumber(table:HTMLTableSectionElement, min:number, max:number, key:string, cfg:KeyValueTuple[]|null):HTMLInputElement
    {
        let value:number=0;
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key && !isNaN(e.value)){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        return <HTMLInputElement>$.Html(inputContainer, "input", ["type", "number", "step", "1", "min", ""+Math.round(min), "max", ""+Math.round(max), "value", ""+Math.round(value),]);
    }

    public static InputFloatNumber(table:HTMLTableSectionElement, key:string, cfg:KeyValueTuple[]|null):HTMLInputElement
    {
        let value:number=0;
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key && !isNaN(e.value)){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        return <HTMLInputElement>$.Html(inputContainer, "input", ["type", "number", "value", ""+value,]);
    }

    public static InputSelect(table:HTMLTableSectionElement, displayValueAndKeyvalues:StringNumberTuple[], key:string, cfg:KeyValueTuple[]|null):HTMLSelectElement
    {
        let value:number=0;
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key && !isNaN(e.value)){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        let select = <HTMLSelectElement>$.Html(inputContainer, "select", [], []);
        for(let i=0;i<displayValueAndKeyvalues.length;i++)
        {
            let option = $.Html(select, "option", ["value", ""+displayValueAndKeyvalues[i].n], [], displayValueAndKeyvalues[i].s);
            if(i==value){
                option.setAttribute("selected", "");
            }
        }
        return select;
    }

   

    public static InputColor(table:HTMLTableSectionElement, key:string, cfg:KeyValueTuple[]|null):HTMLInputElement
    {
        let value:string="#ff0000";
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        return <HTMLInputElement>$.Html(inputContainer, "input", ["type", "color",  "value", value]);
    }
}