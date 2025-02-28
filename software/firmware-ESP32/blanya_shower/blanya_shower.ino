#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Sounds.h> 
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include "ui.h"

/* Other used Ports 
Check screen ports .\libraries\TFT_eSPI\User_Setups\Setup46_GC9A01_ESP32.h
*/

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int MY_DISP_HOR_RES = 240;

/*Screen resolution*/
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

#define PINA 12
#define PINB 14

#define ReadBatteryPower 36
#define ReadMainPower 39
#define ReadPressureTop 34
#define ReadPressureBottom 35
#define ReadMainButton 32
#define ReadTemperature 33  

#define PWMSound 22
#define TransOnSolenoidMotor 21
#define ShutdownESP32 25  
#define TransOnHeatElement 26
#define NeopixelOut 27        


#define NUMPIXELS 8

float DesiredTemperature;
bool PendingToShutdown = false;
bool Battery = false;
bool MainPower = false;
double TemperatureCelsius = 0;
bool Play = false;
String message = "";
bool ErrorReadingTemperature = false;
double rMainPower = 0;

bool unique = true;
int BatteryReadings [6] = {0,0,0,0,0,0};
int readIndex = 0;
int totalReadingsBattery = 0;
double averageBatteryReading = 0;

int pressure2Readings [6] = {0,0,0,0,0,0};
int totalReadingPressure2 = 0;
double averagePressure2 = 0;

int mainPowerReadings [6] = {0,0,0,0,0,0};
int readIndexMainPower = 0;
int totalReadingsMainPower = 0;
double averageMainPowerReading = 0;


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ReadTemperature);
DallasTemperature temperatureSensor(&oneWire);

TFT_eSPI tft = TFT_eSPI();

Sounds UserSounds = Sounds(PWMSound);

Adafruit_NeoPixel pixels(NUMPIXELS, NeopixelOut, NEO_GRB + NEO_KHZ800);

// Create an instance of the BluetoothSerial class
BluetoothSerial SerialBT;
Preferences preferences;

static lv_obj_t * ta;

void loop2(void * pvParameters);

void ShowerPlanner();

void PixelsLight(int arr[3])
{
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.fill(pixels.Color(arr[0],arr[1],arr[2]));
  pixels.setBrightness(105);
  pixels.show(); // Send the updated pixel colors to the hardware.

}

void PixelsWelcome()
{

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  
  pixels.clear(); // Set all pixel colors to 'off'

  pixels.setPixelColor(0, pixels.Color(255, 182, 193)); // Rosa claro
  pixels.setPixelColor(1, pixels.Color(255, 182, 193)); // Rosa claro
  pixels.setPixelColor(2, pixels.Color(255, 182, 193)); // Rosa claro
  
  pixels.setPixelColor(3, pixels.Color(173, 216, 230)); // Azul claro
  pixels.setPixelColor(4, pixels.Color(173, 216, 230)); // Azul claro
  
  pixels.setPixelColor(5, pixels.Color(240, 230, 140)); // Amarillo pálido
  pixels.setPixelColor(6, pixels.Color(240, 230, 140)); // Amarillo pálido
  pixels.setPixelColor(7, pixels.Color(240, 230, 140)); // Amarillo pálido

  pixels.show(); // Send the updated pixel colors to the hardware.

}

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}


void setup()
{

  Serial.begin(115200);
  
  pinMode(TransOnHeatElement, OUTPUT);
  pinMode(ShutdownESP32, OUTPUT);
  pinMode(TransOnSolenoidMotor, OUTPUT);
  
  analogWrite(PINA, 0);
  analogWrite(PINB, 0);
      
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_LIGHTGREY);
  tft.textbgcolor = TFT_LIGHTGREY;
  tft.textcolor = TFT_BLACK;

  tft.drawString("blanya", 82, 100, 4);

  lv_init();

  delay(1000);

  /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

    /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  ui_init();

  temperatureSensor.begin();

  pixels.begin();
  PixelsWelcome();

  preferences.begin("blanya-settings", false);
  DesiredTemperature = preferences.getFloat("max_temp",30.00);
  preferences.end();

  xTaskCreatePinnedToCore (
    loop2,     // Function to implement the task
    "loop2",   // Name of the task
    12500,      // Stack size in bytes
    NULL,      // Task input parameter
    0,         // Priority of the task
    NULL,      // Task handle.
    0          // Core where the task should run
  );

  SerialBT.begin("blanya-shower");
  
  UserSounds.Sucess();

}

void airPumpIn(int value){
  analogWrite(PINA, value); 
}


