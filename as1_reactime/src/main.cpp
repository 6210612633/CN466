#include <Arduino.h>
#include <Wifi.h>
#include <Update.h>
#include <wifiManager.h>
#include <time.h>
#include <stdlib.h>
#include <Adafruit_NeoPixel.h>


WiFiManager wm;
int count = 0;
int stage = 0;
int save_reactime;
Adafruit_NeoPixel pixels(1,18,NEO_GRB + NEO_KHZ800);
int random_int = random(10000)+1000;

void setup() {
  // set led
  pinMode(0, INPUT_PULLUP);
  pixels.begin();
  pixels.setBrightness(60);

  // set serial and print test
  Serial.begin(115200);
  Serial.println("start!!"); 
  Serial.print("waiting for green...  : ");
  Serial.println(random_int);

  // set wifimanager
  
  WiFi.mode(WIFI_AP);
  wm.resetSettings();
  wm.autoConnect("wm_test","12345678");
  Serial.print(WiFi.RSSI());

}

void loop() {
  pixels.clear();

  if(stage == 1){
      if(digitalRead(0) == HIGH){
        pixels.setPixelColor(0,pixels.Color(0,255,0));
      }
      else{
        Serial.println("Stop!!");
        //Serial.println(count);
        Serial.println((String)"time = " + count/1000 + (String)"." + count%1000 + "ms.");
        Serial.print((String)"wifi.rssi = " + WiFi.RSSI());
        stage = 2;
      }
  }

  else if(stage == 0 && count >= random_int){
    stage = 1;
    count = 0;
    pixels.setPixelColor(0,pixels.Color(0,255,0));
    Serial.println("\ngreen!");
  }

  else if(stage == 0){
    pixels.setPixelColor(0,pixels.Color(255,0,0));
  }

  else if(stage == 2)
    pixels.setPixelColor(0,pixels.Color(0,0,255));

  pixels.show();
  delay(1);
  count++;
}