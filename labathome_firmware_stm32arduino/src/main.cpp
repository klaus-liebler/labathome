#include "Arduino.h"
#include "USBPowerDelivery.h"

#undef Serial
#define Serial Serial4

HardwareSerial Serial4(PC11, PC10);

void requestVoltage()
{
  // check if 20V is supported
  for (int i = 0; i < PowerSink.numSourceCapabilities; i += 1)
  {
    if (PowerSink.sourceCapabilities[i].minVoltage <= 20000 && PowerSink.sourceCapabilities[i].maxVoltage >= 20000)
    {
      PowerSink.requestPower(20000);
      return;
    }
  }

  // check if 15V is supported
  for (int i = 0; i < PowerSink.numSourceCapabilities; i += 1)
  {
    if (PowerSink.sourceCapabilities[i].minVoltage <= 15000 && PowerSink.sourceCapabilities[i].maxVoltage >= 15000)
    {
      PowerSink.requestPower(15000);
      return;
    }
  }
  Serial.println("Neither 20V nor 15V is supported");
}

void handleEvent(PDSinkEventType eventType)
{

  if (eventType == PDSinkEventType::sourceCapabilitiesChanged)
  {
    // source capabilities have changed
    if (PowerSink.isConnected())
    {
      Serial.println("PD supply is connected");
      for (int i = 0; i < PowerSink.numSourceCapabilities; i += 1)
      {
        Serial.printf("minVoltage: %d maxVoltage:%d current:%d\n", PowerSink.sourceCapabilities[i].minVoltage, PowerSink.sourceCapabilities[i].maxVoltage, PowerSink.sourceCapabilities[i].maxCurrent);
      }
      requestVoltage();
    }
    else
    {
      // no supply or no USB PD capable supply is connected
      PowerSink.requestPower(5000); // reset to 5V
    }
  }
  else if (eventType == PDSinkEventType::voltageChanged)
  {
    // voltage has changed
    if (PowerSink.activeVoltage != 0)
    {
      Serial.printf("Voltage: %d mV @ %d mA (max)", PowerSink.activeVoltage, PowerSink.activeCurrent);
      Serial.println();
    }
    else
    {
      Serial.println("Disconnected");
    }
  }
  else if (eventType == PDSinkEventType::powerRejected)
  {
    // rare case: power supply rejected requested power
    Serial.println("Power request rejected");
    Serial.printf("Voltage: %d mV @ %d mA (max)", PowerSink.activeVoltage, PowerSink.activeCurrent);
  }
}

void setup()
{
  Serial4.begin(115200);
  Serial4.println("Application started");
  PowerSink.start(handleEvent);

  // Uncomment if using X-NUCLEO-SNK1MK1 shield
  // NucleoSNK1MK1.init();
}

void loop()
{
  PowerSink.poll();
}