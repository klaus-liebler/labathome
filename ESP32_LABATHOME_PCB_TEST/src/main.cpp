//Einbindung der Deklarationen, damit die Funktionalität hier zur Verfügung steht
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

// Bedingter Code
//Bitte hier die richtige Board-Version einbinden
#include <hal.h>
#if hal == labathomev4
HAL hal(MODE_IO33::SERVO2, MODE_MULTI_PINS::UNDEFINED, MODE_SPEAKER::TRANSISTOR);
#elif hal == labathomev3
HAL hal(IO17_MODE::BUZZER, IO4_MODE::SPECIAL_SPECIAL_RELAY3);
#else
  #error "Define a hal in platformio.ini"
#endif

//Definition einer Preprozessor-Konstante (in C++ eigentlich nicht mehr notwendig, wird aber in der Praxis noch oft gemacht)
#define TFT_GREY 0x5AEB

//Deklaration von verschiedenen Variablen: Diese hier sind alle global und statisch (überall vorhanden und ändern ihre Größe nicht)
//Bemerkung: Speicherverwaltung ist ein riesen Thema in C++. 
//Ich rate Ihnen, da nicht zu tief einzusteigen und entweder globale statische oder lokale(kommt gleich) Variablen zu verwenden
TFT_eSPI tft = TFT_eSPI();                                                           
//TFT_ST7789 tft(HSPI_HOST, 2, 240, 240, DisplayRotation::ROT0, GPIO_NUM_MAX, GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_MAX, GPIO_NUM_5, GPIO_NUM_MAX, GPIO_NUM_0);
uint32_t targetTime = 0;
byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;
uint8_t testarray[100];

//Deklaration einer (kleinen dreckigen) Hilfsfunktion, die weiter unten ausprogrammiert ist.
static uint8_t conv2d(const char *p);
//Nutzung dieser Funktion                                               
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

void updateDisplay()
{
  
  if (targetTime < millis())
  {
    // Set next update for 1 second later
    targetTime = millis() + 1000;

    // Adjust the time values by adding 1 second
    ss++; // Advance second
    if (ss == 60)
    {           // Check for roll-over
      ss = 0;   // Reset seconds to zero
      omm = mm; // Save last minute time for display update
      mm++;     // Advance minute
      if (mm > 59)
      { // Check for roll-over
        mm = 0;
        hh++; // Advance hour
        if (hh > 23)
        {         // Check for 24hr roll-over (could roll-over on 13)
          hh = 0; // 0 for 24 hour clock, set to 1 for 12 hour clock
        }
      }
    }

    // Update digital time
    int xpos = 0;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;

    if (omm != mm)
    { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10)
        xpos += tft.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 8);  // Draw hours
      xcolon = xpos;                              // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos - 8, 8);
      if (mm < 10)
        xpos += tft.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 8);  // Draw minutes
      xsecs = xpos;                               // Sae seconds 'x' position for later display updates
    }
    if (oss != ss)
    { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;

      if (ss % 2)
      {                                             // Flash the colons on/off
        tft.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      }
      else
      {
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      //Draw seconds
      if (ss < 10)
        xpos += tft.drawChar('0', xpos, ysecs, 6); // Add leading zero
      tft.drawNumber(ss, xpos, ysecs, 6);          // Draw seconds
    }
  }
  
}

static uint8_t conv2d(const char *p)
{
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

//--------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  Serial.println("Board Test");
  hal.initBoard();
 

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  targetTime = millis() + 1000;
}

//--------------------------------------------------------------------------------------------------------------------------------
int angle_s1 = 0;
int angle_s2 = 0;
int duty = 0;

uint32_t lastSensorReadout = 0;

void loop()
{
  updateDisplay();
  hal.setLEDState(LED::LED_RED, hal.getButtonIsPressed(Switch::SW_RED) ? CRGB::Red : CRGB::Black);
  hal.setLEDState(LED::LED_YELLOW, hal.getButtonIsPressed(Switch::SW_YELLOW) ? CRGB::Yellow : CRGB::Black);
  hal.setLEDState(LED::LED_GREEN, hal.getButtonIsPressed(Switch::SW_GREEN) ? CRGB::Green : CRGB::Black);
  hal.setLEDState(LED::LED_3, hal.isMovementDetected() ? CRGB::White : CRGB::Black);
  hal.setRELAYState(hal.getButtonIsPressed(Switch::SW_GREEN));
  if (hal.getButtonIsPressed(Switch::SW_YELLOW))
  {
    hal.startBuzzer(440 + 2.0 * angle_s1);
  }
  else
  {
    hal.endBuzzer();
  }

  //Servo
  hal.setServo(Servo::Servo1, angle_s1 < 180 ? angle_s1 : 360 - angle_s1);
  hal.setServo(Servo::Servo2, angle_s2 < 180 ? angle_s2 : 360 - angle_s2);
  angle_s1 += 5;
  angle_s2 += 10;
  angle_s1 = angle_s1 > 360 ? 0 : angle_s1;
  angle_s2 = angle_s2 > 360 ? 0 : angle_s2;

  //LED POWER WHITE
  if (hal.getButtonIsPressed(Switch::SW_RED))
  {
    hal.setLED_POWER_WHITE_DUTY(duty < 100 ? duty : 200 - duty);
  }
  else
  {
    hal.setLED_POWER_WHITE_DUTY(0);
  }
  duty++;
  duty = duty > 200 ? 0 : duty;

  //Sensors
  uint32_t now = millis();
  if (now - lastSensorReadout > 5000)
  {
    Serial.print("Humidity: ");
    Serial.print(hal.bme280.readFloatHumidity(), 0);

    Serial.print(" Pressure: ");
    Serial.print(hal.bme280.readFloatPressure(), 0);

    Serial.print(" Alt: ");
    //Serial.print(mySensor.readFloatAltitudeMeters(), 1);
    Serial.print(hal.bme280.readFloatAltitudeFeet(), 1);

    Serial.print(" Temp: ");
    //Serial.print(mySensor.readTempC(), 2);
    Serial.print(hal.bme280.readTempC(), 2);
    Serial.println();
    float lux = hal.bh1750.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    lastSensorReadout = now;
  }

  delay(50);
}