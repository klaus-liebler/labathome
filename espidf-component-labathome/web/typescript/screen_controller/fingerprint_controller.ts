import { Ref, createRef, ref } from "lit-html/directives/ref.js";
import { Finger, Namespace, NotifyEnrollNewFinger, NotifyFingerDetected, RequestCancelInstruction, RequestDeleteAllFingers, RequestDeleteFinger, RequestEnrollNewFinger, RequestFingerActionManually, RequestFingerprintSensorInfo, RequestFingers, RequestRenameFinger, RequestStoreFingerAction, RequestStoreFingerSchedule, RequestWrapper, Requests, ResponseDeleteFinger, ResponseEnrollNewFinger, ResponseFingerprintSensorInfo, ResponseFingers, ResponseWrapper, Responses } from "../../generated/flatbuffers/fingerprint";
import { ScreenController } from "./screen_controller";
import * as flatbuffers from 'flatbuffers';
import { Html, Severity } from "../utils/common";
import { html } from "lit-html";

import square_plus from '../../svgs/solid/square-plus.svg?raw'
import rotate from '../../svgs/solid/rotate.svg?raw'
import trash from '../../svgs/solid/trash.svg?raw'
import ban from '../../svgs/solid/ban.svg?raw'
import door_open from '../../svgs/solid/door-open.svg?raw'
import info from '../../svgs/solid/info.svg?raw'
import music from '../../svgs/solid/music.svg?raw'
import { unsafeSVG } from "lit-html/directives/unsafe-svg.js";
import { FilenameDialog, OkCancelDialog, OkDialog } from "../dialog_controller/dialog_controller";

enum RET {
    OK = 0x00,                           //!< Command execution is complete
    PACKET_RECIEVE_ERR = 0x01,           //!< Error when receiving data package
    NO_FINGER_ON_SENSOR = 0x02,          //!< No finger on the sensor
    ENROLL_FAIL = 0x03,                  //!< Failed to enroll the finger
    GENERATE_CHARACTER_FILE_FAIL = 0x06, //!< Failed to generate character file due to overly disorderly fingerprint image
    FEATURE_FAIL = 0x07,                 //!< Failed to generate character file due to the lack of character point or small fingerprint image
    NO_MATCH = 0x08,                     //!< Finger doesn't match
    FINGER_NOT_FOUND = 0x09,             //!< Failed to find matching finger
    FAILTO_COMBINE_FINGER_FILES = 0x0A,  //!< Failed to combine the character files
    BAD_LOCATION = 0x0B,                 //!< Addressed PageID is beyond the finger library
    DB_RANGE_FAIL = 0x0C,                //!< Error when reading template from library or invalid template
    UPLOAD_TEMPLATE_FAIL = 0x0D,         //!< Error when uploading template
    PACKETRESPONSEFAIL = 0x0E,           //!< Module failed to receive the following data packages
    UPLOADFAIL = 0x0F,                   //!< Error when uploading image
    DELETEFAIL = 0x10,                   //!< Failed to delete the template
    DBCLEARFAIL = 0x11,                  //!< Failed to clear finger library
    WRONG_PASSWORD = 0x13,               //!< wrong password!
    INVALIDIMAGE = 0x15,                 //!< Failed to generate image because of lac of valid primary image
    FLASHERR = 0x18,                     //!< Error when writing flash
    NO_DEFINITION_ERROR = 0x19,
    INVALIDREG = 0x1A, //!< Invalid register number
    INCORRECT_CONFIGURATION = 0x1b,
    WRONG_NOTEPAD_PAGE_NUMBER = 0x1c,
    COMMUNICATION_PORT_FAILURE = 0x1d,
    FINGERPRINT_LIBRARY_IS_FULL = 0x1f,

    ADDRESS_CODE_INCORRECT = 0x20,
    PASSWORT_MUST_BE_VERIFIED = 0x21,     // password must be verified;
    FINGERPRINT_TEMPLATE_IS_EMPTY = 0x22, // fingerprint template is empty;
    FINGERPRINT_LIB_IS_EMPTY = 0x24,
    TIMEOUT = 0x26,
    FINGERPRINT_ALREADY_EXISTS = 0x27,
    SENSOR_HARDWARE_ERROR = 0x29,

    UNSUPPORTED_COMMAND = 0xfc,
    HARDWARE_ERROR = 0xfd,
    COMMAND_EXECUTION_FAILURE = 0xfe,

