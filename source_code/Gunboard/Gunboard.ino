#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <IRsend.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SHOT_GPIO 6
#define RELOAD_GPIO 15
#define START_GPIO 7
#define BUZZER_GPIO 40
#define BUZZER2_GPIO 41
#define TOPIC_SCORE  TOPIC_PREFIX "/score"
#define TOPIC_SUMSCORE  TOPIC_PREFIX "/sumscore"
#define TOPIC_RANDOMNUM1    TOPIC_PREFIX "/randomnum1"
#define TOPIC_RANDOMNUM2    TOPIC_PREFIX "/randomnum2"
#define TOPIC_RANDOMNUM3    TOPIC_PREFIX "/randomnum3"
#define TOPIC_GAMESTATE    TOPIC_PREFIX "/gamestate"
#define TOPIC_OFSSTATE    TOPIC_PREFIX "/ofsstate"
#define TOPIC_COUNTDOWN    TOPIC_PREFIX "/countdown"
#define TOPIC_CHANGEGAMESTATE    TOPIC_PREFIX "/changegamestate"

const uint16_t IR_SEND_PIN = 8;
int randomNumber,count,score,hour,minute,sec,last,t_left,last_m,now_m;
int set_t,now_t,left_t,last_t,now_w,set;
int x = 10, y;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);
IRsend irsend(IR_SEND_PIN);


enum sw_state_t {
  WAIT_SW_PRESS,
  DEBOUNCE1,
  WAIT_SW_RELEASE,
  DEBOUNCE2,
};

enum game_state_t{
  wait,
  ready_to_start,
  
  ingame,
  g_off,
  game_end
};

enum ofs_state_t{
  ofs_on,
  ofs_off
};


uint32_t last_publish;
game_state_t game_state;
ofs_state_t ofs_state;
sw_state_t sh_state;
sw_state_t re_state;
sw_state_t st_state;
sw_state_t ag_state;

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
    for (;;) {}  // wait here forever
  }
  //  mqtt.set_tCallback(mqtt_callback);
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_SCORE);
  mqtt.subscribe(TOPIC_CHANGEGAMESTATE);
  mqtt.subscribe(TOPIC_OFSSTATE);
  mqtt.subscribe(TOPIC_COUNTDOWN);
  printf("MQTT broker connected.\n");
}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_SCORE) == 0) {
    score++;
    String scoreStr = String(score);
    mqtt.publish(TOPIC_SUMSCORE, scoreStr.c_str());
  }
  if (strcmp(topic, TOPIC_COUNTDOWN) == 0) {
    payload[length] = 0; // null-terminate the payload to treat it as a string
    int countdowntime = atoi((char*)payload);
    set_t = countdowntime*60000;
    y = set_t;
  }
  if (strcmp(topic, TOPIC_CHANGEGAMESTATE) == 0) {
    payload[length] = 0; // null-terminate the payload to treat it as a string
    int gmode = atoi((char*)payload); 
    if (gmode == 0) {
      game_state = g_off;
    }
    else if (gmode == 1) {
      game_state = ready_to_start;
    }
    else if (gmode == 2) {
      game_state = ingame;
    }
    else if (gmode == 3) {
      game_state = wait;
    }
    else if (gmode == 4) {
      game_state = game_end;
    }
  }
  if (strcmp(topic, TOPIC_OFSSTATE) == 0) {
    payload[length] = 0; // null-terminate the payload to treat it as a string
    int ofsmode = atoi((char*)payload); 
    if (ofsmode == 0) {
      ofs_state = ofs_off;
    }
    else if (ofsmode == 1) {
      ofs_state = ofs_on;
    }
  }
}


