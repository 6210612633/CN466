#include <Arduino.h>
#include <Wifi.h>
#include <Update.h>
#include <wifiManager.h>
#include <time.h>
#include <stdlib.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>



WiFiManager wm;

const char* rootCaCer = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
"-----END CERTIFICATE-----\n";

int count = 0;
char str[10];
int no = 1;
int stage = 0;
int save_reactime;
Adafruit_NeoPixel pixels(1,18,NEO_GRB + NEO_KHZ800);
int random_int = random(10000)+1000;

void setClock(){
  configTime(0,0,"pool.ntp.org");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 *3600*2){
    delay(500);
    yield();
    nowSecs = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&nowSecs,&timeinfo);
  Serial.println(asctime(&timeinfo));
}

void connect_firebase(){
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client){
    client -> setCACert(rootCaCer);
    if(1){
      HTTPClient https;
      if(https.begin(*client, "https://cn466-demo-3e559-default-rtdb.firebaseio.com/iot_reactime.json")){
        https.addHeader("Content-Type","application/json");
        char body[500];
        sprintf(body,"{\"time(ms)\": %d}",count);
        int httpResponseCode = https.POST(body);
        Serial.println("http status -> " + httpResponseCode);
        https.end();
      }
    }
  }
}

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
  //wm.resetSettings();

  wm.autoConnect("wm_test","12345678");
  Serial.print(WiFi.RSSI());
  setClock();
  

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
        sprintf(str, "%d.%dms", count/1000,count%1000);
        connect_firebase();
        Serial.println((String)"time = " + str);
        Serial.println((String)"wifi.rssi = " + WiFi.RSSI());
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


