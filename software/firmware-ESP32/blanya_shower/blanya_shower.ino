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
#include "cert.h"
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

bool unique = true;

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
      if (message[0] == 't'){

          String strNew = message.substring(2,4);
          float tempFromUser = strNew.toFloat();
          
          if(tempFromUser > 45.0 || tempFromUser == 0.0){
            UserSounds.Error();
            SerialBT.println("Unsafe Temperature"); 
          }
          else {
            preferences.begin("blanya-settings", false);
            preferences.putFloat("max_temp", tempFromUser);
            DesiredTemperature = preferences.getFloat("max_temp");
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
  
  if (analogRead(ReadMainPower) > 1000)
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

    lv_label_set_text(ui_temperatureNumber, String(TemperatureCelsius, 0).c_str());
    lv_arc_set_value(ui_uiTemperatureGauge,TemperatureCelsius);

    vTaskDelay(10);

  }
}

void ShowerPlanner()
{
    // Non stop execution time

    if(Play){
      lv_label_set_text(ui_Label4, "Pause");
    } else {
      lv_label_set_text(ui_Label4, "Play");
    }

    if(MainPower){
      
     digitalWrite(TransOnSolenoidMotor, LOW); // To avoid pressure accumulation
     int colors [3] = {255,69,0};
     PixelsLight(colors);  
          
     if (TemperatureCelsius > DesiredTemperature)
        {
          digitalWrite(TransOnHeatElement, LOW); // Shutdown relay
          UserSounds.Temperature();  // Alert the user that the water is ready     
          Play = false;
        }

      if(Play){
          digitalWrite(TransOnHeatElement, HIGH); // Start relay, heat water.
          airPumpIn(220);
      }
      else {
          digitalWrite(TransOnHeatElement, LOW); // Stop relay.
          airPumpIn(0);
        }
        
    }

    
    if(Battery){
     digitalWrite(TransOnSolenoidMotor, HIGH); // Pressure accumulation
     int colors [3] = {255,255,255};
     PixelsLight(colors);  
     
      if(Play){
         airPumpIn(255);
      }
      else {
         airPumpIn(0);
        }
    }


    if(PendingToShutdown && Battery) {
      airPumpIn(0);
      UserSounds.Shutdown();
      digitalWrite(ShutdownESP32, HIGH); //To shutdown all
      delay(10000);
    }
    
    delay(200);

}