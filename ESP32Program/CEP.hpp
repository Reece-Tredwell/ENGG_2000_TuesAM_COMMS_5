#pragma once
#include "ErrorCode.hpp"
#include <ArduinoJson.h>
#include <Servo.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "macros.hpp"
#include "CEPStateMachine.hpp"

// Maps from 0 - 100 to 0 - 255 as per pwm
int mapPWM(int val) {
  return (int)((float)val * 2.55f);
}

namespace CEP {
  class CEP {
  private:
    Servo doorServo;
    CEPStateMachine state;

    WiFiUDP udp;
    IPAddress remoteIP;
    int remotePort;

    // system time the time packet was received, used to calculate true time
    int32_t timePacketReceivedTime;
    // the time the time packet gave us
    int32_t timePacketTime;
    // used for deltaTime
    int32_t lastUpdateTime;

    int32_t lastHeartbeatReceivedTime;
    int32_t lastHeartbeatSentTime;

    // Speed we need to build up to
    float requestedSpeed;
    // Actualised speed
    float motorSpeed;

    // ready is defined as having a connection and receiving the time command
    bool ready;
  public:
    CEP() {
      lastHeartbeatReceivedTime = 0;
      lastHeartbeatReceivedTime = 0;
      lastUpdateTime = 0;
      requestedSpeed = 0;
      motorSpeed = 0;
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

      doorServo.attach(SERVO_DATA_PIN);
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
        notifyRemoval(250);
      }

      Serial.print("WiFi connected. Local IP:");
      Serial.println(WiFi.localIP());
      sendMessage("CEP is alive and kicking!");
      
      udp.begin(CEP_PORT);
      return ErrorCode::SUCCESSFUL;
    }
    void notifyRemoval(int blinkSpeed) {
      state.networkDisconnected();
      while (true) {
          onLedCommand(G_LED_PIN, true);
          onLedCommand(Y_LED_PIN, true);
          onLedCommand(R_LED_PIN, true);
          onLedCommand(B_LED_PIN, true);
          delay(blinkSpeed);
          onLedCommand(G_LED_PIN, false);
          onLedCommand(Y_LED_PIN, false);
          onLedCommand(R_LED_PIN, false);
          onLedCommand(B_LED_PIN, false);
          delay(blinkSpeed);
        }
    }
    void disconnect() {
      WiFi.disconnect();
      state.networkDisconnected();
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

      Serial.println("Sent Heartbeat");
    }
    // Send a message to the CCP that will be displayed on debug output
    void sendMessage(String message) {
      Serial.println(message);

      JsonDocument doc;
      doc["cmd"] = "message";
      doc["message"] = message;

      char output[CEP_MAX_PACKET_SIZE];
      serializeJson(doc, output, CEP_MAX_PACKET_SIZE);
      sendPacket(String(output));
    }
    // Time packets are treated as communication acknowledgements
    ErrorCode onTimeCommand(int32_t timestamp) {
      timePacketReceivedTime = millis();
      timePacketTime = timestamp;
      lastUpdateTime = getCurrentTime();
      ready = true;
      state.initialisationFinished();

      JsonDocument doc;
      doc["cmd"] = "ack";
      doc["timestamp"] = getCurrentTime();
  
      char output[CEP_MAX_PACKET_SIZE];
      serializeJson(doc, output, CEP_MAX_PACKET_SIZE);
      sendPacket(String(output));

      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onHeartbeatCommand(int32_t timestamp) {
      lastHeartbeatReceivedTime = timestamp;
    }
    ErrorCode onSpeedCommand(int32_t speed) {
      requestedSpeed = (float)speed;
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onDoorCommand(bool state) {
      if (state) {
        myServo.write(180);
      } else {
        myServo.write(0);
      }
      return ErrorCode::NOT_IMPLEMENTED;
    }
    ErrorCode onLedCommand(int32_t pin, bool state) {
      digitalWrite(pin, state);
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onStopCommand() {
      // refer to https://www.pololu.com/product/4733 for more info
      // Brake as fast as possible, maybe we need to tweak this for ABS sake
      analogWrite(MOTOR_IN_1_PIN, 0);
      analogWrite(MOTOR_IN_2_PIN, 255);
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onShutdownCommand() {
      sendMessage("Shutdown requested, goodbye for now");
      disconnect();
      // 2hz as requested by MCP document
      notifyRemoval(500);
    }
    void processPackets() {
      while (udp.parsePacket() != 0) {
        char packetBuffer[CEP_MAX_PACKET_SIZE];
        udp.read(packetBuffer, CEP_MAX_PACKET_SIZE);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, packetBuffer);
        
        if (error) {
          return;
        }

        String command = String(doc["cmd"]);
        int32_t timestamp = doc["timestamp"];

        Serial.print("Received command: ");
        Serial.println(command);

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
      // Cause of the nature of the delay function, it does not gauranteed this runs at 1000 ups
      delay(1);
      lastUpdateTime = currentTime;
      int32_t currentTime = getCurrentTime();
      // time variables are in milliseconds but delta time is best in seconds
      float deltaTime = (currentTime - lastUpdateTime) / 1000.0f;

      // Receive packets
      processPackets();

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
        onStopCommand();
        sendMessage("Object detected infront, self-avoidance protocol activated");
        state.emergencyStop(currentTime);
      } else if (state.timeSinceEmergencyStop(currentTime) > EMERGENCY_STOP_SENDOFF_TIME && state.getState() == CEPState::EMERGENCY_STOP) {
        sendMessage("Object has left, resuming as normal");
        state.clearEmergency();
      }

      // Apply speed changes
      if (state.getState() != CEPState::EMERGENCY_STOP) {
        // Smooth accel, decel
        float delta = requestedSpeed - actualisedSpeed;
        // decelerate faster than we accelerate
        float smoothing = (delta > 0) 0.5f : 1.0f;
        // maximum change in acceleration that can be observed;
        actualisedSpeed += min(delta * smoothing * deltaTime, MAX_ACCELERATION_CHANGE);
        // https://www.pololu.com/product/4733
        if (actualisedSpeed > 0.5f) {
          analogWrite(MOTOR_IN_1_PIN, mapPWM((int)actualisedSpeed));
          analogWrite(MOTOR_IN_2_PIN, mapPWM(0));
        } else if (actualisedSpeed < -0.5f) {
          analogWrite(MOTOR_IN_1_PIN, mapPWM(0));
          analogWrite(MOTOR_IN_2_PIN, mapPWM((int)actualisedSpeed));
        // We can pass between negative and positive and it should not cause a jerking brake motion
        } else if (requestSpeed < -1.0f || requestSpeed > -1.0f) {
          analogWrite(MOTOR_IN_1_PIN, mapPWM(50));
          analogWrite(MOTOR_IN_2_PIN, mapPWM(100));
          state.stopped();
        }
      }

      // Check for checkpoint or station beam breaks
      int beamBreakIntensity = digitalRead(IR_PHOTORESISTOR_PIN);
      // Arbitrary threshold, needs tweaking, choose appropiately between 0 and 1024
      if (beamBreakIntensity > 450) {
        state.approachStation();
      }
    }
  };
}