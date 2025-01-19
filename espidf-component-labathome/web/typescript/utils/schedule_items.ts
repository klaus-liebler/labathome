import { html, TemplateResult } from "lit-html";
import {IAppManagement} from "./interfaces"
import { RequestSchedulerOpen } from "../../generated/flatbuffers/scheduler/request-scheduler-open";
import { eSchedule } from "../../generated/flatbuffers/scheduler/e-schedule";
import { OneWeekIn15Minutes } from "../../generated/flatbuffers/scheduler/one-week-in15-minutes";
import { ResponseSchedulerOpen } from "../../generated/flatbuffers/scheduler/response-scheduler-open";
import { SunRandom } from "../../generated/flatbuffers/scheduler/sun-random";
import { uSchedule } from "../../generated/flatbuffers/scheduler/u-schedule";
import { Requests, Responses } from "../../generated/flatbuffers/webmanager";
import * as flatbuffers from 'flatbuffers';
import { RequestSchedulerSave } from "../../generated/flatbuffers/scheduler/request-scheduler-save";
import { Schedule } from "../../generated/flatbuffers/scheduler/schedule";
import { OneWeekIn15MinutesData } from "../../generated/flatbuffers/scheduler/one-week-in15-minutes-data";
import { iSunRandomDialogHandler, iWeeklyScheduleDialogHandler } from "../dialog_controller/weeklyschedule_dialog";
import { RequestSchedulerDelete } from "../../generated/flatbuffers/scheduler/request-scheduler-delete";
import { SunRandomScheduleEditorDialog } from "../dialog_controller/SunRandomScheduleEditorDialog";
import { RequestSchedulerRename } from "../../generated/flatbuffers/scheduler/request-scheduler-rename";


