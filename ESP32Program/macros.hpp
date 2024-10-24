#define CEP_MAX_PACKET_SIZE 512
// In seconds
#define CEP_WIFI_TIMEOUT 5
// In milliseconds
#define CEP_HEARTBEAT_TIMEOUT 6000
// In milliseconds, time between each heartbeat
#define CEP_HEARTBEAT_DELAY 2000

// maximum change in acceleration that can be observed;
#define MAX_ACCELERATION_CHANGE 5.0f

// Time to wait after an obstruction is cleared to begin moving again
// In millseconds
#define EMERGENCY_STOP_SENDOFF_TIME 5000

// Percentage of motor power to approach the station
// Note that the maximum speed of the blade runner is about 300mm/s
#define STATION_APPROACH_SPEED 10
// Speed to reverse at if we overshoot the station
#define STATION_OVERSHOOT_REVERSE_SPEED -5

// value between 0 and 1024, where 0 is always pass, 1024 is maximum intensity to pass
#define IR_PHOTORESISTOR_SENSITIVITY 128

// Time to wait for boarding, in milliseconds
#define BOARDING_TIME 10000

// Pin locations for sensors
#define IR_SENSOR_PIN 25
#define SERVO_DATA_PIN 24
#define IR_PHOTORESISTOR_PIN 26
#define G_LED_PIN 11
#define Y_LED_PIN 12
#define R_LED_PIN 13
#define B_LED_PIN 14
#define MOTOR_IN_1_PIN 6
#define MOTOR_IN_2_PIN 7