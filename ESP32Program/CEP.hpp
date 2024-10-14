#pragma once
#include "ErrorCode.hpp"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "macros.hpp"

namespace CEP {
  class CEP {
  private:
    WiFiUDP udp;
    IPAddress remoteIP;
    int remotePort;

    // system time the time packet was received, used to calculate true time
    int32_t timePacketReceivedTime;
    // the time the time packet gave us
    int32_t timePacketTime;

    int32_t lastHeartbeatReceivedTime;
    int32_t lastHeartbeatSentTime;

    // ready is defined as having a connection and receiving the time command
    bool ready;
  public:
    CEP() {
      lastHeartbeatReceivedTime = 0;
      lastHeartbeatReceivedTime = 0;
      ready = false;
    }
    ErrorCode setup() {
      pinMode(IR_SENSOR_PIN, INPUT);

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
      udp.begin(CEP_PORT);
      uint32_t timeout = 0;
      while (WiFi.status() != WL_CONNECTED && timeout < CEP_WIFI_TIMEOUT) {
        delay(1000);
        timeout++;
      }

      if (timeout >= CEP_WIFI_TIMEOUT) {
        Serial.print("Failed to connect in ");
        Serial.print(timeout);
        Serial.println(" seconds");

        // Enter infinite loop
        while (true) {
          onLedCommand(G_LED_PIN, true);
          onLedCommand(Y_LED_PIN, true);
          onLedCommand(R_LED_PIN, true);
          onLedCommand(B_LED_PIN, true);
          delay(500);
          onLedCommand(G_LED_PIN, false);
          onLedCommand(Y_LED_PIN, false);
          onLedCommand(R_LED_PIN, false);
          onLedCommand(B_LED_PIN, false);
          delay(500);
        }
      }

      Serial.print("WiFi connected. Local IP:");
      Serial.println(WiFi.localIP());
      
      udp.begin(CEP_PORT);
      return ErrorCode::SUCCESSFUL;
    }
    void disconnect() {
      WiFi.disconnect();
    }
    void sendPacket(String data) {
      udp.beginPacket(remoteIP, remotePort);
      udp.print(data);
      udp.endPacket();
    }
    void sendHeartbeat(int32_t currentTime) {
      JsonDocument doc;
      doc["cmd"] = "heartbeat";
      doc["timestamp"] = currentTime;
  
      char output[CEP_MAX_PACKET_SIZE];
      serializeJson(doc, output, CEP_MAX_PACKET_SIZE);
      sendPacket(String(output));

      lastHeartbeatSentTime = currentTime;
    }
    // Send a message to the CCP that will be displayed on debug output
    void sendMessage(String message) {
      JsonDocument doc;
      doc["cmd"] = "message";
      doc["message"] = message;

      char output[CEP_MAX_PACKET_SIZE];
      serializeJson(doc, output, CEP_MAX_PACKET_SIZE);
      sendPacket(String(output));
    }
    ErrorCode onTimeCommand(int32_t timestamp) {
      sendMessage("Received time command");
      timePacketReceivedTime = millis();
      timePacketTime = timestamp;
      ready = true;
    }
    ErrorCode onHeartbeatCommand(int32_t timestamp) {
      lastHeartbeatReceivedTime = timestamp;
    }
    ErrorCode onSpeedCommand(int32_t speed) {
      // https://www.pololu.com/product/4733
      if (speed > 0) {
        analogWrite(MOTOR_IN_1_PIN, 255);
      } else {
        analogWrite(MOTOR_IN_2_PIN, 255);
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
      // refer to https://www.pololu.com/product/4733 for more info
      analogWrite(MOTOR_IN_1_PIN, 0);
      analogWrite(MOTOR_IN_2_PIN, 255);
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onShutdownCommand() {
      sendMessage("Shutdown requested, goodbye for now");
      onLedCommand(G_LED_PIN, false);
      onLedCommand(Y_LED_PIN, false);
      onLedCommand(R_LED_PIN, false);
      onLedCommand(B_LED_PIN, false);
      disconnect();
      exit(0);
    }
    void processPackets() {
      char packetBuffer[CEP_MAX_PACKET_SIZE];
      udp.read(packetBuffer, CEP_MAX_PACKET_SIZE);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, packetBuffer);
      
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
      }

      String command = String(doc["cmd"]);
      int32_t timestamp = doc["timestamp"];

      if (command == "time") {
        onTimeCommand(timestamp);
      } else if (command == "heartbeat") {
        onHeartbeatCommand(doc["speed"]);
      } else if (command == "speed") {
        onSpeedCommand(doc["speed"]);
      } else if (command == "door") {
        onDoorCommand(doc["state"]);
      } else if (command == "led") {
        onLedCommand(doc["pin"], doc["state"]);
      } else if (command == "stop") {
        onStopCommand();
      } else if (command == "shutdown") {
        onShutdownCommand();
      } else {
        Serial.print("Received unknown command ");
        Serial.println(command);
      }
    }
    // To be used for all time based methods
    int32_t getCurrentTime() {
      return timePacketReceivedTime + (millis() - timePacketTime);
    }
    void heartbeatTimeout() {
        onStopCommand();
        onLedCommand(G_LED_PIN, true);
        onLedCommand(Y_LED_PIN, true);
        onLedCommand(R_LED_PIN, true);
        onLedCommand(B_LED_PIN, true);
        exit(-1);
    }
    void update() {
      int32_t currentTime = getCurrentTime();

      // Receive packets
      while (udp.parsePacket() != 0) {
        processPackets();
      }

      // Send heartbeats
      if (ready && currentTime - lastHeartbeatSentTime > CEP_HEARTBEAT_DELAY) {
        sendHeartbeat(currentTime);
      }

      // Wait for heartbeat timeout
      if (ready && currentTime - lastHeartbeatReceivedTime > CEP_HEARTBEAT_TIMEOUT) {
        heartbeatTimeout();
      }

      // Self-emergency stop
      bool somethingIsInFrontOfUs = digitalRead(IR_SENSOR_PIN);
      if (somethingIsInFrontOfUs) {
        sendMessage("Object detected infront, self-avoidance protocol activated");
        onStopCommand();
      }
    }
  };
}