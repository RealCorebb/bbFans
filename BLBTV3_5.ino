#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include "LittleFS.h"
#define NUMstrip 42
#define Digit1 35
#define Digit2 28
#define Digit3 21
#define Digit4 14
#define Digit5 7
#define Digit6 0
#define PIN 13

// Level from 0-4
#define ASYNC_HTTP_DEBUG_PORT     Serial

#define _ASYNC_HTTP_LOGLEVEL_     1
#define _WIFIMGR_LOGLEVEL_        1

// 300s = 5 minutes to not flooding, 60s in testing
#define HTTP_REQUEST_INTERVAL     60  //300
#include <AsyncHTTPRequest_Generic.h>

//--------------------------//
#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>
#include <AsyncHTTPRequest_Generic.h> 
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(NUMstrip, PIN);
//Adafruit_NeoPixel strip(NUMstrip, PIN, NEO_GRB + NEO_KHZ800);
AsyncHTTPRequest request;



//GLOBAL------------------//
int changing=0;
int r=51;
int g=241;
int b=255;
int pixel[NUMstrip]={0};
int DisplayNum = 0;
uint32_t MODE_DIGIT=1;                //0 color picker          //1 rainbowloop             //2 RainbowSmooth
uint32_t MODE_LIGHTING=2;            //0 color picker                     //2 RainbowSmooth
int brightness=1;
int oldSubscriberCount=0;
int Anilooptime=0;
AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wm(&server,&dns);
//LED--------SETUP-----------//
int LEDR=25;
int LEDG=26;
int LEDB=27;
uint32_t R=255;  
uint32_t G=255; 
uint32_t B=255;     
uint8_t LEDcolorhue = 0;
boolean LED_PICK_CHANGE=false;
const boolean invert = false; // set true if common anode, false if common cathode
int LED_brightness = 127;

void sendRequest() 
{
  static bool requestOpenResult;
  
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
  {
    
    requestOpenResult = request.open("GET", "http://api.bilibili.com/x/relation/stat?vmid=121386396");
    
    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      Serial.println("GETTING...");
      request.send();
    }
    else
    {
      Serial.println("Can't send bad request");
    }
  }
  else
  {
    Serial.println("Can't send request");
  }
}

