#include <Arduino.h>

//Bitte hier die richtige Board-Version einbinden
#include <labathomeV3.h>


BSP b(IO17_MODE::BUZZER);

//--------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Board Test");
  b.initBoard();
}

//--------------------------------------------------------------------------------------------------------------------------------
int angle_s1=0;
int angle_s2=0;
int duty=0;

uint32_t lastSensorReadout=0;

void loop() {
  b.setLEDState(LED::LED_RED, b.getButtonIsPressed(Switch::SW_RED)?CRGB::Red:CRGB::Black);
  b.setLEDState(LED::LED_YELLOW, b.getButtonIsPressed(Switch::SW_YELLOW)?CRGB::Yellow:CRGB::Black);
  b.setLEDState(LED::LED_GREEN, b.getButtonIsPressed(Switch::SW_GREEN)?CRGB::Green:CRGB::Black);
  b.setLEDState(LED::LED_3, b.isMovementDetected()?CRGB::White:CRGB::Black);
  b.setRELAYState(b.getButtonIsPressed(Switch::SW_GREEN));
  if(b.getButtonIsPressed(Switch::SW_YELLOW))
  {
    b.startBuzzer(440+2.0*angle_s1);
  }
  else
  {
    b.endBuzzer();
  }
  

  //Servo
  b.setServo(Servo::Servo1, angle_s1<180?angle_s1:360-angle_s1);
  b.setServo(Servo::Servo2, angle_s2<180?angle_s2:360-angle_s2);
  angle_s1+=5;
  angle_s2+=10;
  angle_s1=angle_s1>360?0:angle_s1;
  angle_s2=angle_s2>360?0:angle_s2;
  

  //LED POWER WHITE
  //b.setLED_POWER_WHITE_DUTY(duty<100?duty:200-duty);
  b.setLED_POWER_WHITE_DUTY(0);
  duty++;
  duty=duty>200?0:duty;
  
  //Sensors
  uint32_t now = millis();
  if(now-lastSensorReadout>5000)
  {
      Serial.print("Humidity: ");
      Serial.print(b.bme280.readFloatHumidity(), 0);

      Serial.print(" Pressure: ");
      Serial.print(b.bme280.readFloatPressure(), 0);

      Serial.print(" Alt: ");
      //Serial.print(mySensor.readFloatAltitudeMeters(), 1);
      Serial.print(b.bme280.readFloatAltitudeFeet(), 1);

      Serial.print(" Temp: ");
      //Serial.print(mySensor.readTempC(), 2);
      Serial.print(b.bme280.readTempC(), 2);
      Serial.println();
      float lux = b.bh1750.readLightLevel();
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println(" lx");
      lastSensorReadout=now;
  }
  

  delay(50);


}