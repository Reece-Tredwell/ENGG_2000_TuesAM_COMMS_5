#include <WiFi.h>
#include "credentials.hpp"
#include "CEP.hpp"


CEP::CEP cep;

float duration_us; 
float distance_cm;
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Program");
  delay(10);
  pinMode(TRIG_PIN, OUTPUT);
  // configure the echo pin to input mode
  pinMode(ECHO_PIN, INPUT);

  if (cep.connect(CREDENTIALS_SSID, CCP_IP, CCP_PORT) != CEP::ErrorCode::SUCCESSFUL)
    exit(-1);

}

void loop() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance_cm = 0.017 * duration_us;
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  cep.processPackets();
}
