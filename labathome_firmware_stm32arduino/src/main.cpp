#include <cstdio>
#include "Arduino.h"
#include "USBPowerDelivery.h"
#include "log.h"
#include <Wire.h>
#include <common.hh>
#include <single_led.hh>
#include "Encoder.hh"

#include <version_15_0/pins.hh>
#include "stm32_esp32_communication.hh"

constexpr time_t TIMEOUT_FOR_FAILSAFE{3000};
bool noDataWarningAlreadyPrinted{false};
bool gotDataInfoAlreadyPrinted{false};

// Time related
time_t timeToPutActorsInFailsafe{TIMEOUT_FOR_FAILSAFE};

// Manager
SINGLE_LED::M *led{nullptr};

HardwareTimer *TIM_BL_DRV{nullptr}; // TIM1 for Brushless
constexpr uint32_t BL_DRV_PH1_CH = 1;
constexpr uint32_t BL_DRV_PH2_CH = 2;
constexpr uint32_t BL_DRV_PH3_CH = 3;
ROTARY_ENCODER::M encoder(PA0, PA1, ROTARY_ENCODER::MODE::SINGLE);
constexpr uint32_t ROTENC_A_CH = 1;
constexpr uint32_t ROTENC_B_CH = 2;
HardwareTimer *TIM_HALL{nullptr}; // TIM3 for Hall Sensors, ch 2,3,4
constexpr uint32_t HALL_PH1_CH_CH = 4;
constexpr uint32_t HALL_PH2_CH_CH = 3;
constexpr uint32_t HALL_PH3_CH_CH = 2;

HardwareTimer *TIM_LED_WHITE{nullptr}; // TIM4 for Power LED, ch 4
constexpr uint32_t LED_WHITE_CH = 4;

HardwareTimer *TIM_SERVO_0_1{nullptr}; // TIM15 for Servo0+1, ch1,2
constexpr uint32_t SERVO0_CH = 1;
constexpr uint32_t SERVO1_CH = 2;

HardwareTimer *TIM_SERVO_2{nullptr}; // TIM16 for Servo2, ch1
constexpr uint32_t SERVO2_CH = 1;

HardwareTimer *TIM_FAN{nullptr}; // TIM17 for Fan, ch1
constexpr uint32_t FAN_CH = 1;

SINGLE_LED::BlinkPattern WAITING_FOR_CONNECTION(500, 500);
SINGLE_LED::BlinkPattern UNDER_CONTROL_FROM_MASTER(100, 900);
SINGLE_LED::BlinkPattern PROBLEM(100, 100);

void SetPhysicalOutputs();

namespace I2C_SLAVE
{

  uint8_t e2s_buffer[E2S::ESP2STM_SIZE];
  uint8_t s2e_buffer[S2E::STM2ESP_SIZE];
  size_t currentAddress = 0; // Aktuelle Lese-/Schreibadresse
  void onI2CReceive(int howMany)
  {
    if (howMany >= 1)
    {
      // Erstes empfangenes Byte ist die Adresse
      currentAddress = Wire.read();
      howMany--;
    }

    while (howMany-- > 0)
    {
      uint8_t data = Wire.read();
      currentAddress %= E2S::ESP2STM_SIZE;
      e2s_buffer[currentAddress++] = data;
    }
    SetPhysicalOutputs();
  }

  void onI2CRequest()
  {
    currentAddress %= S2E::STM2ESP_SIZE;
    Wire.write(s2e_buffer[currentAddress]);
    currentAddress++;
  }
}

namespace USB_PD
{
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

  void handleUsbPdEvent(PDSinkEventType eventType)
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
}

unsigned int degree2us(unsigned int degrees)
{
  return 1000 + degrees * 150 / 27;
}

void SetServoAngle(uint8_t servo_0_1_2_3, uint8_t angle_0_180)
{
  if (angle_0_180 > 180)
    angle_0_180 = 180;
  uint32_t us = degree2us(angle_0_180);
  log_info("Set Servo %d to %lu us", servo_0_1_2_3, us);
  switch (servo_0_1_2_3)
  {
  case 0:
    TIM_SERVO_0_1->setCaptureCompare(SERVO0_CH, us, MICROSEC_COMPARE_FORMAT);
    break;
  case 1:
    TIM_SERVO_0_1->setCaptureCompare(SERVO1_CH, us, MICROSEC_COMPARE_FORMAT);
    break;
  case 2:
    TIM_SERVO_2->setCaptureCompare(SERVO2_CH, us, MICROSEC_COMPARE_FORMAT);
    break;
  default:
    break;
  }
}

void SetFanSpeed(uint8_t power_0_100)
{
  if (power_0_100 > 100)
  {
    power_0_100 = 100;
  }
  TIM_FAN->setCaptureCompare(FAN_CH, power_0_100, PERCENT_COMPARE_FORMAT);
}