void setup() {
  Serial.begin(115200);
  irsend.begin();
  Wire.begin(48, 47);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  randomSeed(analogRead(0));
  pinMode(SHOT_GPIO, INPUT_PULLUP);
  pinMode(RELOAD_GPIO, INPUT_PULLUP);
  pinMode(START_GPIO, INPUT_PULLUP);
  sh_state = WAIT_SW_PRESS;
  re_state = WAIT_SW_PRESS;
  st_state = WAIT_SW_PRESS;
  ag_state = WAIT_SW_PRESS;
  game_state = g_off;
  ofs_state = ofs_off;
  connect_wifi();
  connect_mqtt();
  count = 12;
  last_publish = 0;
  last_t = 0;
  last = 0;
  last_m = 0;
  set_t = 30000;
  set = 60000;
  score = 0;
  randomNumber = random(1,4);
}


void loop() {
  mqtt.loop();
  // check for incoming subscribed topics
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  uint32_t now_w;
  uint32_t now_t;
  uint32_t now_m;
  uint32_t now;
  if (game_state == wait){
    if (ofs_state == ofs_on){
      noTone(BUZZER2_GPIO);
      now_w = millis();
      display.clearDisplay();
      if (set_t - 1000 >=0){
        if (now_w - last_t >= 1000){
          wait_time(set_t,1000,x);
          last_t = now_w;
          set_t -= 1000;
        }
      }
      else{
        int GN = 1;
        String GNStr = String(GN);
        mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
      }
    }
    else if (ofs_state == ofs_off){
      int GN = 1;
      String GNStr = String(GN);
      mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
    }
  }
  else if(game_state == g_off){
      noTone(BUZZER2_GPIO);
      display.clearDisplay();
      display.display();
      set_t = y;
      set = 60000;
  }
  else if(game_state == ready_to_start){
    now_m = millis();
    display.clearDisplay();
    display.printf("Press Button to start\n");
    display.setTextSize(2);
    display.setCursor(25, 25);
    display.printf("Start!");
    display.display();
    if (ofs_state == ofs_on){
      tone(BUZZER2_GPIO, 800);
    }
    ///start switch
    if (st_state == WAIT_SW_PRESS) {
          if (digitalRead(START_GPIO) == 0) { // SW is pressed
            st_state = DEBOUNCE1;
            last_publish = millis();
          }
        }
        else if (st_state == DEBOUNCE1) {
          now = millis();
          if (now - last_publish >= 10) {
            st_state = WAIT_SW_RELEASE;
            last_publish = now;
          }
        }
        else if (st_state == WAIT_SW_RELEASE) {
          if (digitalRead(START_GPIO) == 1) {        
            st_state = DEBOUNCE2;
            
            last_publish = millis();
          }
        }
        else if (st_state == DEBOUNCE2) {
          now = millis();
          if (now - last_publish >= 10) {
            st_state = WAIT_SW_PRESS;
            int GN = 2;
            String GNStr = String(GN);
            mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
            display.clearDisplay();
            display.setTextSize(1);
            display.setCursor(0, 10);
            display.printf("Bullet : 12/12\n");
            now = millis();
            if (now - last_publish >= 10) {
              if (randomNumber == 1){
                mqtt.publish(TOPIC_RANDOMNUM1, "3");
              }
              else if (randomNumber == 2){
                mqtt.publish(TOPIC_RANDOMNUM2, "1");
              }
              else if (randomNumber == 3){
                mqtt.publish(TOPIC_RANDOMNUM3, "2");
              }
              last_publish = now;
            }
            count = 12;
            score = 0;
            String scoreStr = String(score);
            mqtt.publish(TOPIC_SUMSCORE, scoreStr.c_str());
            last_publish = now;
          }
        }
      last_m = 0;
  }
  else if(game_state == ingame){
    now_t = millis();
    now_m = millis();
    noTone(BUZZER2_GPIO);
    if (set - 1000 >=0){
      ///timer
      if (now_t - last >= 1000){
        display.clearDisplay();
        display.setCursor(0, 10);
        display.printf("Bullet : %d/12\n", count);
        wait_time(set,1000,x+10);
        last = now_t;
        set -= 1000;
      }
      ///shooting
      if (count > 0){
        if (sh_state == WAIT_SW_PRESS) {
          if (digitalRead(SHOT_GPIO) == 0) { // SW is pressed
            sh_state = DEBOUNCE1;
            count--;
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            last_publish = millis();
            irsend.sendNEC(0xFF6897,32);
            Serial.printf("shot");
            sound_shot_eff();
          }
        }
        else if (sh_state == DEBOUNCE1) {
          now = millis();
          if (now - last_publish >= 10) {
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            sh_state = WAIT_SW_RELEASE;
            last_publish = now;
          }
        
        }
        
        else if (sh_state == WAIT_SW_RELEASE) {
          if (digitalRead(SHOT_GPIO) == 1) {        
            sh_state = DEBOUNCE2;
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            last_publish = millis();
          }
        }
        else if (sh_state == DEBOUNCE2) {
          now = millis();
          if (now - last_publish >= 10) {
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            sh_state = WAIT_SW_PRESS;
            last_publish = now;
          }
        }
        display.display();
      }
      ///////nobullet
      else{
        if (sh_state == WAIT_SW_PRESS) {
          if (digitalRead(SHOT_GPIO) == 0) { // SW is pressed
            sh_state = DEBOUNCE1;
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            last_publish = millis();
            sound_nobullet();
          }
        }
        else if (sh_state == DEBOUNCE1) {
          now = millis();
          if (now - last_publish >= 10) {
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            sh_state = WAIT_SW_RELEASE;
            last_publish = now;
          }
        }
        
        else if (sh_state == WAIT_SW_RELEASE) {
          if (digitalRead(SHOT_GPIO) == 1) {        
            sh_state = DEBOUNCE2;
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            last_publish = millis();
          }
        }
        else if (sh_state == DEBOUNCE2) {
          now = millis();
          if (now - last_publish >= 10) {
            //ขึ้นจอ
            display.clearDisplay();
            display.setCursor(0, 10);
            display.printf("Bullet : %d/12\n", count);
            wait_time(set,1000,x+10);
            sh_state = WAIT_SW_PRESS;
            last_publish = now;
          }
        }
        display.display();
      }
      ///////reload
      if (re_state == WAIT_SW_PRESS) {
        if (digitalRead(RELOAD_GPIO) == 0) { // SW is pressed
          //ขึ้นจอ
          display.clearDisplay();
          display.setCursor(0, 10);
          display.printf("Bullet : %d/12\n", count);
          wait_time(set,1000,x+10);
          re_state = DEBOUNCE1;
          last_publish = millis();
          sound_reload_eff();
          Serial.printf("shot");
        }
      }
      else if (re_state == DEBOUNCE1) {
        now = millis();
        if (now - last_publish >= 10) {
          re_state = WAIT_SW_RELEASE;
          count = 12;
          //ขึ้นจอ
          display.clearDisplay();
          display.setCursor(0, 10);
          display.printf("Bullet : %d/12\n", count);
          wait_time(set,1000,x+10);
          display.display();
          last_publish = now;
        }
      
      }
      
      else if (re_state == WAIT_SW_RELEASE) {
        if (digitalRead(RELOAD_GPIO) == 1) {        
          //ขึ้นจอ
          display.clearDisplay();
          display.setCursor(0, 10);
          display.printf("Bullet : %d/12\n", count);
          wait_time(set,1000,x+10);
          re_state = DEBOUNCE2;
          last_publish = millis();
        }
      }
      else if (re_state == DEBOUNCE2) {
        now = millis();
        if (now - last_publish >= 10) {
          //ขึ้นจอ
          display.clearDisplay();
          display.setCursor(0, 10);
          display.printf("Bullet : %d/12\n", count);
          wait_time(set,1000,x+10);
          re_state = WAIT_SW_PRESS;
          last_publish = now;
        }
      }
      display.display();
    }
    else{
      int GN = 4;
      String GNStr = String(GN);
      mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
    }
  }
  ///Check Score
  else if (game_state == game_end){
    if (ofs_state == ofs_on){
      if (score >= 13){
        noTone(BUZZER2_GPIO);
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(35, 15);
        display.printf("Well done!\n");
        display.setTextSize(1);
        display.setCursor(5, 30);
        display.printf("Time to do your job!");
        display.display();
        set_t = y;
        set = 60000;
        if (ag_state == WAIT_SW_PRESS) {
        if (digitalRead(START_GPIO) == 0) { // SW is pressed
          ag_state = DEBOUNCE1;
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE1) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_RELEASE;
          last_publish = now;
        }
      }
      else if (ag_state == WAIT_SW_RELEASE) {
        if (digitalRead(START_GPIO) == 1) {        
          ag_state = DEBOUNCE2;
            
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE2) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_PRESS;
          int GN = 3;
          String GNStr = String(GN);
          mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
        }
      }
      }
      else if (score < 13){
        tone(BUZZER2_GPIO, 800);
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(35, 30);
        display.printf("Try Again!");
        display.display();
        set_t = y;
        set = 60000;
        if (ag_state == WAIT_SW_PRESS) {
        if (digitalRead(START_GPIO) == 0) { // SW is pressed
          ag_state = DEBOUNCE1;
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE1) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_RELEASE;
          last_publish = now;
        }
      }
      else if (ag_state == WAIT_SW_RELEASE) {
        if (digitalRead(START_GPIO) == 1) {        
          ag_state = DEBOUNCE2;
            
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE2) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_PRESS;
          int GN = 1;
          String GNStr = String(GN);
          mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
        }
      }
      }
    }
    else if (ofs_state == ofs_off){
      noTone(BUZZER2_GPIO);
      display.clearDisplay();
      display.setTextSize(1);
        display.setCursor(35, 15);
        display.printf("Game END!\n");
        display.setTextSize(1);
        display.setCursor(20, 30);
        display.printf("Press start to");
        display.setTextSize(1);
        display.setCursor(30, 45);
        display.printf("play again!");
        display.display();
      set_t = y;
      set = 60000;
      if (ag_state == WAIT_SW_PRESS) {
        if (digitalRead(START_GPIO) == 0) { // SW is pressed
          ag_state = DEBOUNCE1;
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE1) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_RELEASE;
          last_publish = now;
        }
      }
      else if (ag_state == WAIT_SW_RELEASE) {
        if (digitalRead(START_GPIO) == 1) {        
          ag_state = DEBOUNCE2;
            
          last_publish = millis();
        }
      }
      else if (ag_state == DEBOUNCE2) {
        now = millis();
        if (now - last_publish >= 10) {
          ag_state = WAIT_SW_PRESS;
          int GN = 1;
          String GNStr = String(GN);
          mqtt.publish(TOPIC_GAMESTATE, GNStr.c_str());
        }
      }
    }
  }
}

    
  


void sound_shot_eff(){
  tone(BUZZER_GPIO, 5000);
  delay(120);
  tone(BUZZER_GPIO, 900);
  delay(80);
  noTone(BUZZER_GPIO);
}
void sound_nobullet(){
  tone(BUZZER_GPIO,2500);
  delay(150);
  noTone(BUZZER_GPIO);
}
void sound_reload_eff(){
  tone(BUZZER_GPIO, 200);
  delay(120);
  tone(BUZZER_GPIO, 500);
  delay(80);
  noTone(BUZZER_GPIO);
}

void wait_time(int set_t, int now_t, int x){
  t_left = set_t-now_t;
  hour = int(t_left/3600000);
  minute = int((t_left-(hour*3600000))/60000);
  sec = int((t_left-(hour*3600000)-(minute*60000))/1000);
  display.setTextSize(1);
  display.setCursor(0,x);
  display.printf("Timer :");
  display.setCursor(0, x+10);
  display.printf("   %d Hours\n   %d Minutes\n   %d Seconds\n",hour,minute,sec);
  display.display(); 
}