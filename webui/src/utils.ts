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
}