    xPARSER_CANNOT_FIND_STARTCODE = 0x100,
    xPARSER_WRONG_MODULE_ADDRESS = 0x101,
    xPARSER_ACKNOWLEDGE_PACKET_EXPECTED = 0x102,
    xPARSER_UNEXPECTED_LENGTH = 0x103,
    xPARSER_CHECKSUM_ERROR = 0x104,
    xPARSER_TIMEOUT = 0x105,
    xNVS_READWRITE_ERROR = 0x106,
    xNVS_NAME_ALREADY_EXISTS = 0x107,
    xNVS_NAME_UNKNOWN = 0x108,
    xNVS_NAME_TOO_LONG = 0x109,
    xCANNOT_GET_MUTEX = 0x10A,
    xNVS_NOT_AVAILABLE = 0x10B,
    xNAME_IS_NULL = 0x10C,
};

const index2action=[
    "Open Door Side",
    "Open Door Front",
    "Open Garage",
    "Alarm Silent",
    "Play MissionImpossible"
];

export class FingerprintScreenController extends ScreenController {


    private tblFingers: Ref<HTMLTableSectionElement> = createRef();
    private tblFingerprintSensorInfo: Ref<HTMLTableSectionElement> = createRef();
    private fingerIndex2tr = new Map<number, HTMLTableRowElement>();
    private fingerIndex2name = new Map<number, string>();

    private scheduleNames:Array<string>=[]

    public Template = () => html`
    <h1>Current Fingers </h1>
        <div class="buttons">
            <button class="withsvg" @click=${() => this.btnUpdateFingers()}>${unsafeSVG(rotate)}<span>Update List<span></button>
            <button class="withsvg" @click=${() => this.btnFingerprintEnroll()}>${unsafeSVG(square_plus)}<span>Enroll new Finger<span></button>
            <button class="withsvg" @click=${() => this.btnDeleteAll()}>${unsafeSVG(trash)}<span>Delete All<span></button>
            <button class="withsvg" @click=${() => this.btnCancelInstruction()}>${unsafeSVG(ban)}<span>Cancel Instruction<span></button>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Name</th>
                    <th>Index</th>
                    <th>Scheduler</th>
                    <th>Action</th>
                    <th>Manage Entry</th>
                </tr>

            </thead>
            <tbody ${ref(this.tblFingers)}></tbody>
        </table>
        
        <h1>System</h1>
        <div class="buttons">
            <button class="withsvg" @click=${() => this.btnManualAction(0)}>${unsafeSVG(door_open)}<span>Open Door<span></button>
            <button class="withsvg" @click=${() => this.btnManualAction(4)}>${unsafeSVG(music)}<span>Play Melody<span></button>
            <button class="withsvg" @click=${() => this.btnFingerprintGetSensorInfo()}>${unsafeSVG(info)}<span>Get Sensor Info<span></button>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Property</th>
                    <th>Value</th>
                </tr>
            </thead>
            <tbody ${ref(this.tblFingerprintSensorInfo)}></tbody>
        </table>`