void SetLedPowerPower(uint8_t power_0_100)
{
  if (power_0_100 > 100)
  {
    power_0_100 = 100;
  }
  TIM_LED_WHITE->setCaptureCompare(LED_WHITE_CH, power_0_100, PERCENT_COMPARE_FORMAT);
}

void SetPhysicalOutputs()
{
  // write rx_buffer to my outputs
  if (ParseU8(I2C_SLAVE::e2s_buffer, E2S::RELAY_BLRESET_POS) && 0b1)
  {
    digitalWrite(PIN::RELAY, HIGH);
  }
  else
  {
    digitalWrite(PIN::RELAY, LOW);
  }

  if (ParseU8(I2C_SLAVE::e2s_buffer, E2S::RELAY_BLRESET_POS) && 0b10)
  {
    digitalWrite(PIN::BL_RESET, HIGH); //
  }
  else
  {
    digitalWrite(PIN::BL_RESET, LOW);
  }

  SetServoAngle(1, ParseU8(I2C_SLAVE::e2s_buffer, E2S::SERVO1_POS));
  SetServoAngle(2, ParseU8(I2C_SLAVE::e2s_buffer, E2S::SERVO2_POS));
  SetServoAngle(3, ParseU8(I2C_SLAVE::e2s_buffer, E2S::SERVO3_POS));
  SetFanSpeed(ParseU8(I2C_SLAVE::e2s_buffer, E2S::FAN0_POS));
  // TODO USBC
  // HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, ParseU16(esp2stm_buf, DAC1_POS));
  // HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, ParseU16(esp2stm_buf, DAC2_POS));
  // SetHeaterPower: loop1ms automatically fetches correct value from buffer
  SetLedPowerPower(ParseU8(I2C_SLAVE::e2s_buffer, E2S::LED_POWER_POS));
  // TODO Brushless Mode
  // TODO Brushless Speeds
}

void setActorsToSafeSetting()
{
  memset(I2C_SLAVE::e2s_buffer, 0, E2S::ESP2STM_SIZE);
  SetPhysicalOutputs();
}

uint8_t CollectBitInputs()
{
  return (digitalRead(PIN::BTN_RED) << 0) |
         ((!digitalRead(PIN::BTN_YELLOW)) << 1) |
         (digitalRead(PIN::MOVEMENT) << 2) |
         ((!digitalRead(PIN::BL_FAULT)) << 3);
}

void app_loop20ms(uint32_t now)
{
  led->Loop(now);

  //I2C_SLAVE::e2s_buffer[E2S::HEATER_POS] = encoder.GetTicks(); // COMMENT OUT FOR TESTING ONLY

  WriteU8(CollectBitInputs(), I2C_SLAVE::s2e_buffer, S2E::BTN_MOVEMENT_BLFAULT_POS);
  WriteU16(encoder.GetTicks(), I2C_SLAVE::s2e_buffer, S2E::ROTENC_POS);
  WriteU16(0, I2C_SLAVE::s2e_buffer, S2E::BRIGHTNESS_POS);
  WriteU16(PowerSink.activeVoltage, I2C_SLAVE::s2e_buffer, S2E::USBPD_VOLTAGE_IS_POS);
  WriteU16(analogRead(PIN::ADC_0), I2C_SLAVE::s2e_buffer, S2E::ADC0_POS);
  WriteU16(analogRead(PIN::ADC_1), I2C_SLAVE::s2e_buffer, S2E::ADC1_POS);

  if (now >= timeToPutActorsInFailsafe)
  {
    if (!noDataWarningAlreadyPrinted)
    {
      log_warn("No Data from ESP32 - goto failsafe!");
      noDataWarningAlreadyPrinted = true;
    }
    gotDataInfoAlreadyPrinted = false;
    setActorsToSafeSetting();
    SetBitIdx(I2C_SLAVE::s2e_buffer[S2E::STATUS_POS], 2);
    timeToPutActorsInFailsafe = INT64_MAX;
  }
  else
  {
    if (!gotDataInfoAlreadyPrinted)
    {
      log_info("Got Data from ESP32");
      gotDataInfoAlreadyPrinted = true;
    }
    noDataWarningAlreadyPrinted = false;
  }
}

void app_loop1000ms(uint32_t now)
{
  (void)now;
  log_info("stat=0x%02X, input_bits=0x%02X, enco=%d heater=%d", I2C_SLAVE::s2e_buffer[S2E::STATUS_POS], CollectBitInputs(), ParseU16(I2C_SLAVE::s2e_buffer, S2E::ROTENC_POS), I2C_SLAVE::e2s_buffer[E2S::HEATER_POS]);
}