void loop()
{ 
  
 if (SerialBT.available()){  
  
    char incomingChar = SerialBT.read();
  
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{ 
      if (message.length() > 0 && message[0] == 't'){

          float tempFromUser = message.substring(2,4).toFloat();
          
          if(tempFromUser > 45.0 || tempFromUser == 0.0){
            UserSounds.Error();
            SerialBT.println("Unsafe Temperature"); 
          }
          else {
            preferences.begin("blanya-settings", false);
            preferences.putFloat("max_temp", tempFromUser);
            DesiredTemperature = preferences.getFloat("max_temp");
            preferences.end();
            SerialBT.println(DesiredTemperature); 
            UserSounds.Sucess();
            }
          }
          
          message = "";
      } 
    }

    temperatureSensor.requestTemperatures();
    if (TemperatureCelsius != DEVICE_DISCONNECTED_C)
    {
      TemperatureCelsius = temperatureSensor.getTempCByIndex(0);
      ErrorReadingTemperature = false; 
    }
    else {
      ErrorReadingTemperature = true; 
    }
  
    rMainPower = analogRead(ReadMainPower);
    totalReadingsMainPower -= mainPowerReadings[readIndexMainPower];
    mainPowerReadings[readIndexMainPower] = rMainPower;
    totalReadingsMainPower += rMainPower;
    averageMainPowerReading = totalReadingsMainPower / 6;  
    readIndexMainPower = (readIndexMainPower +1) % 6;
    
    if (averageMainPowerReading > 800)
    {
      MainPower = true;
      Battery = false;
      PendingToShutdown = true;
    }
    else
    {
      MainPower = false;
      Battery = true;
    }
}

void loop2(void * pvParameters)
{
  for( ;; )
  {

    lv_timer_handler();
    
    if(analogRead(ReadMainButton) > 1000) {
    UserSounds.Sucess();
      if(unique) {
        int colors [3] = {220,20,60};
        PixelsLight(colors); 
        if(Play) {
          Play = false;
        } else {
          Play = true;
        }
        delay(800);
        if(analogRead(ReadMainButton) > 1000){
          delay(1000);
          if(analogRead(ReadMainButton) > 1000){
            PendingToShutdown = true;
          }
        }
      }
      unique = false;
    }
    else {
      ShowerPlanner();
      unique = true;
    }

    int batteryLevel = analogRead(ReadBatteryPower);
    double pTop = analogRead(ReadPressureTop);
    double pBottom = analogRead(ReadPressureBottom);


    totalReadingsBattery -= BatteryReadings[readIndex];
    BatteryReadings[readIndex] = batteryLevel;
    totalReadingsBattery += batteryLevel;

    averageBatteryReading = totalReadingsBattery / 6;  
    double batteryPercentage = ((averageBatteryReading - 2200) * 100) / (2750 - 2200); 

    totalReadingPressure2 -= pressure2Readings[readIndex];
    pressure2Readings[readIndex] = pBottom;
    totalReadingPressure2 += pBottom;
    averagePressure2 = totalReadingPressure2 / 6;  

    readIndex = (readIndex +1) % 6;

    lv_label_set_text(ui_NumTemperature, String(TemperatureCelsius, 1).c_str()); 
    lv_label_set_text(ui_NumUserTemp, String(DesiredTemperature, 1).c_str());
    lv_label_set_text(ui_NumPressureTop, String(pTop, 0).c_str());
    lv_label_set_text(ui_NumPressureBottom, String(averagePressure2, 0).c_str());
    lv_label_set_text(ui_NumBattery, String(batteryPercentage, 0).c_str());

    vTaskDelay(5);

  }
}

void ShowerPlanner()
{

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 200) return;
    lastUpdate = millis();
  
    // Non stop execution time

    lv_label_set_text(ui_ButtonStart, Play ? "Pause" : "Play");

    if(MainPower){

     lv_obj_add_flag(ui_NumBattery, LV_OBJ_FLAG_HIDDEN);

     digitalWrite(TransOnSolenoidMotor, LOW); // To avoid pressure accumulation
     int colors [3] = {255,69,0};
     PixelsLight(colors);  
          
     if (TemperatureCelsius > DesiredTemperature)
        {
          digitalWrite(TransOnHeatElement, LOW); // Shutdown relay
          UserSounds.Temperature();  // Alert the user that the water is ready     
          Play = false;
        }

      if(Play && averagePressure2 > 10){
          digitalWrite(TransOnHeatElement, HIGH); // Start relay, heat water.
          airPumpIn(220);
      }
      else {
          digitalWrite(TransOnHeatElement, LOW); // Stop relay.
          airPumpIn(0);
        }
        
    }

    
    if(Battery){
     digitalWrite(TransOnHeatElement, LOW); // Shutdown relay
     digitalWrite(TransOnSolenoidMotor, HIGH); // Pressure accumulation
     int colors [3] = {255,255,255};
     PixelsLight(colors);  
     
      if(Play){
         airPumpIn(255);
         lv_obj_add_flag(ui_NumBattery, LV_OBJ_FLAG_HIDDEN);
      }
      else {
         airPumpIn(0);
         lv_obj_clear_flag(ui_NumBattery, LV_OBJ_FLAG_HIDDEN);
        }
    }


    if(PendingToShutdown && Battery) {
      airPumpIn(0);
      UserSounds.Shutdown();
      digitalWrite(ShutdownESP32, HIGH); //To shutdown all
      delay(10000);
    }


/*
    if(ErrorReadingTemperature == true){
      UserSounds.Shutdown();
      digitalWrite(ShutdownESP32, HIGH); //To shutdown all
      Play = false; 
    }
*/

}
