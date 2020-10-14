#include <Arduino.h>

#define SENSOR_INPUT      2
// alle variablen, die im Interrupt-handler gebötigt werden, mit "volatile" kennzeichnen
volatile long low_impulse_start = 0;
// the distance once it's been measured
volatile long concentration = 0;

//Interrupt-Handler mit IRAM_ATTR kennzeichen
void IRAM_ATTR sensorHandling(){
    // don't do anything if no pulse has been sent
    if(digitalRead(SENSOR_INPUT)=0)
    {
        low_impulse_start=micros();
    }
    else
    {
        uint64_t time_passed_by = (micros() - low_impulse_start);
        concentration= 42;//...

    }
}

void setup(){
    Serial.begin(115200); //serielle Kommunikation
    pinMode(SENSOR_INPUT, INPUT_PULLUP ); //an den Pin schließen Sie den Sensor an
    attachInterrupt(SENSOR_INPUT, sensorHandling, CHANGE);
}

void loop() {
  Serial.print("Concentration: ");
  Serial.println(concentration);
  delay(1000);
}