#pragma once
#include "ErrorCode.hpp"
#include <ArduinoJson.h>

#define CEP_MAX_PACKET_SIZE 512
#define CEP_WIFI_TIMEOUT 5

#define IR_SENSOR_PIN 25
#define SERVO_DATA_PIN 24
#define IR_PHOTORESISTOR_PIN 26
#define PHOTORESISTOR_1_PIN 23

#define G_LED_PIN 11
#define Y_LED_PIN 12
#define R_LED_PIN 13
#define B_LED_PIN 14

#define MOTOR_IN_1_PIN 6
#define MOTOR_IN_2_PIN 7

namespace CEP {
  class CEP {
  private:
    IPAddress local;
    WiFiUDP udp;
    IPAddress remoteIP;
    int remotePort;
    uint32_t lastReceivedSpeedCommandTimestamp;
  public:
    CEP() {
      lastReceivedSpeedCommandTimestamp = 0;
      pinMode(IR_SENSOR_PIN, INPUT);
    }
    ErrorCode setup() {
      pinMode(IR_SENSOR_PIN, INPUT);
      pinMode(IR_SENSOR_PIN, INPUT);
      pinMode(IR_SENSOR_PIN, INPUT);
      pinMode(IR_SENSOR_PIN, INPUT);

      pinMode(G_LED_PIN, OUTPUT);
      pinMode(Y_LED_PIN, OUTPUT);
      pinMode(R_LED_PIN, OUTPUT);
      pinMode(B_LED_PIN, OUTPUT);

      pinMode(MOTOR_IN_1_PIN, OUTPUT);
      pinMode(MOTOR_IN_2_PIN, OUTPUT);
    }
    ErrorCode connect(const char* ssid, const IPAddress& rip, int rp) {
      remoteIP = rip;
      remotePort = rp;

      Serial.print("Connecting to ");
      Serial.println(ssid);

      if (!WiFi.config(CEP_IP)) {
        Serial.println("Failed to configure Static IP");
        return ErrorCode::CONFIG_FAILED;
    }

      WiFi.begin(ssid);

      uint32_t timeout = 0;
      while (WiFi.status() != WL_CONNECTED && timeout < CEP_WIFI_TIMEOUT) {
        delay(1000);
        timeout++;
      }

      if (timeout >= CEP_WIFI_TIMEOUT) {
        Serial.print("Failed to connect in ");
        Serial.print(timeout);
        Serial.println(" seconds");
        return ErrorCode::CONNECTION_FAILED;
      }

      local = WiFi.localIP();

      Serial.print("WiFi connected. Local IP:");
      Serial.println(local);
      
      udp.begin(CEP_PORT);
      return ErrorCode::SUCCESSFUL;
    }
    void disconnect() {
      WiFi.disconnect();
    }
    ErrorCode onSpeedCommand(int32_t speed) {
      // https://www.pololu.com/product/4733
      if (speed > 0) {
        analogWrite(MOTOR_IN_1_PIN, 63);
      } else {
        analogWrite(MOTOR_IN_2_PIN, 63);
      }

      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onDoorCommand(bool state) {
      return ErrorCode::NOT_IMPLEMENTED;
    }
    ErrorCode onLedCommand(int32_t pin, bool state) {
      digitalWrite(pin, state);
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onStopCommand() {
      return ErrorCode::NOT_IMPLEMENTED;
    }
    void processPackets() {
      int packetSize = udp.parsePacket();
      if (packetSize == 0)
        return;

      Serial.print("packet of size ");
      Serial.print(packetSize);
      Serial.println(" was received");

      // 512 bytes should be enough
      char packetBuffer[CEP_MAX_PACKET_SIZE];
      udp.read(packetBuffer, CEP_MAX_PACKET_SIZE);

      // 5 nodes is more than enough
      StaticJsonDocument<5> doc;
      DeserializationError error = deserializeJson(doc, packetBuffer);

      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
      }

      // Get command type
      String command = String(doc["cmd"]);
      // Get command timestamp for speed packet drops
      uint32_t timestamp = doc["timestamp"];

      // Drop old speed packets
      if (command == "SPEED" && timestamp > lastReceivedSpeedCommandTimestamp) {
        lastReceivedSpeedCommandTimestamp = timestamp;
        onSpeedCommand(doc["speed"]);

        Serial.print("Received SPEED command");
      } else if (command == "DOOR") {
        onDoorCommand(doc["state"]);

        Serial.println("Received DOOR command");
      } else if (command == "LED") {
        onLedCommand(doc["pin"], doc["state"]);

        Serial.println("Received LED command");
      } else if (command == "STOP") {
        onStopCommand();

        Serial.println("Received STOP command");
      } else {
        Serial.print("Received unknown command ");
        Serial.println(command);
      }
    }
    void sendPacket(String data) {
      udp.beginPacket(remoteIP, remotePort);
      udp.print(data);
      udp.endPacket();
    }
    void update() {
      sendPacket("Hello!");

      // Self-emergency stop
      bool somethingIsInFrontOfUs = digitalRead(IR_SENSOR_PIN);
      if (somethingIsInFrontOfUs) {
        onStopCommand();
      }

      processPackets();
    }
  };
}