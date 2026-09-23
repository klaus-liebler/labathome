// Funktionsdeklarationen
void initializeSendData();
bool communicateWithSTM32();
void processReceivedData();
void updateSendData();
void printDebugInfo();
void handleCommunicationError();
bool readFromSTM32();
bool writeToSTM32();

// Definition der Strukturen (wie gegeben)
struct __attribute__((packed)) S2E_t
{
    uint8_t Status;
    uint8_t ButtonRed:1;
    uint8_t ButtonYellow:1;
    uint8_t Movement:1;
    uint8_t Blfault:1;
    uint8_t PIN_PB12:1;
    uint16_t Rotenc;
    uint16_t Brightness;
    uint16_t UsbpdVoltage_mv;
    uint16_t Adc0;
    uint16_t Adc1;
    uint16_t Adc2_24V;
    uint16_t Adc3_Bl_i_sense;
};

struct __attribute__((packed)) E2S_t{
    uint8_t AddressPointer;
    uint8_t Relay:1;
    uint8_t Blreset:1;
    uint8_t Blsleep:1;
    uint8_t LcdReset:1;
    uint16_t UsbpdVoltage_mv;
    uint16_t Dac0;
    uint16_t Dac1;
    uint8_t Servo[6];
    uint8_t Fan;
    uint8_t LedPower;
    uint8_t Heater;
    uint8_t ThreephaseMode;
    uint8_t ThreephaseP1;
    uint8_t ThreephaseP2;
    uint8_t ThreephaseP3;
};

constexpr int E2S_s = sizeof(E2S_t);
constexpr int S2E_s = sizeof(S2E_t);



// Globale Variablen
S2E_t receivedData;    // Daten vom STM32 (gelesen)
E2S_t sendData;        // Daten zum STM32 (geschrieben)

// I2C Kommunikations-Status
bool i2cCommunicationOk = false;
uint32_t lastSuccessfulCommunication = 0;

bool communicateWithSTM32() {
    // Daten vom STM32 lesen (S2E_t)
    if (!readFromSTM32()) {
        return false;
    }
    
    // Daten an STM32 schreiben (E2S_t)
    if (!writeToSTM32()) {
        return false;
    }
    
    i2cCommunicationOk = true;
    lastSuccessfulCommunication = millis();
    return true;
}

bool readFromSTM32() {
    Wire.requestFrom(STM32_I2C_ADDRESS, (size_t)S2E_s);
    
    if (Wire.available() < S2E_s) {
        Serial.println("Fehler: Nicht genug Daten vom STM32 empfangen");
        return false;
    }
    
    // Daten Byte für Byte einlesen
    uint8_t* dataPtr = (uint8_t*)&receivedData;
    for (int i = 0; i < S2E_s; i++) {
        dataPtr[i] = Wire.read();
    }
    
    return true;
}

bool writeToSTM32() {
    Wire.beginTransmission(STM32_I2C_ADDRESS);
    
    // Daten Byte für Byte senden
    uint8_t* dataPtr = (uint8_t*)&sendData;
    for (int i = 0; i < E2S_s; i++) {
        Wire.write(dataPtr[i]);
    }
    
    if (Wire.endTransmission() != 0) {
        Serial.println("Fehler beim Senden an STM32");
        return false;
    }
    
    return true;
}

void initializeSendData() {
    // Initialwerte für alle Daten setzen
    memset(&sendData, 0, sizeof(sendData));
    
    // Standardwerte setzen
    sendData.AddressPointer = 0;
    sendData.Relay = 0;
    sendData.Blreset = 0;
    sendData.Blsleep = 0;
    sendData.LcdReset = 1; // LCD normalerweise reset
    sendData.UsbpdVoltage_mv = 5000; // 5V Standard
    sendData.Dac0 = 0;
    sendData.Dac1 = 0;
    memset(sendData.Servo, 0, sizeof(sendData.Servo));
    sendData.Fan = 0;
    sendData.LedPower = 0;
    sendData.Heater = 0;
    sendData.ThreephaseMode = 0;
    sendData.ThreephaseP1 = 0;
    sendData.ThreephaseP2 = 0;
    sendData.ThreephaseP3 = 0;
}

void handleCommunicationError() {
    i2cCommunicationOk = false;
    
    // Nur alle 5 Sekunden Fehler melden um Serial nicht zu überfluten
    static uint32_t lastErrorPrint = 0;
    if (millis() - lastErrorPrint > 5000) {
        Serial.println("I2C Kommunikationsfehler mit STM32");
        lastErrorPrint = millis();
        
        // Versuche I2C-Bus zu resetten
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    }
}

void printDebugInfo() {
    Serial.println("=== I2C Kommunikation Status ===");
    Serial.printf("Letzte erfolgreiche Kommunikation: %lu ms\n", 
                  millis() - lastSuccessfulCommunication);
    Serial.printf("I2C Status: %s\n", i2cCommunicationOk ? "OK" : "FEHLER");
    
    // Einige empfangene Werte anzeigen
    Serial.printf("Rotary Encoder: %d\n", receivedData.Rotenc);
    Serial.printf("USB PD Voltage: %d mV\n", receivedData.UsbpdVoltage_mv);
    Serial.printf("Button Red: %d, Yellow: %d\n", 
                  receivedData.ButtonRed, receivedData.ButtonYellow);
    Serial.printf("ADC0: %d, ADC1: %d\n", receivedData.Adc0, receivedData.Adc1);
    Serial.println();
}

// Zusätzliche Hilfsfunktionen
void setRelay(bool state) {
    sendData.Relay = state ? 1 : 0;
}

void setLedPower(uint8_t power) {
    sendData.LedPower = power;
}

void setFanSpeed(uint8_t speed) {
    sendData.Fan = speed;
}

void setServoPosition(uint8_t servoIndex, uint8_t position) {
    if (servoIndex < 6) {
        sendData.Servo[servoIndex] = position;
    }
}

void setUsbPdVoltage(uint16_t voltage_mv) {
    sendData.UsbpdVoltage_mv = voltage_mv;
}