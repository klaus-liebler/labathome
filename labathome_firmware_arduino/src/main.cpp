#include <Arduino.h>
#include <Wire.h>
// I2C Konfiguration
const uint8_t STM32_I2C_ADDRESS = 0x7E;
const int I2C_SDA_PIN = 5;
const int I2C_SCL_PIN = 6;
const uint32_t I2C_FREQUENCY = 100000; // 100 kHz
#include "communication.h"




void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ESP32-S3 I2C Master Initialisierung");
    
    // I2C initialisieren
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    
    // Initialwerte für Send-Daten setzen
    initializeSendData();
    
    Serial.println("I2C Master bereit");
    Serial.printf("S2E_t Größe: %d Bytes\n", S2E_s);
    Serial.printf("E2S_t Größe: %d Bytes\n", E2S_s);
}

uint32_t lastDebugTime = 0;
void loop() {
    // Hauptkommunikationszyklus
    if (communicateWithSTM32()) {
        // Erfolgreiche Kommunikation
        processReceivedData();
        updateSendData();
        
        // Optional: Debug-Ausgabe
        if (millis() - lastDebugTime > 2000) {
            printDebugInfo();
            lastDebugTime = millis();
        }
    } else {
        // Fehlerbehandlung
        handleCommunicationError();
    }
    
    delay(10); // Kurze Pause zwischen den Zyklen
}



void processReceivedData() {
    // Hier die empfangenen Daten verarbeiten
    // Beispiel: Buttons, ADC-Werte, etc.
    
    if (receivedData.ButtonRed) {
        // Rote Button wurde gedrückt
        // Reaktion implementieren
    }
    
    if (receivedData.ButtonYellow) {
        // Gelbe Button wurde gedrückt
        // Reaktion implementieren
    }
    
    // ADC-Werte können hier verarbeitet werden
    // receivedData.Adc0, receivedData.Adc1, etc.
}

void updateSendData() {
    // Hier die zu sendenden Daten basierend auf
    // Programm-Logik aktualisieren
    
    // Beispiel: Fan-Geschwindigkeit basierend auf Temperatur steuern
    // if (receivedData.Adc2_24V > 2000) {
    //     sendData.Fan = 255; // Maximalgeschwindigkeit
    // } else {
    //     sendData.Fan = 128; // Halbe Geschwindigkeit
    // }
    
    // Beispiel: LED-Helligkeit vom Rotary Encoder übernehmen
    // sendData.LedPower = receivedData.Rotenc >> 8; // MSB des Encoders
}

