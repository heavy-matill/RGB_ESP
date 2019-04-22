/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

// Wifi
#include <ESP8266WiFi.h>
#define wifi_ssid "istdochegal"
#define wifi_password "123sagichnicht"

//#define wifi_ssid "Wehlahm"
//#define wifi_password "Christel44367"
//#define wifi_ssid "AndroidAP"
//#define wifi_ssid "LaptopAP"
//#define wifi_password "asdasdasd"
WiFiClient wifiClient;

// MQTT
//#include <PubSubClient.h>
#include <MQTT.h>
#define mqtt_server "192.168.178.70"
//#define mqtt_server "192.168.137.1"
//#define mqtt_server "192.168.1.28"

#define mqtt_user "" + WiFi.macAddress()
#define mqtt_password "your_password"
//PubSubClient mqttClient(wifiClient);

MQTTClient mqttClient;
// RGBAnimator
#include <RGBAnimator.hpp>
#define ADDRESSABLE
//#define PWM
#define RELAY
#define OTA
//#define DEBUGGING
#define STATUS

#define DEV_NAME "Leinwand"
#define DEV_POS 0

#ifdef ADDRESSABLE
  #include <FastLED.h>
  // GPIO5 ESP12-E
  #define PIN_DATA 1
  
  // 30LED/M
  #define LED_TYPE    WS2811
  #define COLOR_ORDER BRG

  #define NUM_LEDS    2
  #define BRIGHTNESS  255
  CRGB leds[NUM_LEDS];
#endif
#ifdef PWM
// #include <SetPwmFrequency.h>
  #define PIN_R D8
  #define PIN_G D6
  #define PIN_B D7
  #define PIN_W 6
#endif
#ifdef RELAY
  #define PIN_RLY1 12
  #define PIN_RLY2 13
  #define ST_RLY1 digitalRead(PIN_RLY1);
  #define ST_RLY2 digitalRead(PIN_RLY2);

  #define RELAY_OFF_DELAY 5000
  time_t time_off = millis(); 
#endif
#ifdef OTA
  #include <ESP8266WebServer.h>
  //#include <ESP8266mDNS.h>
  #include <ESP8266HTTPUpdateServer.h>
  const char* host = "esp8266-webupdate";
  ESP8266WebServer httpServer(80);
  ESP8266HTTPUpdateServer httpUpdater;
#endif
uint32_t last, now;
RGBAnimator animor;
bool first_byte = true;
byte outByte = 0;
color_t color_current;

#ifdef STATUS  
  #include <TaskScheduler.h>
  void print_esp_status();
  Task task_esp_status(10000, TASK_FOREVER, &print_esp_status);
  Scheduler runner;
#endif

void setup() {
  #ifdef RELAY
    pinMode(PIN_RLY1, OUTPUT);
    pinMode(PIN_RLY2, OUTPUT);
    delay(500);   

    digitalWrite(PIN_RLY1, 1);
    digitalWrite(PIN_RLY2, 0);
    delay(500);   
    digitalWrite(PIN_RLY1, 0);
    digitalWrite(PIN_RLY2, 1);
    delay(500);   
    digitalWrite(PIN_RLY1, 0);
    digitalWrite(PIN_RLY2, 0);
  #endif
  #ifdef ADDRESSABLE
    FastLED.addLeds<LED_TYPE, PIN_DATA, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
  #endif
  #ifdef PWM
    //setPwmFrequency(5,32);
    //setPwmFrequency(6,32); //default 2kHz //beware this changes millis()
    //setPwmFrequency(9,8); //4kHz for atmega328

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(PIN_R, OUTPUT);
    pinMode(PIN_G, OUTPUT);
    pinMode(PIN_B, OUTPUT);
  #endif
  paint_col(color_t(255,0,0));
  delay(500);
  
  // Serial
  Serial.begin(115200);

  // Wifi 
  setup_wifi();
  
  #ifdef OTA
    setup_ota();
  #endif
  // Wifi connected == green
  paint_col(color_t(0,255,0));
  delay(500);
  // MQTT
  //mqttClient.setServer(mqtt_server, 1883);
  mqttClient.begin(mqtt_server, wifiClient);
  #ifdef STATUS
      runner.addTask(task_esp_status);
      task_esp_status.enable();
  #endif
  // MQTT connected == blue
  paint_col(color_t(0,0,255));
  delay(500);
  paint_col(color_t(0,0,0));

  animor = RGBAnimator();
  //animor.add_fade(color_t(0,255,0), color_t(255,0,0),2000,1000,3,true);
  animor.add_flash(color_t(255,0,0), color_t(0,0,0),100,100,1,false);
  animor.add_flash(color_t(0,255,0), color_t(0,0,0),100,100,1,false);
  animor.add_flash(color_t(0,0,255), color_t(0,0,0),100,100,1,false);
    
  animor.start();
  animor.animate(1);

  last = millis();
}
void paint_col(color_t col)
{  

  #ifdef ADDRESSABLE
    fill_solid(leds, NUM_LEDS, CRGB(col.R, col.G, col.B));
    FastLED.show();
  #endif
  #ifdef PWM
    analogWrite(PIN_R, col.R);  
    analogWrite(PIN_G, col.G);  
    analogWrite(PIN_B, col.B); 
  #endif 
}

