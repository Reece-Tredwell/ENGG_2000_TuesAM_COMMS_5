#pragma once
#include "ErrorCode.hpp"
#include <ArduinoJson.h>

#define CEP_MAX_PACKET_SIZE 512
#define CEP_WIFI_TIMEOUT 5

#define IR_SENSOR_PIN 3
#define IR_SENSOR_LED_PIN 13

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

    ErrorCode connect(const char* ssid, const IPAddress& rip, int rp) {
      remoteIP = rip;
      remotePort = rp;

      Serial.print("Connecting to ");
      Serial.println(ssid);

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

      return ErrorCode::SUCCESSFUL;
    }
    void disconnect() {
      WiFi.disconnect();
    }
    void onSpeedCommand(int32_t speed) {

    }
    void onDoorCommand(bool state) {

    }
    void onLedCommand(int32_t pin, bool state) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, state);
    }
    void onStopCommand() {
      
    }
    void processPackets() {
      int packetSize = udp.parsePacket();
      if (packetSize == 0) return;

      Serial.print("packet of size ");
      Serial.print(packetSize);
      Serial.println(" was received");

      char packetBuffer[CEP_MAX_PACKET_SIZE];
      udp.read(packetBuffer, CEP_MAX_PACKET_SIZE);

      StaticJsonDocument<200> doc;
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