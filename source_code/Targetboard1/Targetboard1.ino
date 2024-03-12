#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <IRrecv.h>
#include <string>


#define RED_GPIO       42
#define YELLOW_GPIO    41
#define GREEN_GPIO     40
#define TOPIC_HIT    TOPIC_PREFIX "/hit"
#define TOPIC_TARGET1  TOPIC_PREFIX "/target1"
#define TOPIC_RANDOMNUM1    TOPIC_PREFIX "/randomnum1"
#define TOPIC_TARGETSTATE    TOPIC_PREFIX "/targetstate"


IRrecv irrecv(16);
decode_results results;
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);


enum game_state_t{
  wait,
  ready_to_start,
  ingame,
  g_off,
  game_end
};

enum led_state_t{
  ledon,
  ledoff
};

game_state_t game_state;
led_state_t ledstate;
uint32_t start_time, timestamp, now;
int randomNumber, x;



void connect_wifi() {
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s.\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    printf(".");
    fflush(stdout);
    delay(500);
  }
  printf("\nWiFi connected.\n");
}


void connect_mqtt() {
  printf("Connecting to MQTT broker at %s.\n", MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS)) {
    printf("Failed to connect to MQTT broker.\n");
    digitalWrite(RED_GPIO, 1);
    for (;;) {} // wait here forever
  }
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_TARGET1);
  mqtt.subscribe(TOPIC_TARGETSTATE);
  printf("MQTT broker connected.\n");
  digitalWrite(YELLOW_GPIO, 1);
  delay(3000);
  digitalWrite(YELLOW_GPIO, 0);
}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_TARGET1) == 0) {
    payload[length] = 0;
    int value = atoi((char*)payload); 
    if (value == 1) {
      digitalWrite(GREEN_GPIO, 1);
      ledstate = ledon;
      randomNumber = random(1, 3);
    }
  }
  if (strcmp(topic, TOPIC_TARGETSTATE) == 0) {
    payload[length] = 0;
    int gmode = atoi((char*)payload); 
    if (gmode == 2) {
      game_state = ingame;
    }
    else{
      game_state = g_off;
    }
  }
}


void setup() {
  randomSeed(analogRead(0));
  pinMode(GREEN_GPIO, OUTPUT);
  pinMode(YELLOW_GPIO, OUTPUT);
  irrecv.enableIRIn();
  Serial.begin(115200);
  connect_wifi();
  connect_mqtt();
  game_state = g_off;
  ledstate = ledoff;
  start_time = 0;
  timestamp = millis();
  x = 1;
}


void loop() {

  mqtt.loop();

  if (game_state == ingame){

    if (ledstate == ledon) { //ledstate check

      resetMillis();
      now = millis();

      if (now - start_time >= 5000) { //Check how much time has passed.
        digitalWrite(GREEN_GPIO, 0);
        ledstate = ledoff;
        x = 1;
        if (randomNumber == 1){
          mqtt.publish(TOPIC_RANDOMNUM1, "2");
        }
        else if (randomNumber == 2){
          mqtt.publish(TOPIC_RANDOMNUM1, "3");
        }
      }

      else if (now - start_time < 5000){
        if (irrecv.decode(&results)) {
          Serial.println(results.value, HEX);
          if (results.decode_type == NEC) {
            if (results.value == 0xFF6897) { //Check ir
              digitalWrite(GREEN_GPIO, 0);
              mqtt.publish(TOPIC_HIT, "1");
              ledstate = ledoff;
              x = 1;
              if (randomNumber == 1){
                mqtt.publish(TOPIC_RANDOMNUM1, "2");
              }
              else if (randomNumber == 2){
                mqtt.publish(TOPIC_RANDOMNUM1, "3");
              }
            }
          }
          irrecv.resume();
        }
      }
    }
  
  }
  else if (game_state == g_off){
    ledstate = ledoff;
    digitalWrite(GREEN_GPIO, 0);
    x = 1;
  
  }
}

void resetMillis() { //set led start time
  if (x == 1){
  timestamp = millis();
  start_time = timestamp;
  x = 0;
  } 
}
