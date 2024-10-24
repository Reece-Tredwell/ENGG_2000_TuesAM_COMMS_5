#pragma once
#include "ErrorCode.hpp"
#include <ArduinoJson.h>
#include <ESP32Servo.h>
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
    float motorSpeed;

    float duration_us; 
    float distance_cm;
    int32_t boardingStartTime;

    // ready is defined as having a connection and receiving the time command
    bool ready;
  public:
    CEP() {
      lastHeartbeatReceivedTime = 0;
      lastHeartbeatReceivedTime = 0;
      lastUpdateTime = 0;
      requestedSpeed = 0;
      boardingStartTime = 0;
      motorSpeed = 0;
      ready = false;
    }
    ErrorCode setup() {
      pinMode(IR_PHOTORESISTOR_PIN, INPUT);

      pinMode(G_LED_PIN, OUTPUT);
      pinMode(Y_LED_PIN, OUTPUT);
      pinMode(R_LED_PIN, OUTPUT);
      pinMode(B_LED_PIN, OUTPUT);

      pinMode(MOTOR_IN_1_PIN, OUTPUT);
      pinMode(MOTOR_IN_2_PIN, OUTPUT);

      pinMode(TRIG_PIN, OUTPUT);
      pinMode(ECHO_PIN, INPUT);

      doorServo.setPeriodHertz(50);
      doorServo.attach(SERVO_DATA_PIN, 500, 2400);
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
    ErrorCode onDoorCommand(bool doorState) {
      // Assume open
      if (doorState) {
        if (state.getState() == CEPState::STATION_ARRIVAL) {
          state.beginBoarding();
        }
        doorServo.write(180); //myServo.write(180);
      // Assume close
      } else {
        doorServo.write(0); //myServo.write(0);
      }
      return ErrorCode::SUCCESSFUL;
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
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onLocateCommand() {
      state.approachStation();
      requestedSpeed = STATION_APPROACH_SPEED;
      return ErrorCode::SUCCESSFUL;
    }
    ErrorCode onOvershotCommand() {
      state.overshotStation();
      requestedSpeed = STATION_OVERSHOOT_REVERSE_SPEED;
      return ErrorCode::SUCCESSFUL;
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
        } else if (command == "locate_station") {
          onLocateCommand();
        } else if (command == "overshot_station") {
          onOvershotCommand();
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
      int32_t currentTime = getCurrentTime();
      lastUpdateTime = currentTime;
      //int32_t currentTime = getCurrentTime();
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
      bool somethingIsInFrontOfUs = digitalRead(IR_SENSOR_PIN); // LOOK HERE ANAS
      if (somethingIsInFrontOfUs) {
        onStopCommand();
        sendMessage("Object detected infront, self-avoidance protocol activated");
        state.emergencyStop(currentTime);
        requestedSpeed = 0.0f;
      } else if (state.timeSinceEmergencyStop(currentTime) > EMERGENCY_STOP_SENDOFF_TIME && state.getState() == CEPState::EMERGENCY_STOP) {
        sendMessage("Object has left, resuming as normal");
        state.clearEmergency();
      }

      // Apply speed changes
      if (state.getState() != CEPState::EMERGENCY_STOP) {
        // Smooth acceleration and deceleration
        float delta = requestedSpeed - motorSpeed;
        // Decelerate faster than acceleration
        float smoothing = (delta > 0) ? 0.5f : 1.0f;
        // Limit the maximum change in acceleration
        float acceleration = delta * smoothing * deltaTime;
        acceleration = constrain(acceleration, -MAX_ACCELERATION_CHANGE, MAX_ACCELERATION_CHANGE);
        motorSpeed += acceleration;
        bool accelerating = (delta * motorSpeed >= 0);

        // Handle motor control based on actualised speed
        if (fabs(motorSpeed) > SPEED_THRESHOLD) {
          state.beginMoving();
            if (motorSpeed > 0) {
                // Forward motion
                if (accelerating) {
                    analogWrite(MOTOR_IN_1_PIN, mapPWM((int)motorSpeed));
                    analogWrite(MOTOR_IN_2_PIN, mapPWM(0));
                } else {
                    int brakePWM = MAX_PWM - (int)motorSpeed;
                    analogWrite(MOTOR_IN_1_PIN, mapPWM(MAX_PWM));
                    analogWrite(MOTOR_IN_2_PIN, mapPWM(brakePWM));
                }
            } else {
                // Reverse motion
                if (accelerating) {
                    analogWrite(MOTOR_IN_1_PIN, mapPWM(0));
                    analogWrite(MOTOR_IN_2_PIN, mapPWM((int)(-motorSpeed)));
                } else {
                    int brakePWM = MAX_PWM + (int)motorSpeed; // motorSpeed is negative
                    analogWrite(MOTOR_IN_1_PIN, mapPWM(brakePWM));
                    analogWrite(MOTOR_IN_2_PIN, mapPWM(MAX_PWM));
                }
            }
        } else {
            // Keep a small braking force
            analogWrite(MOTOR_IN_1_PIN, mapPWM(1));
            analogWrite(MOTOR_IN_2_PIN, mapPWM(1));
            state.stopped();
        }
      }

      // Station locating code
      if (state.getState() == CEPState::APPROACHING_STATION) {
        // We've already slowed down at this point
        // Check for station IR LED
        int ledIntensity = digitalRead(IR_PHOTORESISTOR_PIN); //LOOK HERE ANAS
        if (ledIntensity > IR_PHOTORESISTOR_SENSITIVITY) {
          onStopCommand();
          state.arriveAtStation();
        }
      }
      
      if (state.getState() == CEPState::OVERSHOT_STATION) {
        sendMessage("FUCK me, I overshot the station");
      }

      if (state.getState() == CEPState::STATION_ARRIVAL) {
        boardingStartTime = currentTime;
        sendMessage("Arrived at station, waiting for boarding commands");
      }

      // Boarding procedure
      if (state.getState() == CEPState::BOARDING) {
        // Doors should be open by this point
      }

      if (state.getState() == CEPState::STATION_DEPARTURE) {
        // Doors should be closed
        sendMessage("Lets get out of here mfs");
        state.proceedAfterDeparture();
      }

      // 
    }
  };
}