(function(){function r(e,n,t){function o(i,f){if(!n[i]){if(!e[i]){var c="function"==typeof require&&require;if(!f&&c)return c(i,!0);if(u)return u(i,!0);var a=new Error("Cannot find module '"+i+"'");throw a.code="MODULE_NOT_FOUND",a}var p=n[i]={exports:{}};e[i][0].call(p.exports,function(r){var n=e[i][1][r];return o(n||r)},p,p.exports,r,e,n,t)}return n[i].exports}for(var u="function"==typeof require&&require,i=0;i<t.length;i++)o(t[i]);return o}return r})()({1:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.DevelopCFCController = exports.ScreenController = void 0;
const Flowchart_1 = require("./Flowchart");
class ScreenController {
    constructor(div) {
        this.div = div;
        this.hideDIV();
        this.state = ControllerState.CREATED;
    }
    get ElementId() { return this.div.id; }
    get State() { return this.state; }
    set State(value) { this.state = value; }
    showDIV() {
        this.div.style.display = "block";
    }
    hideDIV() {
        this.div.style.display = "none";
    }
}
exports.ScreenController = ScreenController;
class DevelopCFCController extends ScreenController {
    constructor(div) {
        super(div);
        this.div = div;
        let data = {
            operators: [
                {
                    caption: "RedButton_1",
                    type: "RedButton",
                    id: "RedButton_1",
                    posX: 10,
                    posY: 10,
                },
                {
                    caption: "GreenButton_1",
                    type: "GreenButton",
                    id: "GreenButton_1",
                    posX: 10,
                    posY: 150,
                },
                {
                    caption: "AND_1",
                    type: "AND",
                    id: "AND_1",
                    posX: 250,
                    posY: 10,
                },
                {
                    caption: "RedLed_1",
                    type: "RedLed",
                    id: "RedLed_1",
                    posX: 500,
                    posY: 10,
                },
            ],
            links: [
                {
                    color: "black",
                    fromId: "RedButton_1",
                    fromOutput: 0,
                    toId: "AND_1",
                    toInput: 0
                },
                {
                    color: "black",
                    fromId: "GreenButton_1",
                    fromOutput: 0,
                    toId: "AND_1",
                    toInput: 1
                },
                {
                    color: "black",
                    fromId: "AND_1",
                    fromOutput: 0,
                    toId: "RedLed_1",
                    toInput: 0
                },
            ]
        };
        let options = new Flowchart_1.FlowchartOptions();
        options.data = data;
        this.fc = new Flowchart_1.Flowchart(this.div, options);
    }
    onFirstStart() {
        this.fc.onFirstStart();
    }
    onRestart() { }
    onStop() { }
    onCreate() { }
}
exports.DevelopCFCController = DevelopCFCController;
class DashboardController extends ScreenController {
    constructor(div) {
        super(div);
        this.div = div;
    }
    onFirstStart() { }
    onRestart() { }
    onStop() { }
    onCreate() { }
}
class ReportsController extends ScreenController {
    constructor(div) {
        super(div);
        this.div = div;
    }
    onFirstStart() { }
    onRestart() { }
    onStop() { }
    onCreate() {
        return;
        let data = {
            // A labels array that can contain any sort of values
            labels: [],
            // Our series array that contains series objects or in this case series data arrays
            series: [[]],
            low: 0,
            high: 40
        };
        let options = {
            width: 600,
            height: 400
        };
        let currVal = 20;
        for (let i = 0; i < 10; i++) {
            data.labels.push("" + (10 - i));
            data.series[0][i] = currVal;
        }
        // Create a new line chart object where as first parameter we pass in a selector
        // that is resolving to our chart container element. The Second parameter
        // is the actual data object.
        let chart = new Chartist.Line('.ct-chart', data, options);
        let timer = window.setInterval(() => {
            let foo = data.series[0].slice(1);
            foo.push(20 + 3 * Math.random());
            data.series[0] = foo;
            chart.update(data, options, false);
        }, 500);
        window.setTimeout(() => { window.clearInterval(timer); }, 10000);
    }
}
var ControllerState;
(function (ControllerState) {
    ControllerState[ControllerState["CREATED"] = 0] = "CREATED";
    ControllerState[ControllerState["STARTED"] = 1] = "STARTED";
    ControllerState[ControllerState["STOPPED"] = 2] = "STOPPED";
})(ControllerState || (ControllerState = {}));
class AppController {
    constructor() {
        this.stateDiv = document.getElementById("spnConnectionState");
        this.screenControllers = [];
        this.activeControllerIndex = 0;
    }
    SetApplicationState(state) {
        this.stateDiv.innerHTML = state;
    }
    setActiveScreen(newIndex) {
        this.screenControllers.forEach((controller, i) => {
            if (i == newIndex) {
                controller.showDIV();
                if (controller.State == ControllerState.CREATED) {
                    controller.onFirstStart();
                    controller.State = ControllerState.STARTED;
                }
                else {
                    controller.onRestart();
                    controller.State = ControllerState.STARTED;
                }
            }
            else {
                controller.hideDIV();
                if (controller.State == ControllerState.STARTED) {
                    controller.onStop();
                    controller.State = ControllerState.STOPPED;
                }
            }
        });
        this.activeControllerIndex = newIndex;
    }
    startup() {
        this.screenControllers.push(new DashboardController(document.getElementById("screen_dashboard")));
        this.screenControllers.push(new DevelopCFCController(document.getElementById("screen_develop")));
        this.screenControllers.push(new ReportsController(document.getElementById("screen_reports")));
        this.screenControllers.forEach((sc) => sc.onCreate());
        this.setActiveScreen(0);
        let id2index = new Map();
        this.screenControllers.forEach((value, index) => { id2index.set("show_" + value.ElementId, index); });
        document.querySelectorAll("nav a").forEach((a) => {
            let id = a.id;
            let index = id2index.get(a.id) || 0;
            a.onclick = (e) => this.setActiveScreen(index);
        });
        this.SetApplicationState("WebSocket is not connected");
        let websocket = new WebSocket('ws://' + location.hostname + '/w');
        websocket.onopen = e => {
            this.SetApplicationState('WebSocket connection opened');
            document.getElementById("test").innerHTML = "WebSocket is connected!";
        };
        websocket.onmessage = (evt) => {
            var msg = evt.data;
            let value;
            switch (msg.charAt(0)) {
                case 'L':
                    console.log(msg);
                    value = msg.replace(/[^0-9\.]/g, '');
                    switch (value) {
                        case "0":
                            document.getElementById("led1").style.backgroundColor = "black";
                            break;
                        case "1":
                            document.getElementById("led1").style.backgroundColor = "green";
                            break;
                        case "2":
                            document.getElementById("led2").style.backgroundColor = "black";
                            break;
                        case "3":
                            document.getElementById("led2").style.backgroundColor = "green";
                            break;
                    }
                    console.log("Led = " + value);
                    break;
                default:
                    let p = JSON.parse(evt.data);
                    document.getElementById("td_myName").innerText = p.d.myName;
                    document.getElementById("td_temperature").innerText = p.d.temperature;
                    document.getElementById("td_humidity").innerText = p.d.humidity;
                    document.getElementById("td_heap").innerText = p.info.heap;
                    document.getElementById("td_time").innerText = p.info.time;
                    break;
            }
        };
        websocket.onclose = (e) => {
            console.log('Websocket connection closed due to ' + e.reason);
            this.SetApplicationState('Websocket connection closed due to ' + e.reason);
        };
        websocket.onerror = (evt) => {
            console.log('Websocket error: ' + evt.returnValue);
            this.SetApplicationState("WebSocket error!" + evt.returnValue);
        };
        document.querySelectorAll("#pButtons button").forEach((b) => {
            b.onclick = (e) => {
                websocket.send("L" + b.dataset.rel);
            };
        });
    }
}
let app;
document.addEventListener("DOMContentLoaded", (e) => {
    app = new AppController();
    app.startup();
});
},{"./Flowchart":2}],2:[function(require,module,exports){
"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.Flowchart = exports.FlowchartOptions = void 0;
const FlowchartConnector_1 = require("./FlowchartConnector");
const FlowchartExporter_1 = require("./FlowchartExporter");
const FlowchartLink_1 = require("./FlowchartLink");
const FlowchartOperator_1 = require("./FlowchartOperator");
const operatorimpl = __importStar(require("./FlowchartOperatorImpl"));
const TopologicalSorfDFS_1 = require("./TopologicalSorfDFS");
const Utils_1 = require("./Utils");
/*
In einem Ressources-Toolbar befinden sich alle Ressourcen (Input und Output), die ein Board anbietet. Diese können genau einmal auf das Board gezogen werden
Problem: OneWire-Ressourcen!:
Operator-Klassen haben ein Positionstyp: Default, Input, Output
Operator-Klassen haben einen Singleton-Typ, nämlich Default, Singleton. Bei Singletons darf nur eine Instanz der KLasse erzeugt werden
Operator-Instanzen können per Propery-Grid konfiguriert werden. Sie stellen eine Methode PopulateProperyGrid(HMTLDivElement) zur Verfügung, das ein editerbares HTML-Grid ins DIV hineinzeichnen. Außerhalb wurde dieses div zuvor geleert und im Anschluss wird von außerhalb ein Save-Button gerendert.
  Ein null-Rückgabewert bedeutet, dass kein PropertyGrid benötigt wird.
  Sie stellen weiterhin eine Methode SaveProperyGrid(HMTLDivElement) zur Verfügung, in der sie die Inhalte wieder einlesen und intern wie auch immer speichern
  Sie stellen weiterhin eine Methode GetPropertyGridDataAsJSONString() zur Verfügung. Diese gibt die Daten als JSON-String zurück

*/
class FlowchartOptions {
    constructor() {
        this.canUserEditLinks = true;
        this.canUserMoveOperators = true;
        this.data = undefined;
        this.distanceFromArrow = 3;
        this.defaultOperatorClass = 'flowchart-default-operator';
        this.defaultLinkColor = '#3366ff';
        this.defaultSelectedLinkColor = 'black';
        this.linkWidth = 10;
        this.grid = 10;
        this.multipleLinksOnOutput = true;
        this.multipleLinksOnInput = false;
        this.linkVerticalDecal = 0;
    }
}
exports.FlowchartOptions = FlowchartOptions;
//Connector besteht aus "wrapper", dem Label, dem großen dreieck und einem kleinen Dreieck
class Flowchart {
    constructor(container, options) {
        this.container = container;
        this.options = options;
        this.operators = new Map();
        this.links = new Map();
        this.lastOutputConnectorClicked = null;
        this.selectedOperator = null;
        this.selectedLink = null;
        this.positionRatio = 1;
        this.temporaryLinkSnapped = false;
        this.upcounter = 0;
        if (!this.container) {
            throw new Error("container is null");
        }
        let subcontainer = Flowchart.Html(this.container, "div", [], ["develop-ui"]);
        subcontainer.onclick = (e) => {
            if (e.target.classList.contains("dropbtn"))
                return;
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem) => { elem.classList.remove("show"); });
        };
        let toolbar = Flowchart.Html(subcontainer, "div", [], ["develop-toolbar"]);
        let filebutton = Flowchart.Html(toolbar, "a", ["href", "#"], ["develop-toolbar"], "File");
        let runbutton = Flowchart.Html(toolbar, "a", ["href", "#"], ["develop-toolbar"], "Run");
        let menuDebug = Flowchart.Html(toolbar, "div", [], ["dropdown"]);
        let menuDebugDropBtn = Flowchart.Html(menuDebug, "button", [], ["dropbtn"], "Debug ▼");
        let menuDebugDropContent = Flowchart.Html(menuDebug, "div", [], ["dropdown-content"]);
        menuDebugDropBtn.onclick = (e) => {
            menuDebugDropContent.classList.toggle("show");
        };
        Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "☭ Compile").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem) => { elem.classList.remove("show"); });
            this.compile();
        };
        Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "👣 Run on device").onclick = (e) => {
        };
        let menuDebugLink2 = Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "◉ Stop");
        let menuDebugLink3 = Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "◯ Erase");
        let workspace = Flowchart.Html(subcontainer, "div", ["tabindex", "0"], ["develop-workspace"]); //tabindex, damit keypress-Events abgefangen werden können
        this.propertyGridHtmlDiv = Flowchart.Html(subcontainer, "div", [], ["develop-properties"]);
        this.flowchartContainerSvgSvg = Flowchart.Svg(workspace, "svg", ["width", "100%", "height", "100%"], ["flowchart-container"]);
        this.linksLayer = Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-links-layer"]);
        this.operatorsLayer = Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-operators-layer", "unselectable"]);
        this.tempLayer = Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-temporary-link-layer"]);
        this.tempLayer.style.visibility = "hidden"; //visible
        let defs = Flowchart.Svg(this.tempLayer, "defs", []);
        let markerArrow = Flowchart.Svg(defs, "marker", ["id", "marker-arrow", "markerWidth", "4", "markerHeight", "4", "refX", "1", "refY", "2", "orient", "0"]);
        this.markerArrow = Flowchart.Svg(markerArrow, "path", ["d", "M0,0 L0,4 L2,2 z", "fill", "red", "stroke", "black", "stroke-width", "0.5"]);
        let markerCircle = Flowchart.Svg(defs, "marker", ["id", "marker-circle", "markerWidth", "4", "markerHeight", "4", "refX", "2", "refY", "2", "orient", "0"]);
        this.markerCircle = Flowchart.Svg(markerCircle, "circle", ["cx", "2", "cy", "2", "r", "2", "fill", "red", "stroke-width", "1px", "stroke", "black"]);
        this.temporaryLink = Flowchart.Svg(this.tempLayer, "line", ["x1", "0", "y1", "0", "x2", "0", "y2", "0", "stroke-dasharray", "6,6", "stroke-width", "4", "stroke", "black", "fill", "none", "marker-end", "url(#marker-arrow)"]);
        let toolsActivator = Flowchart.Svg(this.flowchartContainerSvgSvg, "rect", ["width", "40", "height", "100%", "fill", "white", "fill-opacity", "0"]);
        this.toolsLayer = Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-tools-layer", "unselectable"]);
        this.toolsLayer.style.display = "none"; //visible
        let toolsRect = Flowchart.Svg(this.toolsLayer, "rect", ["width", "140", "height", "100%", "rx", "10", "ry", "10"], ["tools-container"]);
        //The onmousemove event occurs every time the mouse pointer is moved over the div element.
        //The mouseenter event only occurs when the mouse pointer enters the div element.
        //The onmouseover event occurs when the mouse pointer enters the div element, and its child elements (p and span).
        //The mouseout event triggers when the mouse pointer leaves any child elements as well the selected element.
        //The mouseleave event is only triggered when the mouse pointer leaves the selected element.
        toolsActivator.onmouseenter = (e) => {
            this.toolsLayer.style.display = "initial";
        };
        this.toolsLayer.onmouseleave = (e) => {
            this.toolsLayer.style.display = "none";
        };
        this.flowchartContainerSvgSvg.onclick = (e) => {
            if (e.target == this.Element) //if the click is in a "free" area, then the target is the uppermost layer; the linkLayer!
             {
                this.unselectOperator();
                this.unselectLink();
            }
        };
        this.flowchartContainerSvgSvg.onmouseup = (e) => {
            console.log("Flowchart this.element.onmouseup with e.target=" + e.target);
        };
        workspace.onkeyup = (e) => {
            if (e.key == "Delete") {
                console.log("Flowchart workspace.onkeyup with e.target=" + e.target + " und Delete-Key");
                this.deleteSelectedOperator();
            }
            else {
                console.log("Flowchart workspace.onkeyup with e.target=" + e.target + " und key " + e.key);
            }
        };
        this.populateToolsLayer();
    }
    get SelectedLink() { return this.selectedLink; }
    ;
    get PositionRatio() { return this.positionRatio; }
    get LinkLayer() { return this.linksLayer; }
    get OperatorsLayer() { return this.operatorsLayer; }
    get ToolsLayer() { return this.toolsLayer; }
    _notifyGlobalMousemoveWithLink(e) {
        if (this.lastOutputConnectorClicked != null && !this.temporaryLinkSnapped) {
            let end = Utils_1.Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
            this.temporaryLink.setAttribute('x2', "" + end.x);
            this.temporaryLink.setAttribute('y2', "" + end.y);
        }
    }
    _notifyGlobalMouseupWithLink(e) {
        this.unsetTemporaryLink();
    }
    _notifyOutputConnectorMousedown(c, e) {
        this.temporaryLinkSnapped = false;
        let start = c.GetLinkpoint();
        let end = Utils_1.Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
        this.temporaryLink.setAttribute('x1', "" + start.x);
        this.temporaryLink.setAttribute('y1', "" + start.y);
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
        this.setTemporaryLink(c);
        document.onmouseup = (e) => {
            document.onmouseup = null;
            document.onmousemove = null;
            this._notifyGlobalMouseupWithLink(e);
        };
        document.onmousemove = (e) => {
            this._notifyGlobalMousemoveWithLink(e);
        };
    }
    _notifyInputConnectorMouseup(c, e) {
        if (this.lastOutputConnectorClicked == null)
            return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0)
            return;
        if (this.lastOutputConnectorClicked.Type == c.Type) {
            this.createLink(null, this.lastOutputConnectorClicked, c);
        }
        this.unsetTemporaryLink();
    }
    _notifyOperatorClicked(o, e) {
        this.SelectOperator(o);
    }
    _notifyLinkClicked(link, e) {
        this.selectLink(link);
    }
    _notifyInputConnectorMouseenter(c, e) {
        if (this.lastOutputConnectorClicked == null || this.lastOutputConnectorClicked.Type != c.Type)
            return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0)
            return;
        this.temporaryLinkSnapped = true;
        let end = c.GetLinkpoint();
        this.temporaryLink.setAttribute("marker-end", "url(#marker-circle)");
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
    }
    _notifyInputConnectorMouseleave(c, e) {
        this.temporaryLinkSnapped = false;
        this.temporaryLink.setAttribute("marker-end", "url(#marker-arrow)");
    }
    unselectLink() {
        if (this.selectedLink != null) {
            if (this.options.onLinkUnselect && !this.options.onLinkUnselect(this.selectedLink)) {
                return;
            }
            this.selectedLink.UncolorizeLink();
            this.selectedLink = null;
        }
    }
    selectLink(link) {
        this.unselectLink();
        if (this.options.onLinkSelect && !this.options.onLinkSelect(link)) {
            return;
        }
        this.unselectOperator();
        this.selectedLink = link;
        link.ColorizeLink(this.options.defaultSelectedLinkColor);
    }
    compile() {
        let index2wrappedOperator = new Map();
        this.operators.forEach((v, k, m) => {
            index2wrappedOperator.set(v.GlobalOperatorIndex, new TopologicalSorfDFS_1.NodeWrapper(v));
        });
        let wrappedOutputOperators = [];
        for (let i of index2wrappedOperator.values()) {
            //Stelle für jede "gewrapte Node" fest, welche Operatoren von Ihr abhängig sind
            let dependents = new Set();
            for (const inputkv of i.Payload.InputsKVIt) {
                for (const linkkv of inputkv[1].LinksKVIt) {
                    let dependentOperator = linkkv[1].From.Parent;
                    let dependentWrappedNode = index2wrappedOperator.get(dependentOperator.GlobalOperatorIndex);
                    if (!dependentWrappedNode)
                        throw new Error("Implementation Error: dependentWrappedNode is undefined");
                    dependents.add(dependentWrappedNode);
                }
            }
            dependents.forEach(e => i.DependendNodes.push(e));
            //füge alle mit Typ "Output" einer Liste hinzu
            if (i.Payload.TypeInfo.Position == FlowchartOperator_1.PositionType.Output)
                wrappedOutputOperators.push(i);
        }
        let algorithm = new TopologicalSorfDFS_1.TopologicalSortDFS();
        let sortedList = algorithm.sort(wrappedOutputOperators);
        for (const key in sortedList) {
            let value = sortedList[key];
            value.Payload.SetDebugInfoText("Sequenznummer " + key);
        }
        FlowchartExporter_1.FlowchartExporter.Export(sortedList.map((e) => e.Payload));
    }
    deleteSelectedOperator() {
        if (!this.selectedOperator)
            return;
        this.selectedOperator.RemoveFromDOM();
        this.operators.delete(this.selectedOperator.GlobalOperatorIndex);
        for (const outputKV of this.selectedOperator.OutputsKVIt) {
            for (const linkKV of outputKV[1].LinksKVIt) {
                this.DeleteLink(linkKV[1].GlobalLinkIndex);
            }
        }
        this.unselectOperator();
    }
    get Options() { return this.options; }
    get Element() { return this.flowchartContainerSvgSvg; }
    onFirstStart() {
        if (typeof this.options.data !== undefined && this.options.data != null) {
            this.setData(this.options.data);
        }
    }
    populateToolsLayer() {
        let y = 10;
        for (let clazz in operatorimpl) {
            if (operatorimpl[clazz].GetTypeInfo) {
                let info = operatorimpl[clazz].GetTypeInfo();
                //if(info.Position!=PositionType.Default || info.Singleton!=SingletonType.Default) continue;
            }
            let toolGroup = Flowchart.Svg(this.toolsLayer, "g", ["transform", `translate(5 ${y})`]);
            let box = Flowchart.Svg(toolGroup, "rect", ["width", "130", "height", "30", "rx", "10", "ry", "10"], ["tool-box"]);
            let title = Flowchart.Svg(toolGroup, "text", ["x", "5", "y", "25"], ["tool-caption"]);
            toolGroup.onmousedown = (e) => {
                let cnt = this.upcounter;
                let name = clazz.substring(0, clazz.length - "Operator".length) + "_" + this.upcounter;
                if (this.options.onOperatorCreate && !this.options.onOperatorCreate(name, null, false)) {
                    return null;
                }
                console.log("Creating " + name);
                this.upcounter++;
                let o = new operatorimpl[clazz](this, name);
                let coords = Utils_1.Utils.EventCoordinatesInSVG(e, this.Element);
                o.MoveTo(coords.x - 10, coords.y - 10);
                o.RegisterDragging(e);
                this.operators.set(o.GlobalOperatorIndex, o);
            };
            title.textContent = clazz;
            y += 40;
        }
    }
    setData(data) {
        this.links.forEach((e) => e.RemoveFromDOM());
        this.links.clear();
        this.operators.forEach((e) => e.RemoveFromDOM());
        this.operators.clear();
        let opId2op = {};
        for (const d of data.operators) {
            let o = this.createOperator(d);
            opId2op[d.id] = o;
        }
        for (const d of data.links) {
            let fromOp = opId2op[d.fromId];
            let toOp = opId2op[d.toId];
            if (fromOp == null || toOp == null)
                continue;
            let fromConn = fromOp.GetOutputConnectorByIndex(d.fromOutput);
            let toConn = toOp.GetInputConnectorByIndex(d.toInput);
            if (fromConn == null || toConn == null)
                continue;
            this.createLink(d, fromConn, toConn);
        }
    }
    DeleteLink(globalLinkIndex) {
        let l = this.links.get(globalLinkIndex);
        if (l == null) {
            throw Error("Link to delete is null");
        }
        l.RemoveFromDOM();
        this.links.delete(globalLinkIndex);
        l.To.RemoveLink(l);
        l.From.RemoveLink(l);
    }
    createLink(data, from, to) {
        if (this.options.onLinkCreate && !this.options.onLinkCreate(from.Caption, data))
            return null;
        if (!this.options.multipleLinksOnOutput && from.LinksLength > 0)
            return null;
        if (!this.options.multipleLinksOnInput && to.LinksLength > 0)
            return null;
        let l = new FlowchartLink_1.FlowchartLink(this, "", this.Options.defaultLinkColor, from, to);
        from.AddLink(l);
        to.AddLink(l);
        this.links.set(l.GlobalLinkIndex, l);
        return l;
    }
    createOperator(data) {
        let name = data.type + "Operator";
        if (!operatorimpl[name]) {
            throw new Error(`Unknown type ${data.type}`);
        }
        if (this.options.onOperatorCreate && !this.options.onOperatorCreate(data.caption, null, false)) {
            throw new Error(`Creation of operator ${data.type} prevented by onOperatorCreate plugin`);
        }
        let o = new operatorimpl[name](this, data.caption);
        o.MoveTo(data.posX, data.posY);
        this.operators.set(o.GlobalOperatorIndex, o);
        return o;
    }
    unsetTemporaryLink() {
        this.lastOutputConnectorClicked = null;
        this.tempLayer.style.visibility = "hidden";
    }
    setTemporaryLink(c) {
        this.lastOutputConnectorClicked = c;
        let color = Flowchart.DATATYPE2COLOR.get(c.Type);
        if (!color)
            color = "BLACK";
        this.markerArrow.style.fill = color;
        this.markerCircle.style.fill = color;
        this.tempLayer.style.visibility = "visible";
    }
    unselectOperator() {
        if (this.options.onOperatorUnselect && !this.options.onOperatorUnselect())
            return;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        if (this.selectedOperator == null)
            return;
        this.selectedOperator.ShowAsSelected(false);
        this.selectedOperator = null;
    }
    SelectOperator(operator) {
        if (this.options.onOperatorSelect && !this.options.onOperatorSelect(operator.Caption))
            return;
        this.unselectLink();
        if (this.selectedOperator != null)
            this.selectedOperator.ShowAsSelected(false);
        operator.ShowAsSelected(true);
        this.selectedOperator = operator;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        Flowchart.Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `Properties for ${this.selectedOperator.Caption}`);
        let table = Flowchart.Html(this.propertyGridHtmlDiv, "table", [], ["develop-propertygrid-table"]);
        let tr = Flowchart.Html(table, "tr", [], ["develop-propertygrid-tr"]);
        Flowchart.Html(tr, "th", [], ["develop-propertygrid-th"], "Key");
        Flowchart.Html(tr, "th", [], ["develop-propertygrid-th"], "Value");
        if (this.selectedOperator.PopulateProperyGrid(table)) {
            Flowchart.Html(this.propertyGridHtmlDiv, "button", [], ["develop-propertygrid-button"], `Save`);
        }
        else {
            this.propertyGridHtmlDiv.innerText = ""; //clear
            Flowchart.Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `No Properties for ${this.selectedOperator.Caption}`);
        }
    }
    // Found here : http://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
    static _shadeColor(color, percent) {
        var f = parseInt(color.slice(1), 16), t = percent < 0 ? 0 : 255, p = percent < 0 ? percent * -1 : percent, R = f >> 16, G = f >> 8 & 0x00FF, B = f & 0x0000FF;
        return "#" + (0x1000000 + (Math.round((t - R) * p) + R) * 0x10000 + (Math.round((t - G) * p) + G) * 0x100 + (Math.round((t - B) * p) + B)).toString(16).slice(1);
    }
    static Svg(parent, type, attributes, classes) {
        return Flowchart.Elem(Flowchart.SVGNS, parent, type, attributes, classes);
    }
    static Html(parent, type, attributes, classes, textContent) {
        return Flowchart.Elem(Flowchart.HTMLNS, parent, type, attributes, classes, textContent);
    }
    static Elem(ns, parent, type, attributes, classes, textContent) {
        let element = document.createElementNS(ns, type);
        if (classes) {
            for (const clazz of classes) {
                element.classList.add(clazz);
            }
        }
        let i;
        for (i = 0; i < attributes.length; i += 2) {
            element.setAttribute(attributes[i], attributes[i + 1]);
        }
        if (textContent) {
            element.textContent = textContent;
        }
        parent.appendChild(element);
        return element;
    }
}
exports.Flowchart = Flowchart;
Flowchart.SVGNS = "http://www.w3.org/2000/svg";
Flowchart.XLINKNS = "http://www.w3.org/1999/xlink";
Flowchart.HTMLNS = "http://www.w3.org/1999/xhtml";
Flowchart.DATATYPE2COLOR = new Map([[FlowchartConnector_1.ConnectorType.BOOLEAN, "RED"], [FlowchartConnector_1.ConnectorType.COLOR, "GREEN"], [FlowchartConnector_1.ConnectorType.FLOAT, "BLUE"], [FlowchartConnector_1.ConnectorType.INTEGER, "YELLOW"], [FlowchartConnector_1.ConnectorType.COLOR, "PURPLE"]]);
},{"./FlowchartConnector":3,"./FlowchartExporter":4,"./FlowchartLink":5,"./FlowchartOperator":6,"./FlowchartOperatorImpl":7,"./TopologicalSorfDFS":8,"./Utils":9}],3:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FlowchartOutputConnector = exports.FlowchartInputConnector = exports.FlowchartConnector = exports.ConnectorType = void 0;
const Flowchart_1 = require("./Flowchart");
const TRANSLATEY = 20;
var ConnectorType;
(function (ConnectorType) {
    ConnectorType[ConnectorType["UNDEFINED"] = 0] = "UNDEFINED";
    ConnectorType[ConnectorType["BOOLEAN"] = 1] = "BOOLEAN";
    ConnectorType[ConnectorType["INTEGER"] = 2] = "INTEGER";
    ConnectorType[ConnectorType["FLOAT"] = 3] = "FLOAT";
    ConnectorType[ConnectorType["COLOR"] = 4] = "COLOR";
})(ConnectorType = exports.ConnectorType || (exports.ConnectorType = {}));
class FlowchartConnector {
    constructor(parent, caption, type) {
        this.parent = parent;
        this.caption = caption;
        this.type = type;
        this.links = new Map();
        this.HasLink = (globalLinkIndex) => this.links.has(globalLinkIndex);
        this.AddLink = (link) => this.links.set(link.GlobalLinkIndex, link);
        this.RemoveLink = (link) => this.links.delete(link.GlobalLinkIndex);
        this.index = FlowchartConnector.INDEX++;
        let spec = this.getIOSpecifics();
        let translateY = TRANSLATEY * spec.parent.childElementCount;
        this.element = Flowchart_1.Flowchart.Svg(spec.parent, "g", ["transform", `translate(0 ${translateY})`], [`operator-${spec.inputOrOutput}`]);
        this.element.setAttribute("data-connector-datatype", ConnectorType[type]);
        let text = Flowchart_1.Flowchart.Svg(this.element, "text", ["dx", "" + spec.dx, "dy", "4"], [`operator-${spec.inputOrOutput}-caption`]);
        text.textContent = caption;
        this.connectorGroup = Flowchart_1.Flowchart.Svg(this.element, "g", []);
        this.connector = Flowchart_1.Flowchart.Svg(this.connectorGroup, "circle", ["r", "4"], [`operator-${spec.inputOrOutput}-connector`, ConnectorType[type]]);
        this.snapper = Flowchart_1.Flowchart.Svg(this.connectorGroup, "circle", ["r", "10"], [`operator-${spec.inputOrOutput}-snapper`]);
        this.element.onmouseover = (e) => {
            for (const link of this.links.values()) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.ColorizeLink(Flowchart_1.Flowchart._shadeColor(this.parent.Parent.Options.defaultLinkColor, -0.4));
                }
            }
        };
        this.element.onmouseout = (e) => {
            for (const link of this.links.values()) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.UncolorizeLink();
                }
            }
        };
    }
    get GlobalConnectorIndex() { return this.index; }
    get Element() { return this.element; }
    get LinksLength() { return this.links.size; }
    ;
    GetLinksCopy() {
        return Array.from(this.links.values());
    }
    get LinksKVIt() { return this.links.entries(); }
    RefreshLinkPositions() {
        this.links.forEach(l => {
            l.RefreshPosition();
        });
    }
    get Parent() { return this.parent; }
    get Caption() { return this.caption; }
    get Type() { return this.type; }
    GetLinkpoint() {
        let flowchart = this.Parent.Parent;
        let posrat = flowchart.PositionRatio;
        let flowchartRect = flowchart.Element.getBoundingClientRect();
        let connectorRect = this.connector.getBoundingClientRect();
        var x = (connectorRect.left - flowchartRect.left) / posrat + connectorRect.width / 2;
        var y = (connectorRect.top - flowchartRect.top) / posrat + connectorRect.height / 2;
        return { x: x, y: y };
    }
}
exports.FlowchartConnector = FlowchartConnector;
FlowchartConnector.INDEX = 0;
class FlowchartInputConnector extends FlowchartConnector {
    constructor(parent, caption, type) {
        super(parent, caption, type);
        this.connectorGroup.onmouseup = (e) => {
            parent.Parent._notifyInputConnectorMouseup(this, e);
        };
        this.connectorGroup.onmouseenter = (e) => {
            parent.Parent._notifyInputConnectorMouseenter(this, e);
        };
        this.connectorGroup.onmouseleave = (e) => {
            parent.Parent._notifyInputConnectorMouseleave(this, e);
        };
    }
    GetLinkpointXOffset(width) { return 0; }
    getIOSpecifics() { return { inputOrOutput: "input", parent: this.Parent.InputSvgG, translateY: 0, dx: 8 }; }
}
exports.FlowchartInputConnector = FlowchartInputConnector;
class FlowchartOutputConnector extends FlowchartConnector {
    constructor(parent, caption, type) {
        super(parent, caption, type);
        this.element.onmousedown = (e) => {
            parent.Parent._notifyOutputConnectorMousedown(this, e);
        };
    }
    GetLinkpointXOffset(width) { return width; }
    getIOSpecifics() { return { inputOrOutput: "output", parent: this.Parent.OutputSvgG, translateY: 140, dx: -8 }; }
}
exports.FlowchartOutputConnector = FlowchartOutputConnector;
},{"./Flowchart":2}],4:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FlowchartExporter = void 0;
const FlowchartConnector_1 = require("./FlowchartConnector");
class FlowchartExporter {
    static getRandomValuesWithMathRandom(bytes) {
        const max = Math.pow(2, 8 * bytes.byteLength / bytes.length);
        for (let i = 0, r; i < bytes.length; i++) {
            bytes[i] = Math.random() * max;
        }
    }
    static getRandomBytes(length) {
        const bytes = new Uint8Array(length);
        if (typeof crypto !== 'undefined') {
            crypto.getRandomValues(bytes);
        }
        else if (typeof msCrypto !== 'undefined') {
            msCrypto.getRandomValues(bytes);
        }
        else {
            FlowchartExporter.getRandomValuesWithMathRandom(bytes);
        }
        return bytes;
    }
    ;
    static Export(operators) {
        let typeIndex2globalConnectorIndex2adressOffset = new Map(); //globalConnectorIndex_Outputs 2 variableAdress
        let typeIndex2maxOffset = new Map();
        for (let type in FlowchartConnector_1.ConnectorType) {
            if (!isNaN(Number(type))) {
                typeIndex2globalConnectorIndex2adressOffset.set(Number(type), new Map());
                typeIndex2maxOffset.set(Number(type), 2);
            }
        }
        //Iteriere über alle Output-Connectoren aller Operatoren
        //Ein Output, der beschaltet ist, entspricht einer Speicheradresse.
        //Unbeschaltete Outputs schreiben in die Speicheradresse 0. 
        //Unbeschaltete Inputs lesen von der Speicheradresse 1
        //Echte Speicheradressen gibt es dann ab Index 2
        //In den Maps stehen die Zuordnung Globaler Connector Index --> Index der Speicheraddresse
        //Außerdem bekannt: Wie viele Speicheradressen von jedem Typ benötigen wir
        for (const operator of operators) {
            for (const outputKV of operator.OutputsKVIt) {
                if (outputKV[1].LinksLength == 0) {
                    //unconnected output -->writes to memory adress zero of the respective data type
                    typeIndex2globalConnectorIndex2adressOffset.get(outputKV[1].Type).set(outputKV[1].GlobalConnectorIndex, 0);
                }
                else {
                    //connected output --> create new memory address and set it
                    let index = typeIndex2maxOffset.get(outputKV[1].Type);
                    typeIndex2globalConnectorIndex2adressOffset.get(outputKV[1].Type).set(outputKV[1].GlobalConnectorIndex, index);
                    index++;
                    typeIndex2maxOffset.set(outputKV[1].Type, index);
                }
            }
        }
        /*
        Lege nun die Operatoren in der durch das Array vorgegebenen Struktur in ein Array ab
        */
        let buffer = new ArrayBuffer(Math.pow(2, 16));
        let ctx = { typeIndex2globalConnectorIndex2adressOffset: typeIndex2globalConnectorIndex2adressOffset, buffer: new DataView(buffer), bufferOffset: 0 };
        //Version of Data Structure
        ctx.buffer.setUint32(ctx.bufferOffset, 0xAFFECAFE, true); //Version 0xAFFECAFE means: Development
        ctx.bufferOffset += 4;
        //GUID
        let guid = FlowchartExporter.getRandomBytes(16);
        guid.forEach((v, i) => { ctx.buffer.setUint8(ctx.bufferOffset + i, v); }); //guid of the model
        ctx.bufferOffset += 16;
        for (const operator of operators) {
            operator.SerializeToBinary(ctx);
        }
        let code = "const uint8_t code[] = {";
        for (let i = 0; i < ctx.bufferOffset; i++) {
            code += "0x" + ctx.buffer.getUint8(i).toString(16) + ", ";
        }
        code += "};";
        window.alert(code);
        //var file = new Blob([buffer.slice(0,ctx.bufferOffset)], {type: "application/octet-stream"});
        //URL.createObjectURL(file)
        return guid;
    }
}
exports.FlowchartExporter = FlowchartExporter;
},{"./FlowchartConnector":3}],5:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FlowchartLink = void 0;
const Flowchart_1 = require("./Flowchart");
class FlowchartLink {
    constructor(parent, caption, color, from, to) {
        this.parent = parent;
        this.caption = caption;
        this.color = color;
        this.from = from;
        this.to = to;
        this.index = FlowchartLink.MAX_INDEX++;
        this.element = Flowchart_1.Flowchart.Svg(parent.LinkLayer, "path", ["stroke-width", "" + this.parent.Options.linkWidth, "fill", "none", "id", "LINK" + this.index]);
        this.RefreshPosition();
        this.UncolorizeLink();
        this.parent.LinkLayer.appendChild(this.element);
        this.captionElement = Flowchart_1.Flowchart.Svg(parent.LinkLayer, "text", []);
        let captionPath = Flowchart_1.Flowchart.Svg(this.captionElement, "textPath", ["startOffset", "50%", "text-anchor", "middle"]);
        captionPath.setAttributeNS(Flowchart_1.Flowchart.XLINKNS, "href", '#' + "LINK" + this.index);
        captionPath.innerHTML = caption;
        this.element.onclick = (e) => {
            this.parent._notifyLinkClicked(this, e);
        };
    }
    get GlobalLinkIndex() { return this.index; }
    get From() { return this.from; }
    get To() { return this.to; }
    set Color(color) { this.color = color; }
    RemoveFromDOM() {
        this.element.remove();
        this.captionElement.remove();
    }
    ColorizeLink(color) {
        this.element.setAttribute('stroke', color);
        //this.element.setAttribute('fill', color);
        //TODO: colorize the small triangle in the connector
        //linkData.internal.els.fromSmallConnector.css('border-left-color', color);
        //linkData.internal.els.toSmallConnector.css('border-left-color', color);
    }
    UncolorizeLink() {
        this.ColorizeLink(this.parent.Options.defaultLinkColor);
    }
    RefreshPosition() {
        let fromPosition = this.from.GetLinkpoint();
        let toPosition = this.to.GetLinkpoint();
        let fromX = fromPosition.x;
        let fromY = fromPosition.y + this.parent.Options.linkVerticalDecal;
        let toX = toPosition.x;
        let toY = toPosition.y + this.parent.Options.linkVerticalDecal;
        let distanceFromArrow = this.parent.Options.distanceFromArrow;
        let bezierFromX = (fromX + distanceFromArrow);
        let bezierToX = toX + 1;
        let bezierIntensity = Math.min(100, Math.max(Math.abs(bezierFromX - bezierToX) / 2, Math.abs(fromY - toY)));
        this.element.setAttribute("d", 'M' + bezierFromX + ',' + (fromY) + ' C' + (fromX + distanceFromArrow + bezierIntensity) + ',' + fromY + ' ' + (toX - bezierIntensity) + ',' + toY + ' ' + bezierToX + ',' + toY);
    }
}
exports.FlowchartLink = FlowchartLink;
FlowchartLink.MAX_INDEX = 0;
},{"./Flowchart":2}],6:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FlowchartOperator = exports.TypeInfo = exports.SingletonType = exports.PositionType = void 0;
const Flowchart_1 = require("./Flowchart");
const Utils_1 = require("./Utils");
var PositionType;
(function (PositionType) {
    PositionType[PositionType["Default"] = 0] = "Default";
    PositionType[PositionType["Input"] = 1] = "Input";
    PositionType[PositionType["Output"] = 2] = "Output";
})(PositionType = exports.PositionType || (exports.PositionType = {}));
;
var SingletonType;
(function (SingletonType) {
    SingletonType[SingletonType["Default"] = 0] = "Default";
    SingletonType[SingletonType["Singleton"] = 1] = "Singleton";
})(SingletonType = exports.SingletonType || (exports.SingletonType = {}));
;
class TypeInfo {
    constructor(GlobalTypeIndex, Position, Singleton) {
        this.GlobalTypeIndex = GlobalTypeIndex;
        this.Position = Position;
        this.Singleton = Singleton;
    }
}
exports.TypeInfo = TypeInfo;
class FlowchartOperator {
    constructor(parent, caption, typeInfo) {
        this.parent = parent;
        this.caption = caption;
        this.typeInfo = typeInfo;
        //der Index der Inputs ist rein lokal und beginnt bei 0 fortlaufend
        this.Inputs = [];
        //der Index der Outputs ist rein lokal und beginnt bei 0 fortlaufend
        this.Outputs = [];
        this.x = 0;
        this.y = 0;
        this.GetOutputConnectorByIndex = (i) => this.Outputs[i];
        this.GetInputConnectorByIndex = (i) => this.Inputs[i];
        this.index = FlowchartOperator.MAX_INDEX++;
        this.elementSvgG = Flowchart_1.Flowchart.Svg(parent.OperatorsLayer, "g", [], ["operator"]);
        this.elementSvgG.setAttribute('data-operator-index', "" + this.index);
        this.box = Flowchart_1.Flowchart.Svg(this.elementSvgG, "rect", ["width", "140", "height", "100", "rx", "10", "ry", "10"], ["operator-box"]);
        let title = Flowchart_1.Flowchart.Svg(this.elementSvgG, "text", ["x", "5", "y", "21"], ["operator-title"]);
        title.textContent = caption;
        this.inputSvgG = Flowchart_1.Flowchart.Svg(this.elementSvgG, "g", ["transform", "translate(0 50)"], ["operator-inputs"]);
        this.outputSvgG = Flowchart_1.Flowchart.Svg(this.elementSvgG, "g", ["transform", "translate(140 50)"], ["operator-outputs"]);
        this.debugInfoSvgText = Flowchart_1.Flowchart.Svg(this.elementSvgG, "text", ["x", "0", "y", "100"], ["operator-debuginfo"]);
        this.debugInfoSvgText.textContent = "No debug info";
        this.box.onclick = (e) => {
            console.log("FlowchartOperator this.box.onclick");
            parent._notifyOperatorClicked(this, e);
        };
        if (this.parent.Options.canUserMoveOperators) {
            title.onmousedown = (e) => {
                this.RegisterDragging(e);
            };
        }
    }
    get GlobalOperatorIndex() { return this.index; }
    get ElementSvgG() { return this.elementSvgG; }
    get InputSvgG() { return this.inputSvgG; }
    get OutputSvgG() { return this.outputSvgG; }
    get TypeInfo() { return this.typeInfo; }
    ShowAsSelected(state) {
        if (state) {
            this.box.classList.add('selected');
        }
        else {
            this.box.classList.remove('selected');
        }
    }
    SetDebugInfoText(text) {
        this.debugInfoSvgText.textContent = text;
    }
    RegisterDragging(e) {
        let offsetInOperator = Utils_1.Utils.EventCoordinatesInSVG(e, this.ElementSvgG); //offset innerhalb des Operators
        //Wir benötigen den Offset zwischen der aktuellen Position des Objektes und 
        let offsetX = e.clientX - this.x;
        let offsetY = e.clientY - this.y;
        document.onmouseup = (e) => {
            document.onmouseup = null;
            document.onmousemove = null;
        };
        document.onmousemove = e => {
            //TODO: neue Position nur setzen, wenn this.element.clientRect innerhalb von parent.clientRectangle ist
            this.MoveTo(e.clientX - offsetX, e.clientY - offsetY);
        };
    }
    get Parent() { return this.parent; }
    ;
    get Caption() { return this.caption; }
    get InputsKVIt() { return this.Inputs.entries(); }
    get OutputsKVIt() { return this.Outputs.entries(); }
    RemoveFromDOM() {
        this.elementSvgG.remove();
    }
    AppendConnectors(inputs, outputs) {
        if (this.Inputs.length != 0 || this.Outputs.length != 0)
            throw new Error("AppendConnectors may only be called once!");
        for (const i of inputs) {
            if (i.Parent != this)
                continue;
            this.Inputs.push(i);
        }
        for (const o of outputs) {
            if (o.Parent != this)
                continue;
            this.Outputs.push(o);
        }
        let num = Math.max(this.Inputs.length, this.Outputs.length);
        let height = 50 + num * 20 + 10;
        this.box.setAttribute("height", "" + height);
        this.debugInfoSvgText.setAttribute("y", "" + height);
        //TODO RedrawConnectors; Connectors zeichnen sich nicht im Construktur, sondern erst nach dem Appenden, um die Reihenfolgen in derser Liste und im DOM gleich zu haben
    }
    MoveTo(x, y) {
        let g = this.parent.Options.grid;
        this.x = Math.round(x / g) * g;
        this.y = Math.round(y / g) * g;
        this.elementSvgG.setAttribute("transform", `translate(${this.x} ${this.y})`);
        for (const c of this.Inputs) {
            c.RefreshLinkPositions();
        }
        for (const c of this.Outputs) {
            c.RefreshLinkPositions();
        }
    }
    PopulateProperyGrid(parent) {
        //let tr=Flowchart.Html(parent, "tr", [],["develop-propertygrid-tr"]);
        //Flowchart.Html(tr, "td", [],["develop-propertygrid-td"], "AKey");
        //Flowchart.Html(tr, "td", [],["develop-propertygrid-td"], "AValue");
        return false;
    }
    SerializeInputsAndOutputs(ctx) {
        for (const input of this.Inputs) {
            let variableAdress = 0;
            let links = input.GetLinksCopy();
            if (links.length == 0) {
                variableAdress = 1; //because unconnected inputs read from adress 1 (which is "false", 0, 0.0, black...)
            }
            else {
                let out = links[0].From;
                variableAdress = ctx.typeIndex2globalConnectorIndex2adressOffset.get(out.Type).get(out.GlobalConnectorIndex) || 1;
            }
            ctx.buffer.setUint32(ctx.bufferOffset, variableAdress, true);
            ctx.bufferOffset += 4;
        }
        for (const output of this.Outputs) {
            let variableAdress = 0;
            if (output.LinksLength == 0) {
                variableAdress = 0; //because unconnected outputs write to adress 0 (which is never read!)
            }
            else {
                variableAdress = ctx.typeIndex2globalConnectorIndex2adressOffset.get(output.Type).get(output.GlobalConnectorIndex) || 1;
            }
            ctx.buffer.setUint32(ctx.bufferOffset, variableAdress, true);
            ctx.bufferOffset += 4;
        }
    }
    SerializeToBinary(ctx) {
        //serialize Type
        ctx.buffer.setUint32(ctx.bufferOffset, this.TypeInfo.GlobalTypeIndex, true);
        ctx.bufferOffset += 4;
        //Index of instance
        ctx.buffer.setUint32(ctx.bufferOffset, this.GlobalOperatorIndex, true);
        ctx.bufferOffset += 4;
        this.SerializeInputsAndOutputs(ctx);
        this.SerializeFurtherProperties(ctx);
        //ctx.buffer.setUint32(ctx.bufferOffset, 0xAFFECAFE, true);
        //ctx.bufferOffset+=4;
    }
    SerializeFurtherProperties(mapper) {
        return;
    }
}
exports.FlowchartOperator = FlowchartOperator;
FlowchartOperator.MAX_INDEX = 0;
},{"./Flowchart":2,"./Utils":9}],7:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ConstINTOperator = exports.ConstTRUEOperator = exports.GreenLedOperator = exports.YellowLedOperator = exports.RedLedOperator = exports.RelayOperator = exports.TempSensorOperator = exports.MoveSensorOperator = exports.RedButtonOperator = exports.EncoderButtonOperator = exports.GreenButtonOperator = exports.NotOperator = exports.RSOperator = exports.MINOperator = exports.MAXOperator = exports.MULTOperator = exports.ADDOperator = exports.OROperator = exports.ANDOperator = void 0;
const FlowchartOperator_1 = require("./FlowchartOperator");
const Flowchart_1 = require("./Flowchart");
const FlowchartConnector_1 = require("./FlowchartConnector");
const ANDOperator_TypeIndex = 1;
class ANDOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(1, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.ANDOperator = ANDOperator;
class OROperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(2, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.OROperator = OROperator;
class ADDOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(3, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.INTEGER);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.INTEGER);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.ADDOperator = ADDOperator;
class MULTOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(4, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.INTEGER);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.INTEGER);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.MULTOperator = MULTOperator;
class MAXOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(5, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.INTEGER);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.INTEGER);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.MAXOperator = MAXOperator;
class MINOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(6, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.INTEGER);
        let B = new FlowchartConnector_1.FlowchartInputConnector(this, "B", FlowchartConnector_1.ConnectorType.INTEGER);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}
