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