void app_loop(uint32_t now)
{
  // Heater; Wert ist von 0-100; Zyklus dauert 1000ms
  static uint32_t startOfCycle = 0;
  time_t passedTime = now - startOfCycle;
  if (passedTime >= (10 * 100))
  {
    startOfCycle = now;
    if (I2C_SLAVE::e2s_buffer[E2S::HEATER_POS] > 0)
    {
      digitalWrite(PIN::HEATER, HIGH);
    }
    else
    {
      digitalWrite(PIN::HEATER, LOW);
    }
  }
  else if (passedTime >= 10 * I2C_SLAVE::e2s_buffer[E2S::HEATER_POS])
  {
    digitalWrite(PIN::HEATER, LOW);
  }
}

void setup()
{
#ifdef DEBUG
  delay(3000); // to give platformIO terminal enough time
#endif
  // Serial.println("Application started Serial.println");
  log_info("Application started");
  log_info("Init gpio");
  pinMode(PIN::BTN_RED, INPUT);
  pinMode(PIN::BTN_YELLOW, INPUT);
  pinMode(PIN::MOVEMENT, INPUT);
  pinMode(PIN::BL_FAULT, INPUT);
  pinMode(PIN::HEATER, OUTPUT);
  pinMode(PIN::ADC_0, INPUT_ANALOG);
  pinMode(PIN::ADC_1, INPUT_ANALOG);

  log_info("Init Rotary Encoder");
  if (encoder.Setup())
  {
    log_info("Encoder Initialization OK\n");
  }
  else
  {
    log_warn("Encoder Initialization Failed\n");
    while (1)
      ;
  }
  // rotary_encoder->bind((uint16_t *)(&I2C_SLAVE::s2e_buffer[S2E::ROTENC_POS]));

  log_info("Init Timer");
  // Problem: Die All-In-One-Funktion "setPWM" kann nur ganzzahlige Prozentangaben setzen, Dies ist zu Wenig für Servos
  // Lösung: Hier diese Funktion zum grundsätzlichen Initialisieren verwenden und oben die "setCaptureCompare"
  TIM_LED_WHITE = new HardwareTimer(TIM4);
  TIM_SERVO_0_1 = new HardwareTimer(TIM15);
  TIM_SERVO_2 = new HardwareTimer(TIM16);
  TIM_FAN = new HardwareTimer(TIM17);

  TIM_LED_WHITE->setPWM(LED_WHITE_CH, PB_9_ALT1, 1000, 0);
  TIM_SERVO_0_1->setPWM(SERVO0_CH, PA_2_ALT1, 50, 0);
  TIM_SERVO_0_1->setPWM(SERVO1_CH, PA_3_ALT1, 50, 0);
  TIM_SERVO_2->setPWM(SERVO2_CH, PA_6_ALT1, 50, 0);
  TIM_FAN->setPWM(FAN_CH, PA_7_ALT3, 1000, 0);

  log_info("Init Info LED");
  led = new SINGLE_LED::M(PIN::LED_INFO, true, &WAITING_FOR_CONNECTION);
  led->Begin(millis(), &WAITING_FOR_CONNECTION, 10000);

  log_info("Init USB PD");
  PowerSink.start(USB_PD::handleUsbPdEvent);

  log_info("Init I2C");
  memset(I2C_SLAVE::s2e_buffer, 0, S2E::STM2ESP_SIZE);
  setActorsToSafeSetting();
  Wire.setSDA(PIN::SDA);
  Wire.setSCL(PIN::SCL);
  Wire.onReceive(I2C_SLAVE::onI2CReceive);
  Wire.onRequest(I2C_SLAVE::onI2CRequest);
  Wire.begin(I2C_SETUP::STM32_I2C_ADDRESS);

  log_info("Init completed - eternal loop starts");
}

void loop_servotest()
{
  int pos;
  for (pos = 0; pos <= 180; pos += 1)
  { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    SetServoAngle(0, pos); // tell servo to go to position in variable 'pos'
    SetServoAngle(1, pos);
    SetServoAngle(2, pos);
    delay(15); // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1)
  {                        // goes from 180 degrees to 0 degrees
    SetServoAngle(0, pos); // tell servo to go to position in variable 'pos'
    SetServoAngle(1, pos);
    SetServoAngle(2, pos);
    delay(15); // waits 15ms for the servo to reach the position
  }
}

void loop()
{
  PowerSink.poll();
  static uint32_t last20ms = 0;
  static uint32_t last1000ms = 0;
  uint32_t now = millis();
  app_loop(now);
  uint32_t timePassed20 = now - last20ms;
  if (timePassed20 >= 20)
  {
    last20ms = now;
    app_loop20ms(now);
  }
  uint32_t timePassed1000 = now - last1000ms;
  if (timePassed1000 >= 1000)
  {
    last1000ms = now;
    app_loop1000ms(now);
  }
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