#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
WiFiManager wm;
#define NUMstrip 42
#define Digit1 35
#define Digit2 28
#define Digit3 21
#define Digit4 14
#define Digit5 7
#define Digit6 0
#define PIN 13
//--------------------------//
//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> strip(NUMstrip, PIN);
//GLOBAL------------------//
int r=255;
int g=255;
int b=255;
int pixel[NUMstrip]={0};
int DisplayNum = 0;
uint32_t MODE_DIGIT=1;                //0 color picker          //1 rainbowloop             //2 RainbowSmooth
uint32_t MODE_LIGHTING=2;            //0 color picker                     //2 RainbowSmooth
int brightness=1;
int oldSubscriberCount=0;
int Anilooptime=0;
AsyncWebServer server(80);
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

void setup() {
  int looptime=0;
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Gogogo!");
  strip.Begin();
  strip.SetBrightness(brightness);
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
 WiFi.mode(WIFI_STA);   

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(MODE_DIGIT).c_str());
  });
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(brightness).c_str());
  });
  server.on("/mode", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
        if (request->hasParam("mode", true)) {
            message = request->getParam("mode", true)->value();
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    MODE_DIGIT =  message.toInt();
  });
  server.on("/brightness", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
        if (request->hasParam("brightness", true)) {
            message = request->getParam("brightness", true)->value();
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    brightness =  message.toInt();
  });
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
int period = 1000;
unsigned long time_now = 0;
int Rainbowperiod = 20;
unsigned long Rainbowtime_now = 0;
int Rainbowperiod_LED = 20;
unsigned long Rainbowtime_LED_now = 0;
int animation = 10;
unsigned long animation_now = 0;
//////////


long firstPixelHue = 0;
int changing=0;
void loop() {
  if(millis() > time_now + period){
        time_now = millis();

        HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://api.bilibili.com/x/relation/stat?vmid=121386396");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
 
    if (httpCode == 200) { //Check the returning code
 
      String payload = http.getString();   //Get the request response payload
      String subs=payload.substring(payload.indexOf("\"follower\":")+11,payload.indexOf("}}"));
      int subscriberCount=subs.toInt();
      //Serial.println(subscriberCount);
        if (subscriberCount!=oldSubscriberCount){
        changing=1;
        Serial.println(changing);
        }
      oldSubscriberCount=subscriberCount;
      http.end();  
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
       }       
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
   /*
           switch(MODE_LIGHTING){
              case 0:
                     if(LED_PICK_CHANGE){
                     ledcWrite(1, R); // write red component to channel 1, etc.
                      ledcWrite(2, G);   
                        ledcWrite(3, B); 
                        Serial.printf("Just changed LED Color");
                        LED_PICK_CHANGE=false;
                        
                     }
                        break;
              case 2:
                     if(millis() > Rainbowtime_LED_now + Rainbowperiod_LED){
                      Rainbowtime_LED_now = millis();
                      hueToRGB(LEDcolorhue, LED_brightness);
                      ledcWrite(1, R); // write red component to channel 1, etc.
                      ledcWrite(2, G);   
                        ledcWrite(3, B); 
                        LEDcolorhue++;
                      if (LEDcolorhue>255){
                        LEDcolorhue=0;
                      }
                     }
                     break;
                    } 
                    */
}
void hueToRGB(uint8_t hue, uint8_t brightness)
{
    uint16_t scaledHue = (hue * 6);
    uint8_t segment = scaledHue / 256; // segment 0 to 5 around the
                                            // color wheel
    uint16_t segmentOffset =
      scaledHue - (segment * 256); // position within the segment

    uint8_t complement = 0;
    uint16_t prev = (brightness * ( 255 -  segmentOffset)) / 256;
    uint16_t next = (brightness *  segmentOffset) / 256;

    if(invert)
    {
      brightness = 255 - brightness;
      complement = 255;
      prev = 255 - prev;
      next = 255 - next;
    }

    switch(segment ) {
    case 0:      // red
        R = brightness;
        G = next;
        B = complement;
    break;
    case 1:     // yellow
        R = prev;
        G = brightness;
        B = complement;
    break;
    case 2:     // green
        R = complement;
        G = brightness;
        B = next;
    break;
    case 3:    // cyan
        R = complement;
        G = prev;
        B = brightness;
    break;
    case 4:    // blue
        R = next;
        G = complement;
        B = brightness;
    break;
   case 5:      // magenta
    default:
        R = brightness;
        G = complement;
        B = prev;
    break;
    }
}
