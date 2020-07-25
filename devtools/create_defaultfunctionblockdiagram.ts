import {flatbuffers} from "flatbuffers"
import { labathome } from './fbexecutable_generated';
import * as fs from 'fs';

//Zwei Taster gehen auf AND mit nachgeschaltetem NOT und nachgeschalteter LED und auf ein RS mit nachgeschalteter LED

let builder = new flatbuffers.Builder(0);

labathome.FbAnd2Configuration.startFbAnd2Configuration(builder);
labathome.FbAnd2Configuration.addInputA(builder, 1);
labathome.FbAnd2Configuration.addInputB(builder, 2);
labathome.FbAnd2Configuration.addOutput(builder, 2048);
let and =labathome.FbAnd2Configuration.endFbAnd2Configuration(builder);
labathome.ConfigurationWrapper.startConfigurationWrapper(builder);
labathome.ConfigurationWrapper.addItemType(builder, labathome.FbConfiguration.FbAnd2Configuration);
labathome.ConfigurationWrapper.addItem(builder, and);
let and_wrapper=labathome.ConfigurationWrapper.endConfigurationWrapper(builder);

labathome.FbNotConfiguration.startFbNotConfiguration(builder);
labathome.FbNotConfiguration.addInput(builder, 2048);
labathome.FbNotConfiguration.addOutput(builder, 1024);
var not = labathome.FbNotConfiguration.endFbNotConfiguration(builder);
labathome.ConfigurationWrapper.startConfigurationWrapper(builder);
labathome.ConfigurationWrapper.addItemType(builder, labathome.FbConfiguration.FbNotConfiguration);
labathome.ConfigurationWrapper.addItem(builder, not);
let not_wrapper=labathome.ConfigurationWrapper.endConfigurationWrapper(builder);

labathome.FbRSConfiguration.startFbRSConfiguration(builder);
labathome.FbRSConfiguration.addInputR(builder, 1);
labathome.FbRSConfiguration.addInputS(builder, 2);
labathome.FbRSConfiguration.addOutput(builder, 1025);
let rs = labathome.FbRSConfiguration.endFbRSConfiguration(builder);
labathome.ConfigurationWrapper.startConfigurationWrapper(builder);
labathome.ConfigurationWrapper.addItemType(builder, labathome.FbConfiguration.FbRSConfiguration);
labathome.ConfigurationWrapper.addItem(builder, rs);
let rs_wrapper=labathome.ConfigurationWrapper.endConfigurationWrapper(builder);


let configs = labathome.FbExecutable.createFbConfigVector(builder, [rs_wrapper, and_wrapper, not_wrapper]);

labathome.FbExecutable.startFbExecutable(builder);
labathome.FbExecutable.addId(builder, builder.createLong(42,0));
labathome.FbExecutable.addTimestamp(builder, builder.createLong(1,0));
labathome.FbExecutable.addFbConfig(builder, configs);
labathome.FbExecutable.addMaxBinaryIndex(builder, 2049);
labathome.FbExecutable.addMaxIntegerIndex(builder, 2048);
labathome.FbExecutable.addMaxDoubleIndex(builder, 2048);

let fbExecutable = labathome.FbExecutable.endFbExecutable(builder);
builder.finish(fbExecutable);

let filename = "defaultfunctionblockdiagram.data";
fs.writeFileSync(filename, builder.asUint8Array());

let data = new Uint8Array(fs.readFileSync(filename));
let buf = new flatbuffers.ByteBuffer(data);
let fbe = labathome.FbExecutable.getRootAsFbExecutable(buf);

console.log(fbe.id());
console.log(fbe.maxBinaryIndex());
for (let index = 0; index < fbe.fbConfigLength(); index++) {
    let type = fbe.fbConfig(index).itemType();
    console.log("Type: "+type);
}