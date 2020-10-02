import { Flowchart, FlowchartData, FlowchartOptions } from "./Flowchart";
import { ScreenController } from "./ScreenController";


export class DevelopCFCController extends ScreenController {
    private fc: Flowchart;
    onFirstStart(): void {
        this.fc.onFirstStart();
    }
    public onRestart(): void { }
    public onStop(): void { }
    public onCreate() { }
    constructor(public div: HTMLDivElement) {
        super(div);
        let data: FlowchartData = {
            operators: [
                {
                    index: 0,
                    caption: "RedButton_1",
                    typeName: "RedButton",
                    posX: 10,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 1,
                    caption: "GreenButton_1",
                    typeName: "GreenButton",
                    posX: 10,
                    posY: 150,
                    configurationData: null,
                },
                {
                    index: 2,
                    caption: "AND_1",
                    typeName: "AND",
                    posX: 250,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 3,
                    caption: "RedLed_1",
                    typeName: "RedLed",
                    posX: 500,
                    posY: 10,
                    configurationData: null,
                },
            ],
            links: [
                {
                    color: "black",
                    fromOperatorIndex: 0,
                    fromOutput: 0,
                    toOperatorIndex: 2,
                    toInput: 0
                },
                {
                    color: "black",
                    fromOperatorIndex: 1,
                    fromOutput: 0,
                    toOperatorIndex: 2,
                    toInput: 1
                },
                {
                    color: "black",
                    fromOperatorIndex: 2,
                    fromOutput: 0,
                    toOperatorIndex: 3,
                    toInput: 0
                },
            ]
        };
        let options = new FlowchartOptions();
        options.data = data;
        this.fc = new Flowchart(this.div, options);
    }


}
