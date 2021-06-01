import { AppManagement } from "./AppManagement";
import { Flowchart, FlowchartData, FlowchartOptions } from "./flowchart/Flowchart";
import { ScreenController } from "./ScreenController";


export class DevelopCFCController extends ScreenController {
    private fc: Flowchart;
    private timer: number | undefined;
    onFirstStart(): void {
        this.timer = window.setInterval(() => { this.fc.triggerDebug(); }, 1000);
        this.fc.onFirstStart();
    }
    public onRestart(): void {
        this.timer = window.setInterval(() => { this.fc.triggerDebug(); }, 1000);
    }
    public onStop(): void {
        window.clearInterval(this.timer);
    }
    public onCreate() { }
    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        let data: FlowchartData = {
            operators: [
                {
                    index: 0,
                    caption: "RedButton",
                    globalTypeIndex: 11,
                    posX: 10,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 1,
                    caption: "GreenButton",
                    globalTypeIndex: 9,
                    posX: 10,
                    posY: 150,
                    configurationData: null,
                },
                {
                    index: 2,
                    caption: "AND",
                    globalTypeIndex: 1,
                    posX: 250,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 3,
                    caption: "RedLed",
                    globalTypeIndex: 15,
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
        this.fc = new Flowchart(this.appManagement, this.div, options);
    }


}
