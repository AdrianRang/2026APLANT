#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

#define DISCONNECT_TIME 1000.0 // ms

const char *ssid = "test";
const char *password = "password123";
const char *hostname = "esp.local";

WiFiUDP udp;
uint port = 3526;

Servo servo;

double forewardStart = -1;
double forewardDuration = 0;

void stopAll() {
  servo.write(95);

  forewardStart = -1;
  forewardDuration = 0;

  digitalWrite(2, int(millis() / 100.0) % 2 == 0 ? HIGH : LOW);
  // digitalWrite(2, LOW);
}

void stop() {
  forewardStart = -1;
  forewardDuration = 0;
}

void front() {
}

void back() {
  forewardStart = -1;
  forewardDuration = 0;
}

void left() {
  forewardStart = -1;
  forewardDuration = 0;
}

void right() {
  forewardStart = -1;
  forewardDuration = 0;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  WiFi.softAP(ssid, password);
  WiFi.softAPsetHostname(hostname);

  Serial.println("Started Wifi");
  Serial.println(WiFi.softAPIP().toString());

  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(13, HIGH);
  servo.attach(12);

  udp.begin(port);
}

bool active = false;
bool on = true;

double timeSinceLastPacket;


void loop() {
  if(millis() - timeSinceLastPacket > DISCONNECT_TIME) {
    stopAll();
  } else {
    digitalWrite(2, active ? LOW : HIGH);
  }

  if(forewardStart + forewardDuration >= millis()) {
    front();
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    String packet = udp.readString();

    Serial.printf("Received from %s:%d: %s\n", 
      udp.remoteIP().toString().c_str(), 
      udp.remotePort(), 
      packet.c_str());


    if(packet.equals("LED")) {
      active = !active;
      digitalWrite(2, active ? LOW : HIGH);
    } else if (packet.startsWith("front")) {
      double time = packet.substring(String("front").length()).toDouble();
      forewardDuration = time == 0 ? 500 : time;
      forewardStart = millis();
    } else if(packet.startsWith("s")) {
      servo.write(packet.substring(1).toInt());
      Serial.println(packet.substring(1).toInt());
    } else {
      Serial.println("NO INSTRUCTION");
    }

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("ACK");
    udp.endPacket();

    timeSinceLastPacket = millis();
  }
}