exports.MINOperator = MINOperator;
class RSOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(7, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let R = new FlowchartConnector_1.FlowchartInputConnector(this, "R", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let S = new FlowchartConnector_1.FlowchartInputConnector(this, "S", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}
exports.RSOperator = RSOperator;
class NotOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(8, FlowchartOperator_1.PositionType.Default, FlowchartOperator_1.SingletonType.Default));
        let A = new FlowchartConnector_1.FlowchartInputConnector(this, "A", FlowchartConnector_1.ConnectorType.BOOLEAN);
        let C = new FlowchartConnector_1.FlowchartOutputConnector(this, "C", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }
}
exports.NotOperator = NotOperator;
class GreenButtonOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(9, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Singleton));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "IsPressed", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.GreenButtonOperator = GreenButtonOperator;
class EncoderButtonOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(10, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Singleton));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "IsPressed", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.EncoderButtonOperator = EncoderButtonOperator;
class RedButtonOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(11, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Singleton));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "IsPressed", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.RedButtonOperator = RedButtonOperator;
class MoveSensorOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(12, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Singleton));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "Movement", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.MoveSensorOperator = MoveSensorOperator;
class TempSensorOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(13, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Singleton));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "Temperatur", FlowchartConnector_1.ConnectorType.FLOAT);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.TempSensorOperator = TempSensorOperator;
class RelayOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(14, FlowchartOperator_1.PositionType.Output, FlowchartOperator_1.SingletonType.Singleton));
        let conn = new FlowchartConnector_1.FlowchartInputConnector(this, "Relay", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId = "4711";
    }
}
exports.RelayOperator = RelayOperator;
class RedLedOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(15, FlowchartOperator_1.PositionType.Output, FlowchartOperator_1.SingletonType.Singleton));
        let conn = new FlowchartConnector_1.FlowchartInputConnector(this, "LED", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId = "4711";
    }
}
exports.RedLedOperator = RedLedOperator;
class YellowLedOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(16, FlowchartOperator_1.PositionType.Output, FlowchartOperator_1.SingletonType.Singleton));
        let conn = new FlowchartConnector_1.FlowchartInputConnector(this, "LED", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId = "4711";
    }
}
exports.YellowLedOperator = YellowLedOperator;
class GreenLedOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(17, FlowchartOperator_1.PositionType.Output, FlowchartOperator_1.SingletonType.Singleton));
        let conn = new FlowchartConnector_1.FlowchartInputConnector(this, "LED", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId = "4711";
    }
}
exports.GreenLedOperator = GreenLedOperator;
class ConstTRUEOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(18, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Default));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "TRUE", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
}
exports.ConstTRUEOperator = ConstTRUEOperator;
class ConstINTOperator extends FlowchartOperator_1.FlowchartOperator {
    constructor(parent, caption) {
        super(parent, caption, new FlowchartOperator_1.TypeInfo(19, FlowchartOperator_1.PositionType.Input, FlowchartOperator_1.SingletonType.Default));
        let O = new FlowchartConnector_1.FlowchartOutputConnector(this, "Out", FlowchartConnector_1.ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId = "4711";
    }
    PopulateProperyGrid(parent) {
        PropertyGridHelpers.Number(parent, "Constant", -32768, 32767);
        return true;
    }
}
exports.ConstINTOperator = ConstINTOperator;
class PropertyGridHelpers {
    static Number(table, key, min, max) {
        let tr = Flowchart_1.Flowchart.Html(table, "tr", [], ["develop-propertygrid-tr"]);
        Flowchart_1.Flowchart.Html(tr, "td", [], ["develop-propertygrid-td"], key);
        let inputContainer = Flowchart_1.Flowchart.Html(tr, "td", [], ["develop-propertygrid-td"]);
        Flowchart_1.Flowchart.Html(inputContainer, "input", ["type", "number", "min", "" + Math.round(min), "max", "" + Math.round(max)]);
    }
}
},{"./Flowchart":2,"./FlowchartConnector":3,"./FlowchartOperator":6}],8:[function(require,module,exports){
"use strict";
//https://en.wikipedia.org/wiki/Topological_sorting
Object.defineProperty(exports, "__esModule", { value: true });
exports.TopologicalSortDFS = exports.NodeWrapper = void 0;
var MarkerState;
(function (MarkerState) {
    MarkerState[MarkerState["NONE"] = 0] = "NONE";
    MarkerState[MarkerState["TEMPORARY"] = 1] = "TEMPORARY";
    MarkerState[MarkerState["PERMANENT"] = 2] = "PERMANENT";
})(MarkerState || (MarkerState = {}));
class NodeWrapper {
    constructor(payload) {
        this.payload = payload;
        this.Mark = MarkerState.NONE;
        this.DependendNodes = [];
    }
    get Payload() { return this.payload; }
}
exports.NodeWrapper = NodeWrapper;
;
class TopologicalSortDFS {
    constructor() {
        this.L = new Array();
    }
    sort(headNode) {
        this.L = new Array();
        headNode.forEach((n) => this.visit(n));
        return this.L;
    }
    visit(n) {
        if (n.Mark == MarkerState.PERMANENT)
            return;
        if (n.Mark == MarkerState.TEMPORARY)
            throw new Error("not a DAG");
        n.Mark = MarkerState.TEMPORARY;
        for (const d of n.DependendNodes) {
            this.visit(d);
        }
        n.Mark = MarkerState.PERMANENT;
        this.L.push(n);
    }
}
exports.TopologicalSortDFS = TopologicalSortDFS;
},{}],9:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Utils = void 0;
class Utils {
    static EventCoordinatesInSVG(evt, element, positionRatio = 1) {
        let rect = element.getBoundingClientRect();
        return { x: (evt.clientX - rect.left) / positionRatio, y: (evt.clientY - rect.top) / positionRatio };
    }
}
exports.Utils = Utils;
},{}]},{},[1])

//# sourceMappingURL=bundle.js.map