    public sendRequestDeleteFinger(name: string) {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestDeleteFinger, RequestDeleteFinger.createRequestDeleteFinger(b, b.createString(name))))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }

    private sendRequestStoreFingerTimetable(fingerIndex: number, scheduleName: string) {
        console.log(`sendRequestStoreFingerTimetable fingerIndex=${fingerIndex} scheduleName=${scheduleName}`)
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestStoreFingerSchedule, RequestStoreFingerSchedule.createRequestStoreFingerSchedule(b, fingerIndex, b.createString(scheduleName))))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }

    private sendRequestStoreFingerAction(fingerIndex: number, actionIndex: number) {
        console.log(`sendRequestStoreFingerAction fingerIndex=${fingerIndex} actionIndex=${actionIndex}`)
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestStoreFingerAction, RequestStoreFingerAction.createRequestStoreFingerAction(b, fingerIndex, actionIndex)))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }

    private sendRequestRenameFinger(fingerIndex: number, oldName:string,  newName: string) {
        console.log(`sendRequestRenameFinger fingerIndex=${fingerIndex} newName=${newName}`)
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestRenameFinger, RequestRenameFinger.createRequestRenameFinger(b, b.createString(oldName), b.createString(newName))))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }

    private insertParameter(name: string, value: string | number) {
        var row = this.tblFingerprintSensorInfo.value!.insertRow();
        row.insertCell().textContent = name;
        row.insertCell().textContent = value.toString();
    }

    private insertFinger(f: Finger): void {
        var row = this.tblFingers.value!.insertRow();
        this.fingerIndex2tr.set(f.index(), row);
        this.fingerIndex2name.set(f.index(), f.name()!)

        row.insertCell().textContent = f.name();
        row.insertCell().textContent = String(f.index());
        var cell = row.insertCell();
        var timeSelect = (<HTMLSelectElement>Html(cell, "select", []));
        var fingersScheduleName = f.scheduleName();
        this.scheduleNames.forEach(scheduleName => {
            timeSelect.options.add(new Option(scheduleName, scheduleName, fingersScheduleName==scheduleName, fingersScheduleName==scheduleName));
        });
        (<HTMLInputElement>Html(cell, "input", ["type", "button", "value", `✔Save`])).onclick = () => {
            this.sendRequestStoreFingerTimetable(f.index()!, timeSelect.value);
        };


        cell = row.insertCell();
        var actionSelect = (<HTMLSelectElement>Html(cell, "select", []));
        var fingersActionIndex = f.actionIndex()
        index2action.forEach((a,i) => {
            actionSelect.options.add(new Option(a, a, i==fingersActionIndex, i==fingersActionIndex));
        });
        (<HTMLInputElement>Html(cell, "input", ["type", "button", "value", `✔Save`])).onclick = () => {
            this.sendRequestStoreFingerAction(f.index()!, actionSelect.selectedIndex);
        };


        cell = row.insertCell();
        (<HTMLInputElement>Html(cell, "input", ["type", "button", "value", `❌Delete`])).onclick = () => {
            this.sendRequestDeleteFinger(f!.name()!);
        };
        (<HTMLInputElement>Html(cell, "input", ["type", "button", "value", `✎Rename`])).onclick = () => {
            this.appManagement.ShowDialog(new FilenameDialog("Enter new finger name", (ok, value) => {
                if (!ok) return;
                this.sendRequestRenameFinger(f!.index(), f!.name()!, value);
            }))
        }
    }

    OnMessage(namespace:number, bb: flatbuffers.ByteBuffer): void {
        if(namespace!=Namespace.Value) return;
        let messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb);
        switch (messageWrapper.responseType()) {
            case Responses.ResponseFingerprintSensorInfo: {

                let m = <ResponseFingerprintSensorInfo>messageWrapper.response(new ResponseFingerprintSensorInfo());
                var usedIndices = "";
                var fingerIndex = 0;
                for (var bi = 0; bi < 32; bi++) {
                    var byte = m.libraryUsedIndices().v(bi)
                    for (var biti = 0; biti < 8; biti++) {
                        if (byte & (1 << biti)) {
                            usedIndices += `${fingerIndex},`
                        }
                        fingerIndex++;
                    }
                }

                this.tblFingerprintSensorInfo.value!.textContent = "";
                this.insertParameter("Status", m.status());
                this.insertParameter("Security Level", m.securityLevel());
                this.insertParameter("Library Size Max", m.librarySizeMax() + " Fingers");
                this.insertParameter("Library Size Used", m.librarySizeUsed() + " Fingers");
                this.insertParameter("Library Used Indices", usedIndices);
                this.insertParameter("Device Address", "0x" + m.deviceAddress().toString(16));
                this.insertParameter("DataPacketSizeCode", Math.pow(2, 5 + m.dataPacketSizeCode()) + "byte");
                this.insertParameter("Firmware", m.fwVer()!);
                this.insertParameter("Algorithm", m.algVer()!);
                this.insertParameter("Baud Rate", m.baudRateTimes9600() * 9600 + "baud");
                break;
            }
            case Responses.ResponseFingers:
                {
                    let m = <ResponseFingers>messageWrapper.response(new ResponseFingers());
                    this.tblFingers.value!.textContent = "";
                    this.fingerIndex2name.clear();
                    this.fingerIndex2tr.clear();
                    this.scheduleNames=[];
                    for (let i = 0; i < m.scheduleNamesLength(); i++) {
                        this.scheduleNames.push(m.scheduleNames(i))
                    }
                    for (let i = 0; i < m.fingersLength(); i++) {
                        var f = m.fingers(i);
                        if (!f) continue;
                        this.insertFinger(f)
                    }
                   
                    break;
                }
            case Responses.ResponseDeleteFinger: {
                var m = <ResponseDeleteFinger>messageWrapper.response(new ResponseDeleteFinger());
                var ret = <RET>m.errorcode();
                if (ret != RET.OK) {
                    this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `Error while deleting Finger ${m.name()}: ${RET[ret]}.`));
                } else {
                    this.appManagement.ShowDialog(new OkDialog(Severity.SUCCESS, `Finger ${m.name()} successfully deleted.`));
                    this.sendRequestFingers();
                }
                break;
            }
            case Responses.ResponseEnrollNewFinger: {
                let m = <ResponseEnrollNewFinger>messageWrapper.response(new ResponseEnrollNewFinger());
                var ret = <RET>m.errorcode();
                console.log(`ResponseEnrollNewFinger: errorcode=${RET[ret]}`);
                if (ret != RET.OK) {
                    this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `Enrollment could not be started: ${RET[ret]}`));
                } else {
                    this.appManagement.ShowSnackbar(Severity.SUCCESS, `Enrollment successfully started. Put your finger on the sensor`);
                }
                break;
            }
            case Responses.NotifyEnrollNewFinger: {
                let m = <NotifyEnrollNewFinger>messageWrapper.response(new NotifyEnrollNewFinger());
                var ret = <RET>m.errorcode();
                console.log(`NotifyEnrollNewFinger: step=${m.step()}, name=${m.name()}, errorcode=${RET[ret]}`);
                if (m.step() < 13) {
                    var reading = Math.ceil(m.step() / 2)
                    var collectImage = m.step() % 2 == 1;
                    this.appManagement.ShowSnackbar(
                        ret == RET.OK ? Severity.INFO : Severity.WARN,
                        `Reading ${reading}: ${collectImage ? "Collect Image" : "Generate Feature"}: ${RET[ret]}.`);
                }
                else if (m.step() == 13) {
                    this.appManagement.ShowSnackbar(ret == RET.OK ? Severity.INFO : Severity.WARN, `Repeat fingerprint check: ${RET[ret]}.`);
                }
                else if (m.step() == 14) {
                    this.appManagement.ShowSnackbar(ret == RET.OK ? Severity.INFO : Severity.WARN, `Merge feature: ${RET[ret]}.`);
                }
                else if (m.step() == 15) {
                    this.appManagement.ShowSnackbar(ret == RET.OK ? Severity.SUCCESS : Severity.WARN, `Fingerprint stored in Sensor with  name "${m.name()}" on index ${m.index()}: ${RET[ret]}.`);
                }
                else {
                    this.appManagement.ShowSnackbar(Severity.ERROR, `Unknown step: ${m.step()}, errorcode: ${RET[ret]}.!`);
                }
                break;
            }
            case Responses.NotifyFingerDetected: {
                let m = <NotifyFingerDetected>messageWrapper.response(new NotifyFingerDetected());
                if (m.errorcode() != 0) {
                    this.appManagement.ShowSnackbar(Severity.WARN, `Finger has not been detected.`);
                    console.warn(`Finger has not been detected.`)
                    return;
                }
                var fingerName = this.fingerIndex2name.get(m.index());
                if (!fingerName) {
                    this.appManagement.ShowSnackbar(Severity.SUCCESS, `Unknown Finger ${m.index()} detected with score ${m.score()}.`);
                    console.info(`Unknown Finger ${m.index()} detected with score ${m.score()}.`);
                    return;
                }
                var tr = this.fingerIndex2tr.get(m.index())!;
                this.appManagement.ShowSnackbar(Severity.SUCCESS, `Finger ${fingerName} (${m.index()}) detected with score ${m.score()}.`);
                console.info(`Finger ${fingerName} (${m.index()}) detected with score ${m.score()}.`);
                var storedBgColor = tr.style.backgroundColor;
                tr.style.backgroundColor = "green";
                setTimeout(() => { tr.style.backgroundColor = storedBgColor; }, 2000);
                break;
            }

            default:
                break;
        }
    }

    private btnManualAction(actionIndex:number) {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestFingerActionManually, RequestFingerActionManually.createRequestFingerActionManually(b, 0, actionIndex)))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }
    btnUpdateFingers() {
        this.sendRequestFingers();
    }
    btnFingerprintEnroll() {
        this.appManagement.ShowDialog(new FilenameDialog("Enter name of finger", (ok, name) => {
            if (!ok) {
                return
            }
            console.log(`Send RequestEnrollNewFinger name=${name}`)
            let b = new flatbuffers.Builder(1024);
            b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestEnrollNewFinger, RequestEnrollNewFinger.createRequestEnrollNewFinger(b, b.createString(name))))
            this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
        }))
    }

    btnFingerprintGetSensorInfo() {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestFingerprintSensorInfo, RequestFingerprintSensorInfo.createRequestFingerprintSensorInfo(b))) 
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
        
    }
    btnDeleteAll() {
        this.appManagement.ShowDialog(new OkCancelDialog(Severity.WARN, "Please confirm to DELETE ALL FINGERPRINTS", (ok: boolean) => { 
            if (!ok) return;
            let b = new flatbuffers.Builder(1024);
            b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestDeleteAllFingers, RequestDeleteAllFingers.createRequestDeleteAllFingers(b)))
            this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
        }))
    }
    btnCancelInstruction() {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestCancelInstruction, RequestCancelInstruction.createRequestCancelInstruction(b)))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
    }
    OnCreate(): void {
        this.appManagement.RegisterWebsocketMessageNamespace(this, Namespace.Value);
    }

    private sendRequestFingers() {
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestWrapper.createRequestWrapper(b,Requests.RequestFingers, RequestFingers.createRequestFingers(b)))
        this.appManagement.SendFinishedBuilder(Namespace.Value, b, 3000);
       
    }

    OnFirstStart(): void {
        this.sendRequestFingers();
    }
    OnRestart(): void {
        this.sendRequestFingers();
    }
    OnPause(): void {
    }

}