// the loop function runs over and over again forever
void loop() {
  //Serial.print(".");
  // MQTT
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();  
  #ifdef STATUS
    runner.execute();
  #endif
  #ifdef OTA
    httpServer.handleClient();
  #endif
  #ifdef RELAY
    switch_relay_for_animation();
  #endif
  // RGBAnimator
  now = millis();
  animor.animate(now-last);
  last = now;  
  paint_col(animor.get_color_current());   
  
  //delay(5);

  // Serial
  //serialEvent();
}

void serialEvent() {  
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    byte inByte = (byte)inChar;
    
    /*uint8_t inByte = (uint8_t)Serial.read();
    //Serial.write(inByte);
    animor.process_data(inByte);
    //Serial.write(inByte);    */
  }
}

void process_byte(byte inByte){
  if (inByte >= 0x30 & inByte <= 0x39)
    {
      inByte -= 0x30;
    }
    else
    {
      if (inByte >= 0x41 & inByte <= 0x46)
      {
        inByte -= 0x37;
      }
      else
      {
        inByte = 0; // error character not 0-9,A-F
      }
    }
    if(first_byte)
    {
      outByte = 16*inByte;
    }
    else
    {
      outByte += inByte;
      animor.process_data(outByte);
      Serial.print(outByte,HEX);
    }
    first_byte = !first_byte;
}

void callback(MQTTClient *mqttClient, char* topic, char* payload, int length) {
  /*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  for (int i = 0; i < length; i++) {
    Serial.print(payload[i], HEX);
  }
  Serial.println();*/

  if (strcmp(topic,"rgb") == 0) {
    // RGB commands
    for (int i = 0; i < length/2; i++) {
      //Serial.println("rgb");
      //Serial.print((char)payload[i]);  
      uint8_t outByte;
      for (int j = 0; j<2; j++) {
        uint8_t inByte = payload[i*2+j];
        if (inByte >= 0x30 & inByte <= 0x39) {
          inByte -= 0x30;
        } else {
          if (inByte >= 0x41 & inByte <= 0x46) {
            inByte -= 0x37;
          } else {
            inByte = 0; // error character not 0-9,A-F
          }
        }
        if (j==0) {
          outByte = 16*inByte;
        } else {            
          outByte += inByte;
          //Serial.print(outByte, HEX);
          animor.process_data(outByte);
        }
      }
    }
  #ifdef RELAY
  } else if (strcmp(topic,"relay") == 0) {
    // Relay commands
    switch (payload[0]) {
      case 0x31: // "1"
        digitalWrite(PIN_RLY1, 1);
        digitalWrite(PIN_RLY2, 1);
        break;
      default:
        digitalWrite(PIN_RLY1, 0);
        digitalWrite(PIN_RLY2, 0);
    }
  } else if (strcmp(topic,"relay1") == 0) {
    switch (payload[0]) {
      case 0x31: // "1"
        digitalWrite(PIN_RLY1, 1);
        break;
      default:
        digitalWrite(PIN_RLY1, 0);
    }
  } else if (strcmp(topic,"relay2") == 0) {
    switch (payload[0]) {
      case 0x31: // "1"
        digitalWrite(PIN_RLY2, 1);
        break;
      default:
        digitalWrite(PIN_RLY2, 0);
    }
  #endif
  }
}

void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  
  WiFi.mode(WIFI_STA);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    delay(500);
}

#ifdef OTA
void setup_ota() {
  //MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  //MDNS.addService("http", "tcp", 80);
  Serial.print("HTTPUpdateServer ready! Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/update in your browser");
}
#endif

#ifdef RELAY
void switch_relay_for_animation() {
  if (animor.b_running) {
    digitalWrite(PIN_RLY2, 1);
    time_off = millis() + RELAY_OFF_DELAY;
  } else {
    if (time_off < millis()) {      
      digitalWrite(PIN_RLY2, 0);
    }
  }
}
#endif

void reconnect() 
{
  // Loop until we're reconnected
  while (!mqttClient.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (mqttClient.connect("ESP8266Client")) {
    if (mqttClient.connect("ESP8266Client"))//, mqtt_user, mqtt_password)) 
    {
      Serial.println("connected");
      mqttClient.onMessageAdvanced(callback);
      //mqttClient.setCallback(callback);
      Serial.println("setCallback");
      mqttClient.subscribe("rgb");
      #ifdef RELAY        
        mqttClient.subscribe("relay");
        mqttClient.subscribe("relay1");
        mqttClient.subscribe("relay2");
      #endif
      Serial.println("subscribed");
      String msg = "{name: '" + String(DEV_NAME) + "', ip: '" + WiFi.localIP().toString() + "', mac: '" + WiFi.macAddress() + "', pos: " + DEV_POS + "}";
      mqttClient.publish("esp", msg);
      
      mqttClient.publish("esp", "free_heap: " + String(ESP.getFreeHeap()));
      mqttClient.publish("esp", ESP.getResetReason());
      //mqttClient.publish("esp", ESP.getResetInfo());
      
      Serial.println("published");
    }
    else 
    {
      Serial.print("failed, rc=");
      //Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for (uint8_t i = 0; i <= 250; i++) {
        #ifdef OTA
          httpServer.handleClient();
        #endif
        delay(20);
      }
    }
  }
}

#ifdef STATUS
  void print_esp_status() {    
    mqttClient.publish("esp/" + WiFi.macAddress(), "free_heap: " + String(ESP.getFreeHeap()));
    mqttClient.publish("esp/"+ WiFi.macAddress() , ESP.getResetReason());
    //mqttClient.publish("esp/"+ WiFi.macAddress() , ESP.getResetInfo());
  }
#endif

