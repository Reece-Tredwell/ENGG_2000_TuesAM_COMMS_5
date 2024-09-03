#pragma once
#include "ErrorCode.hpp"

#define CEP_WIFI_CONNECT_ATTEMPTS 5

namespace CEP {
  class CEP {
  private:
    IPAddress local;
    WiFiUDP udp;
    IPAddress remoteIP;
    int remotePort;
  public:
    CEP() {

    }

    ErrorCode connect(const char* ssid, const IPAddress& rip, int rp) {
      remoteIP = rip;
      remotePort = rp;

      Serial.print("Connecting to ");
      Serial.println(ssid);

      WiFi.begin(ssid);

      uint32_t attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < CEP_WIFI_CONNECT_ATTEMPTS) {
        delay(1000);
        attempts++
      }

      if (attempts >= CEP_WIFI_CONNECT_ATTEMPTS) {
        Serial.print("Failed to connect in ");
        Serial.print(attempts);
        Serial.println(" attempts");
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
    void update() {
      udp.beginPacket(remoteIP, remotePort);
      udp.print("Hello!");
      udp.endPacket();

      int packetSize = udp.parsePacket();

      if (packetSize) {
        
      }
    }
  };
}