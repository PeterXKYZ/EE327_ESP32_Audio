#include <Arduino.h>
#include <ArduinoWebsockets.h>

// Audio stuff -----------------------------------------
#include "XT_DAC_Audio.h"
#include "chime.h"          // Add the "const" keyword to prevent dram overflow error
#include "door_unlocked.h"      // https://www.xtronical.com/dac-audio-4-2-1-released/ See comments
#include "go_away.h"
#include "not_here.h"
// -----------------------------------------------------


// WiFi ------------------------------------------------
#include <WiFi.h>
// -----------------------------------------------------



// ESP Now includes ------------------------------------
#include <esp_now.h>
// -----------------------------------------------------


// ESP Now ---------------------------------------------
// ESP32 CAM MAC Address: 3C-61-05-16-F9-68
uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x16, 0xF9, 0x68};    
esp_now_peer_info_t peerInfo;

int myData = -1;

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  Serial.println("message");
  memcpy(&myData, incomingData, sizeof(myData));
}
// -----------------------------------------------------


// button stuff ---------------------------------------
const int buttonPin = 26;
int pressed_curr = 0;
int pressed_prev = 0;
unsigned long button_time = 0;
// 5 sec delay between consecutive button presses for anti-spam
const size_t button_interval = 5000;  
// -----------------------------------------------------


XT_Wav_Class ChimeAudio(chime);
XT_Wav_Class S1_Audio(door_unlocked);
XT_Wav_Class S2_Audio(not_here);
XT_Wav_Class S3_Audio(go_away);
XT_DAC_Audio_Class DacAudio(25, 0);

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);

  ChimeAudio.Speed = 2;

  // ESP NOW -------------------------------------------
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
  // ---------------------------------------------------

  // button stuff -----------------------------------
  pinMode(buttonPin, INPUT);
  // ------------------------------------------------
}

void loop() {
  DacAudio.FillBuffer();
  unsigned long curr_time = millis();


  // button stuff -----------------------------------
  pressed_curr = digitalRead(buttonPin);
  if (pressed_curr > pressed_prev && curr_time - button_time > button_interval) {
    button_time = curr_time;
    DacAudio.Play(&ChimeAudio, false);  // "false" stops all tracks and play the commanded one only
    Serial.println("playing chime");    // https://www.xtronical.com/dac-audio-4-2-1-released/ See comments
  }
  pressed_prev = pressed_curr;
  // ------------------------------------------------

  switch (myData) {
    case 1:
      DacAudio.Play(&S1_Audio, false);
      Serial.println("play s1");
      myData = -1;
      break;
    case 2:
      DacAudio.Play(&S2_Audio, false);
      Serial.println("play s2");
      myData = -1;
      break;
    case 3:
      DacAudio.Play(&S3_Audio, false);
      Serial.println("play s3");
      myData = -1;
      break;
    default:
      break;
  }
}