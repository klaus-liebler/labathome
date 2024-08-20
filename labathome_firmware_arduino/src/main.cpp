#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(4,5);
}

void loop() {

  int nDevices=0;
 
  Serial.println("Scanning...");
  for(uint8_t address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.printf("Found device at address 0x%02X\n", address);
      nDevices++;
    }
    else if(error==4)
    {
      Serial.printf("Unknown error at address 0x%02X\n", address);
    }   
    delay(20); 
  }
  if (nDevices == 0)
    Serial.printf("Done. No I2C devices found!\n\n");
  else
    Serial.printf("Done. Found %d device(s).\n\n", nDevices);
 
  delay(5000);           // wait 5 seconds for next scan
}

