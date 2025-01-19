
import { ScreenController } from "./screen_controller";
import * as flatbuffers from 'flatbuffers';
import * as c from 'chart.js';
import { html, render } from "lit-html";
import { Ref, createRef, ref } from "lit-html/directives/ref.js";

enum TimeGranularity
{
    TEN_SECONDS = 0,
    ONE_MINUTE = 1,
    ONE_HOUR = 2,
    ONE_DAY = 3,
}


export class TimeseriesController extends ScreenController {
    private chartSeconds?:c.Chart;
    private chartDiv:Ref<HTMLCanvasElement>=createRef()
    private paramPara:Ref<HTMLParagraphElement>=createRef()
    public Template = () => html`<h1>Timeseries Sconds</h1><p ${ref(this.paramPara)}>Value=</p><canvas ${ref(this.chartDiv)}></canvas>`

    private ParamsTemplate = ()=>html`id=${this.match.groups!["id"]} val=${this.match.groups!["val"]}`
    match: RegExpMatchArray;

    OnMessage(_namespace:number, _bb: flatbuffers.ByteBuffer): void {
        throw new Error("TimeseriesController does not expect 'normal' messages");
    }


    private sendRequestTimeseries() {
        //todo: call HTTP GET, file based API
    }

    private updateDate(date: Date, granularity: TimeGranularity) {
        switch (granularity) {
            case TimeGranularity.TEN_SECONDS:
                date.setSeconds(date.getSeconds() + 10);
                break;
            case TimeGranularity.ONE_MINUTE:
                date.setMinutes(date.getMinutes() + 1);
                break;
            case TimeGranularity.ONE_HOUR:
                date.setHours(date.getHours() + 1);
                break;
            case TimeGranularity.ONE_DAY:
                date.setDate(date.getDate() + 1); //Welche Chancen/Grenzen sehen Sie für den Transfer in ihr eigenes Fach?
                break;
            default:
                break;
        }
    }



    onTimeseriesMessage(data: ArrayBuffer): void {
        var dataView = new DataView(data);
        var epochSeconds = dataView.getBigInt64(0);
        var granularity:TimeGranularity = (Number(dataView.getBigInt64(8)));
        console.log(`Start epochSeconds= ${epochSeconds} Granularity=${granularity}`);
        var date = new Date(Number(epochSeconds) * 1000);
        var labels: Array<string> = [];
        var series: Array<Array<number>> = [[], [], [], []];

        var dataOffsetByte = 64;
        while (dataOffsetByte < 4096 / 2) {
            if((dataOffsetByte-64)%256==0){
                labels.push(date.toLocaleString());
            }
            else{
                labels.push("");
            }
            series[0].push(dataView.getInt16(dataOffsetByte));
            dataOffsetByte += 2;
            series[1].push(dataView.getInt16(dataOffsetByte));
            dataOffsetByte += 2;
            series[2].push(dataView.getInt16(dataOffsetByte));
            dataOffsetByte += 2;
            series[3].push(dataView.getInt16(dataOffsetByte));
            dataOffsetByte += 2;
            this.updateDate(date, granularity);
        }

        if(granularity==TimeGranularity.TEN_SECONDS){
            this.chartSeconds!..update({labels, series});
        }
    }

   

    OnCreate(): void {
        

        
    }

    OnFirstStart(): void {
        this.chartSeconds=new c.Chart(
            this.chartDiv.value!,
            {
                type: 'line',
                data: {
                    labels: [1, 2, 3, 4, 5, 6, 7, 8],
                    datasets: [{
                        label: 'My First dataset',
                        backgroundColor: 'rgb(255, 99, 132)',
                        borderColor: 'rgb(255, 99, 132)',
                        data: [0, 10, 5, 2, 20, 30, 45, 0],
                    }]
                }
                
            ,
            options: {
                responsive: true,
                maintainAspectRatio: false,
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
        }
        );
        //this.sendRequestTimeseries();
    }
    OnRestart(): void {
        this.OnFirstStart();
    }
    OnPause(): void {
    }

    SetParameter(match:RegExpMatchArray):void{
        this.match=match;
        render(this.ParamsTemplate(), this.paramPara.value!)
    }

}
