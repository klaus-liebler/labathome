#include "Arduino.h"
#include "USBPowerDelivery.h"
#include <Wire.h>
#include <pins.hh>
#include "stm32_esp32_communication.hh"

namespace I2C_SLAVE
{
  constexpr uint8_t DATA_TO_SEND_LENGHT{30};
  uint8_t number[30];
  uint8_t test[DATA_TO_SEND_LENGHT];
  void onI2CReceive(int byteNum){
    return;
  }

    void onI2CRequest(){
    return;
  }
}

#undef Serial
#define Serial Serial4
HardwareSerial Serial4(PIN::UART4_RX, PIN::UART4_TX);

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
#ifdef DEBUG
  delay(3000); // to give platformIO terminal enough time
#endif
  Serial4.println("Application started");
  
  Wire.setSDA(PIN::SDA);
  Wire.setSCL(PIN::SCL);
  Wire.onReceive(I2C_SLAVE::onI2CReceive);
  Wire.onRequest(I2C_SLAVE::onI2CRequest);
  Wire.begin(I2C::STM32_I2C_ADDRESS);
 


  PowerSink.start(handleEvent);
}

void loop()
{
  PowerSink.poll();
/*
  for(uint8_t address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();

    if (!error)
    {
      Serial.printf("I2C device found at address 0x%02X\n", address);
    }
  }
  delay(5000);
 */ 
}