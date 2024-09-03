#include <WiFi.h>
#include "credentials.hpp"
#include "CEP.hpp"

CEP::CEP cep;

void setup() {
  Serial.begin(115200);
  delay(10);

  cep.connect(CREDENTIALS_SSID, CCP_IP, CCP_PORT);
}

void loop() {
  cep.update();
}
