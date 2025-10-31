#include <Arduino.h>
#include <Wire.h>
// I2C Konfiguration
constexpr uint8_t STM32_I2C_ADDRESS = 0x7E;
constexpr int I2C_SDA_PIN = 5;
constexpr int I2C_SCL_PIN = 6;
constexpr uint32_t I2C_FREQUENCY = 100000; // 100 kHz

// Pin Definitionen
constexpr gpio_num_t PIN_BTN_GREEN = (gpio_num_t)0;

constexpr gpio_num_t PIN_CANRX = (gpio_num_t)1;
constexpr gpio_num_t PIN_CANTX = (gpio_num_t)2;
constexpr gpio_num_t PIN_EXT_CS = (gpio_num_t)3;
constexpr gpio_num_t PIN_I2C_IRQ = (gpio_num_t)4;
constexpr gpio_num_t PIN_I2C_SDA = (gpio_num_t)5;
constexpr gpio_num_t PIN_I2C_SCL = (gpio_num_t)6;

constexpr gpio_num_t PIN_uSD_CMD = (gpio_num_t)7;
constexpr gpio_num_t PIN_LCD_CLK = (gpio_num_t)8;

constexpr gpio_num_t PIN_EXT_MISO = (gpio_num_t)9;
constexpr gpio_num_t PIN_EXT_CLK = (gpio_num_t)10;
constexpr gpio_num_t PIN_EXT_IO1 = (gpio_num_t)11;
constexpr gpio_num_t PIN_EXT_IO2 = (gpio_num_t)12;

constexpr gpio_num_t PIN_LED_WS2812 = (gpio_num_t)13;

constexpr gpio_num_t PIN_I2S_MCLK = (gpio_num_t)14;

constexpr gpio_num_t PIN_uSD_CLK = (gpio_num_t)15;
constexpr gpio_num_t PIN_uSD_D0 = (gpio_num_t)16;

constexpr gpio_num_t PIN_LCD_BL = (gpio_num_t)17;
constexpr gpio_num_t PIN_LCD_DC = (gpio_num_t)18;

constexpr gpio_num_t PIN_TXD0 = (gpio_num_t)43;
constexpr gpio_num_t PIN_RXD0 = (gpio_num_t)44;

constexpr gpio_num_t PIN_RS485_DI = (gpio_num_t)40;
constexpr gpio_num_t PIN_RS485_DE = (gpio_num_t)41;
constexpr gpio_num_t PIN_RS485_RO = (gpio_num_t)42;


constexpr gpio_num_t PIN_I2S_FS = (gpio_num_t)21;
constexpr gpio_num_t PIN_I2S_DAC = (gpio_num_t)45;
constexpr gpio_num_t PIN_EXT_MOSI = (gpio_num_t)46;
constexpr gpio_num_t PIN_I2S_ADC = (gpio_num_t)47;
constexpr gpio_num_t PIN_I2S_BCLK = (gpio_num_t)48;

constexpr gpio_num_t PIN_LCD_DAT = (gpio_num_t)38;
constexpr gpio_num_t PIN_LCD_RESET = (gpio_num_t)35;


constexpr gpio_num_t PIN_ONEWIRE = (gpio_num_t)39;
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
    //sendData.LedPower = receivedData.Rotenc >> 8; // MSB des Encoders
}