void requestCB(void* optParm, AsyncHTTPRequest* request, int readyState) 
{
  (void) optParm;
  
  if (readyState == readyStateDone) 
  {
    String payload = request->responseText();   //Get the request response payload
    Serial.println(payload);
      String subs=payload.substring(payload.indexOf("\"follower\":")+11,payload.indexOf("}}"));
      int subscriberCount=subs.toInt();
      //Serial.println(subscriberCount);
        if (subscriberCount!=oldSubscriberCount){
        changing=1;
        Serial.println(changing);
        }
      oldSubscriberCount=subscriberCount;
      DisplayNum=subscriberCount;
      Serial.println(DisplayNum);
   
        /*
        if  (DisplayNum>99999) DisplayNum=0;
             DisplayNum++;
             */
             if(DisplayNum<100){
                   int Display1 = (DisplayNum/10)%10;
                    int Display2 = (DisplayNum/1)%10;
                    DrawDigit(Digit3,r,g,b,Display1);
                     DrawDigit(Digit4,r,g,b,Display2);
              }
              else if(DisplayNum<1000){
                int Display1 = (DisplayNum/100)%10;
                  int Display2 = (DisplayNum/10)%10;
                    int Display3 = (DisplayNum/1)%10;
                               DrawDigit(Digit2,r,g,b,Display1);
                                DrawDigit(Digit3,r,g,b,Display2);
                                 DrawDigit(Digit4,r,g,b,Display3);
                }
                else if (DisplayNum<10000){
                 int Display1 = (DisplayNum/1000)%10;
                 int Display2 = (DisplayNum/100)%10;
                  int Display3 = (DisplayNum/10)%10;
                    int Display4 = (DisplayNum/1)%10;
                      DrawDigit(Digit2,r,g,b,Display1);
                              DrawDigit(Digit3,r,g,b,Display2);
                                 DrawDigit(Digit4,r,g,b,Display3);
                                    DrawDigitInvert(Digit5,r,g,b,Display4);
                  }
             else if(DisplayNum<100000){
               int Display1 = (DisplayNum/10000)%10;
                int Display2 = (DisplayNum/1000)%10;
                 int Display3 = (DisplayNum/100)%10;
                  int Display4 = (DisplayNum/10)%10;
                    int Display5 = (DisplayNum/1)%10;
                DrawDigit(Digit1,r,g,b,Display1);
                DrawDigit(Digit2,r,g,b,Display2);
                DrawDigit(Digit3,r,g,b,Display3);
                DrawDigit(Digit4,r,g,b,Display4);
                DrawDigitInvert(Digit5,r,g,b,Display5);
             }
              else if(DisplayNum<1000000){
               int Display1 = (DisplayNum/100000)%10;
                int Display2 = (DisplayNum/10000)%10;
                 int Display3 = (DisplayNum/1000)%10;
                  int Display4 = (DisplayNum/100)%10;
                    int Display5 = (DisplayNum/10)%10;
                     int Display6 = (DisplayNum/1)%10;
                DrawDigit(Digit1,r,g,b,Display1);
                DrawDigit(Digit2,r,g,b,Display2);
                DrawDigit(Digit3,r,g,b,Display3);
                DrawDigit(Digit4,r,g,b,Display4);
                DrawDigitInvert(Digit5,r,g,b,Display5);
                DrawDigit(Digit6,r,g,b,Display6);                
             } 
    request->setDebug(false);
  }
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  int looptime=0;
  // put your setup code here, to run once:
 // Serial.begin(115200);
  Serial.println("Gogogo!");
  //Load Config//
  File file = LittleFS.open("config.json", FILE_READ);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));
  else{
    MODE_DIGIT = doc["MODE_DIGIT"];
    brightness = doc["brightness"];
    r = doc["r"];
    g = doc["g"];
    b = doc["b"];
  }
  ///
  strip.Begin();
  strip.SetBrightness(brightness);
  request.onReadyStateChange(requestCB);
  LittleFS.begin();
 //STARUP ANIMATION//
  for(long firstPixelHue = 0; firstPixelHue < 360; firstPixelHue ++) {
    
    if (looptime==0){
        for(int i=0; i<NUMstrip; i++) { // For each pixel in strip...
         // Offset pixel hue by an amount to make one full revolution of the
          // color wheel (range of 65536) along the length of the strip
          // (strip.numPixels() steps):    
          int pixelHue = firstPixelHue + (i * 360L / NUMstrip);
          // RgbColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
          // optionally add saturation and value (brightness) (each 0 to 255).
         // Here we're using just the single-argument hue variant. The result
         // is passed through strip.gamma32() to provide 'truer' colors
         // before assigning to each pixel:
         strip.SetPixelColor(i, HslColor(pixelHue/360.0f,1.0f,0.5f));
         strip.Show(); // Update strip with new contents
         if(brightness<80){
            brightness+=1;
            strip.SetBrightness(brightness);
         }
       delay(10);  // Pause for a moment
    }
      //------------------------------WIFI---------------------------//
 WiFi.mode(WIFI_AP_STA);   
 WiFi.softAP("bbFans");
    Serial.begin(115200);
    if(wm.autoConnect("bbFans")){
        Serial.println("connected...yeey :)");
    }
    else {
        Serial.println("Configportal running");
    }
//-------------------------------------------------------------------------------//
    }
    else{
        for(int i=0; i<NUMstrip; i++) { 
            int pixelHue = firstPixelHue + (i * 360L / NUMstrip);
            strip.SetPixelColor(i, HslColor(pixelHue/360.0f,1.0f,0.5f));
          }
        }
       if (looptime!=0){
        if(brightness<80){
            brightness+=1;
            strip.SetBrightness(brightness);
         }
        strip.Show();
        delay(10);
       }
       looptime++;
    }
 //LED------------------------------//
 /*
   ledcAttachPin(LEDR, 1); // assign RGB led pins to channels
  ledcAttachPin(LEDG, 2);
  ledcAttachPin(LEDB, 3);
   ledcSetup(1, 5000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);   
  */
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.htm");
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(MODE_DIGIT).c_str());
  });
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(brightness).c_str());
  });
  server.on("/changeMode", HTTP_GET, [](AsyncWebServerRequest *request){
    String message;
        if (request->hasParam("mode")) {
            message = request->getParam("mode")->value();
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    MODE_DIGIT =  message.toInt();
    doc["MODE_DIGIT"] = MODE_DIGIT;
    writeConfig();
  });
  server.on("/changeBrightness", HTTP_GET, [](AsyncWebServerRequest *request){
    String message;
        if (request->hasParam("brightness")) {
            message = request->getParam("brightness")->value();
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    brightness =  message.toInt();
    strip.SetBrightness(brightness);
    doc["brightness"] = brightness;
    writeConfig();
  });
  server.on("/changeColor", HTTP_GET, [](AsyncWebServerRequest *request){
    r =  request->getParam("r")->value().toInt();
    g =  request->getParam("g")->value().toInt();
    b =  request->getParam("b")->value().toInt();
        request->send(200, "text/plain", "OK");
    doc["r"] = r;
    doc["g"] = g;
    doc["b"] = b;
    writeConfig();
  });
  server.onNotFound(notFound);
  server.begin();
  }

void writeConfig(){
  File file = LittleFS.open("config.json", FILE_WRITE);
    if(file){
      if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
      }
      file.close();
    }
}

