#include "HardwareSerial.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

#define DISCONNECT_TIME 1000.0 // ms
#define LEFT 32
#define RIGHT 33
#define WEAPON 2

#define DRIVE_VALUE 180
#define STOP_VALUE 95

const char *ssid = "test";
const char *password = "password123";
const char *hostname = "esp.local";

WiFiUDP udp;
uint port = 3526;

Servo leftMotor;
Servo rightMotor;
Servo weaponMotor;

double backStart = -1;
double backDuration = 0;

double leftStart = -1;
double leftDuration = 0;

double rightStart = -1;
double rightDuration = 0;

double frontStart = -1;
double frontDuration = 0;

void stopAll() {
  leftMotor.write(STOP_VALUE);
  rightMotor.write(STOP_VALUE);
  weaponMotor.write(STOP_VALUE);

  frontStart = -1;
  frontDuration = 0;

  digitalWrite(2, int(millis() / 100.0) % 2 == 0 ? HIGH : LOW);
  // digitalWrite(2, LOW);
}

void stop() {
  frontStart = -1;
  frontDuration = 0;
  backStart = -1;
  backDuration = 0;
  leftStart = -1;
  leftDuration = 0;
  rightStart = -1;
  rightDuration = 0;

  leftMotor.write(STOP_VALUE);
  rightMotor.write(STOP_VALUE);
  weaponMotor.write(STOP_VALUE);
}

void front() {
  Serial.println("front");
  leftMotor.write(DRIVE_VALUE);
  rightMotor.write(DRIVE_VALUE);
  
  backStart = -1;
  backDuration = 0;
  leftStart = -1;
  leftDuration = 0;
  rightStart = -1;
  rightDuration = 0;
}

void back() {
  frontStart = -1;
  frontDuration = 0;
  leftStart = -1;
  leftDuration = 0;
  rightStart = -1;
  rightDuration = 0;

  leftMotor.write(-DRIVE_VALUE);
  rightMotor.write(-DRIVE_VALUE);
}

void left() {
  leftMotor.write(-DRIVE_VALUE);
  rightMotor.write(DRIVE_VALUE);

  
  frontStart = -1;
  frontDuration = 0;
  backStart = -1;
  backDuration = 0;
  rightStart = -1;
  rightDuration = 0;
}

void right() {
  frontStart = -1;
  frontDuration = 0;
  leftMotor.write(DRIVE_VALUE);
  rightMotor.write(-DRIVE_VALUE);

  
  frontStart = -1;
  frontDuration = 0;
  backStart = -1;
  backDuration = 0;
  leftStart = -1;
  leftDuration = 0;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  WiFi.softAP(ssid, password);
  WiFi.softAPsetHostname(hostname);

  Serial.println("Started Wifi");
  Serial.println(WiFi.softAPIP().toString());

  leftMotor.attach(LEFT);
  rightMotor.attach(RIGHT);
  weaponMotor.attach(WEAPON);

  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(13, HIGH);
  udp.begin(port);
}

bool active = false;
bool on = true;

double timeSinceLastPacket;

void loop() {
  if(millis() - timeSinceLastPacket > DISCONNECT_TIME) {
    Serial.println("Disconnect");
    stopAll();
  } else if(frontStart + frontDuration >= millis()) {
    front();
  } else if (backStart + backDuration >= millis()) {
    back();
  } else if (leftStart + leftDuration >= millis()) {
    left();
  } else if (rightStart + rightDuration >= millis()) {
    right();
  } else {
    digitalWrite(2, active ? LOW : HIGH);
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
      double time = packet.substring(String("front").length()).toDouble() * 1000;
      frontDuration = time == 0 ? 500 : time;
      frontStart = millis();
      Serial.printf("Going front for %f seconds\n", frontDuration/1000);
    } else if (packet.startsWith("back")) {
      double time = packet.substring(String("back").length()).toDouble() * 1000;
      backDuration = time == 0 ? 500 : time;
      backStart = millis();
      Serial.printf("Going back for %f seconds\n", backDuration/1000);
    } else if (packet.startsWith("left")) {
      double time = packet.substring(String("left").length()).toDouble() * 1000;
      leftDuration = time == 0 ? 500 : time;
      leftStart = millis();
      Serial.printf("Going left for %f seconds\n", leftDuration/1000);
    } else if (packet.startsWith("right")) {
      double time = packet.substring(String("right").length()).toDouble() * 1000;
      rightDuration = time == 0 ? 500 : time;
      rightStart = millis();
      Serial.printf("Going right for %f seconds\n", rightDuration/1000);
    } else if(packet.startsWith("s")) {
      stop();
      // servo.write(packet.substring(1).toInt());
      // Serial.println(packet.substring(1).toInt());
    } else {
      Serial.println("NO INSTRUCTION");
    }

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("ACK");
    udp.endPacket();

    timeSinceLastPacket = millis();
  }
}


