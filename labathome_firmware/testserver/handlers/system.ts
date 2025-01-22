import * as flatbuffers from "flatbuffers"
import * as system from "../generated/flatbuffers/system"
import { ISender } from "..";
export function handleSystem(buffer: flatbuffers.ByteBuffer, sender:ISender){
    var rw=system.RequestWrapper.getRootAsRequestWrapper(buffer)
    switch (rw.requestType()) {
        case system.Requests.RequestSystemData:{
            let b = new flatbuffers.Builder(1024);
            var partitionsOffset = new Array<number>();
            partitionsOffset.push(system.PartitionInfo.createPartitionInfo(b, b.createString("Label0"), 0, 0x10, 3072, 1, true, b.createString("AppName"), b.createString("AppVersion"), b.createString("AppDate"), b.createString("AppTime")));
            partitionsOffset.push(system.PartitionInfo.createPartitionInfo(b, b.createString("Label1"), 1, 0x01, 16384, 1, true, b.createString("AppName"), b.createString("AppVersion"), b.createString("AppDate"), b.createString("AppTime")));
            system.ResponseSystemData.startResponseSystemData(b);
            system.ResponseSystemData.addChipCores(b, 2);
            system.ResponseSystemData.addChipFeatures(b, 255);
            system.ResponseSystemData.addPartitions(b, system.ResponseSystemData.createPartitionsVector(b, partitionsOffset));
            system.ResponseSystemData.addChipModel(b, 2);
            system.ResponseSystemData.addChipRevision(b, 3);
            system.ResponseSystemData.addChipTemperature(b, 23.4);
            system.ResponseSystemData.addFreeHeap(b, 1203);
            system.ResponseSystemData.addMacAddressBt(b, system.Mac6.createMac6(b, [1, 2, 3, 4, 5, 6]));
            system.ResponseSystemData.addMacAddressEth(b, system.Mac6.createMac6(b, [1, 2, 3, 4, 5, 6]));
            system.ResponseSystemData.addMacAddressIeee802154(b, system.Mac6.createMac6(b, [1, 2, 3, 4, 5, 6]));
            system.ResponseSystemData.addMacAddressWifiSoftap(b, system.Mac6.createMac6(b, [1, 2, 3, 4, 5, 6]));
            system.ResponseSystemData.addMacAddressWifiSta(b, system.Mac6.createMac6(b, [1, 2, 3, 4, 5, 6]));
            system.ResponseSystemData.addSecondsEpoch(b, BigInt(Math.floor(new Date().getTime() / 1000)));
            system.ResponseSystemData.addSecondsUptime(b, BigInt(10));
            let rsd = system.ResponseSystemData.endResponseSystemData(b);
            b.finish(system.ResponseWrapper.createResponseWrapper(b, system.Responses.ResponseSystemData, rsd));
            sender.send(system.Namespace.Value, b);
        }
        break;
    }
}