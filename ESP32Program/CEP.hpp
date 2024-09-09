#pragma once
#include "ErrorCode.hpp"

#define CEP_WIFI_TIMEOUT 5

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