void DrawDigitInvert(int offset, int r,int g,int b, int n)
{

  if (n == 2 || n == 3 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9) //MIDDLE
  {
    pixel[6+offset]=1;
   // strip.SetPixelColor(0 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[6+offset]=0;
   // strip.SetPixelColor(0 + offset, RgbColor(0, 0, 0));o
  }
  if (n == 0 || n == 1 || n == 2 || n == 3 || n == 4 || n == 7 || n == 8 || n == 9) //TOP RIGHT
  {
    pixel[3+offset]=1;
   // strip.SetPixelColor(6 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[3+offset]=0;
 //   strip.SetPixelColor(6 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) //TOP
  {
    pixel[4+offset]=1;
  //  strip.SetPixelColor(5 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[4+offset]=0;
 //   strip.SetPixelColor(5 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9) //TOP LEFT
  {
    pixel[5+offset]=1;
  //  strip.SetPixelColor(4 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[5+offset]=0;
 //   strip.SetPixelColor(4 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 6 || n == 8) //BOTTOM LEFT
  {
    pixel[0+offset]=1;
 //   strip.SetPixelColor(3 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[0+offset]=0;
   // strip.SetPixelColor(3 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 8 || n == 9) //BOTTOM
  {
    pixel[1+offset]=1;
    //strip.SetPixelColor(2 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[1+offset]=0;
   // strip.SetPixelColor(2 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 1 || n == 3 || n == 4 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) //BOTTOM RIGHT
  {
    pixel[2+offset]=1;
   // strip.SetPixelColor(1 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[2+offset]=0;
    //strip.SetPixelColor(1 + offset, RgbColor(0, 0, 0));
  }
 
}


void DrawDigit(int offset, int r,int g,int b, int n)
{

  if (n == 2 || n == 3 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9) //MIDDLE
  {
    pixel[0+offset]=1;
   // strip.SetPixelColor(0 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[0+offset]=0;
   // strip.SetPixelColor(0 + offset, RgbColor(0, 0, 0));o
  }
  if (n == 0 || n == 1 || n == 2 || n == 3 || n == 4 || n == 7 || n == 8 || n == 9) //TOP RIGHT
  {
    pixel[3+offset]=1;
   // strip.SetPixelColor(6 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[3+offset]=0;
 //   strip.SetPixelColor(6 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) //TOP
  {
    pixel[2+offset]=1;
  //  strip.SetPixelColor(5 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[2+offset]=0;
 //   strip.SetPixelColor(5 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9) //TOP LEFT
  {
    pixel[1+offset]=1;
  //  strip.SetPixelColor(4 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[1+offset]=0;
 //   strip.SetPixelColor(4 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 6 || n == 8) //BOTTOM LEFT
  {
    pixel[6+offset]=1;
 //   strip.SetPixelColor(3 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[6+offset]=0;
   // strip.SetPixelColor(3 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 8 || n == 9) //BOTTOM
  {
    pixel[5+offset]=1;
    //strip.SetPixelColor(2 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[5+offset]=0;
   // strip.SetPixelColor(2 + offset, RgbColor(0, 0, 0));
  }
  if (n == 0 || n == 1 || n == 3 || n == 4 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) //BOTTOM RIGHT
  {
    pixel[4+offset]=1;
   // strip.SetPixelColor(1 + offset, RgbColor(r, g, b));
  }
  else
  {
    pixel[4+offset]=0;
    //strip.SetPixelColor(1 + offset, RgbColor(0, 0, 0));
  }
 
}

//TIMING SETTINGS//
int period = 2000;
unsigned long time_now = 0;
int Rainbowperiod = 20;
unsigned long Rainbowtime_now = 0;
int Rainbowperiod_LED = 20;
unsigned long Rainbowtime_LED_now = 0;
int animation = 10;
unsigned long animation_now = 0;
//////////


long firstPixelHue = 0;

void loop() {
  if(millis() > time_now + period){
        time_now = millis();
        sendRequest();                                                                  //Send the request      
  }
           switch(MODE_DIGIT){
            case 0:
                if(changing==0){
                 for (int i=0;i<NUMstrip;i++){
                  if (pixel[i]==1){
                   strip.SetPixelColor(i, RgbColor(r, g, b));
                   }
                  else {
                    strip.SetPixelColor(i,RgbColor(0, 0, 0));
                 }
                 }
                }
                if(changing==1){
                        for (int i=0;i<2;i++){
                        int count=0;
                        int firstPixelHue=random(0,360);
                        int anii=NUMstrip;
                        if (Anilooptime==0){                
                           if(millis() > animation_now + animation){
                                animation_now=millis();
                               if(anii>0) anii--;
                               int pixelHue = firstPixelHue + (count * 360L / NUMstrip);
                                  if (pixel[i]==1){
                                        count++;
                                       strip.SetPixelColor(i, HslColor(pixelHue/360.0f,1.0f,0.5f));
                                  }
                                  else{
                                      strip.SetPixelColor(i, RgbColor(0,0,0));
                                  }
                                  strip.Show();
                                  firstPixelHue++;
                                  if(firstPixelHue>360) firstPixelHue==0;                             
                                 // delay(10);
                              }
                        }
                        else{
                          for(int i=NUMstrip; i>0; i--) { 
                            if (pixel[i]==1)
                                 strip.SetPixelColor(i, RgbColor(r,g,b));      
                            else
                                strip.SetPixelColor(i, RgbColor(0,0,0));   

                          strip.Show();
                          delay(10);
                          }
                          Anilooptime=0;
                          changing=0;
                          Serial.println(changing);
                          break;              
                        }
                              Anilooptime++;
                        }
                }
                                 break;
                 
            case 1:
                      if(millis() > Rainbowtime_now + Rainbowperiod){
                        int count=0;
                        Rainbowtime_now = millis();
                      if(firstPixelHue < 360){
                          firstPixelHue ++;                       
                           for(int i=0; i<NUMstrip; i++) { 
                               int pixelHue = firstPixelHue + (count * 360L / NUMstrip);
                                  if (pixel[i]==1){
                                        count++;
                                       strip.SetPixelColor(i, HslColor(pixelHue/360.0f,1.0f,0.5f));
                                  }
                                  else{
                                      strip.SetPixelColor(i, RgbColor(0,0,0));
                                  }
                              }
                            }     
                            else firstPixelHue=0;
                           }          
                 break;
             case 2:
                    if(millis() > Rainbowtime_LED_now + Rainbowperiod_LED){
                        Rainbowtime_LED_now = millis();
                        if(firstPixelHue < 360){
                          firstPixelHue ++;  
                    for(int i=0; i<NUMstrip; i++) { 
                               int pixelHue = firstPixelHue + (360L / NUMstrip);
                                  if (pixel[i]==1){
                                       strip.SetPixelColor(i, HslColor(pixelHue/360.0f,1.0f,0.5f));
                                  }
                                  else{
                                      strip.SetPixelColor(i, RgbColor(0,0,0));
                                  }
                              }           
                           }    
                           else firstPixelHue=0;
                    }   
                    break;                   
           }
   strip.Show();

}
