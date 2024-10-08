#include <WiFi.h>
#include "credentials.hpp"
#include "CEP.hpp"

CEP::CEP cep;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Program");
  delay(10);

  if (cep.connect(CREDENTIALS_SSID, CCP_IP, CCP_PORT) != CEP::ErrorCode::SUCCESSFUL)
    exit(-1);

}

void loop() {
  cep.processPackets();
}
