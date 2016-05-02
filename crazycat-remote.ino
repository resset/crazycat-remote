#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <TimerOne.h>

/*
 * Definitions
 */

#define PIN_UP    A0
#define PIN_DOWN  A1
#define PIN_LEFT  A2
#define PIN_RIGHT A3
#define PIN_LASER 10

RF24 radio(7, 8);
byte addresses[][6] = {"1Node","2Node"};

/*
 * Timer
 */

volatile uint8_t timer_timeout;

void blinkLED(void)
{
  timer_timeout = 1;
}

/*
 * Setup
 */

void setup(void)
{
  // Arrow keys
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_LASER, INPUT_PULLUP);

  randomSeed(analogRead(6));

  // nRF init
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default
  radio.openWritingPipe(addresses[1]);

  Serial.begin(115200);

  Timer1.initialize(125000);
  Timer1.attachInterrupt(blinkLED);

  Serial.println(F("CrazyCat v2 initialized"));
}

/*
 * Main loop
 */

uint32_t tx_data = 0;
uint8_t command = 0;
uint8_t laser_on = 1;
uint8_t servo_x_pos = 90;
uint8_t servo_y_pos = 90;
const uint8_t SERVO_X_POS_MAX = 180;
const uint8_t SERVO_X_POS_MIN = 0;
const uint8_t SERVO_Y_POS_MAX = 180;
const uint8_t SERVO_Y_POS_MIN = 0;
const uint8_t STEP = 2;

void loop(void)
{
  if (timer_timeout) {
    timer_timeout = 0;

    if (LOW == digitalRead(PIN_UP)) {
      Serial.println(F("Up"));
      if (SERVO_Y_POS_MIN <= servo_y_pos - STEP) {
        servo_y_pos -= STEP;
      }
    }
    if (LOW == digitalRead(PIN_DOWN)) {
      Serial.println(F("Down"));
      if (SERVO_Y_POS_MAX >= servo_y_pos + STEP) {
        servo_y_pos += STEP;
      }
    }
    if (LOW == digitalRead(PIN_LEFT)) {
      Serial.println(F("Left"));
      if (SERVO_X_POS_MAX >= servo_x_pos + STEP) {
        servo_x_pos += STEP;
      }
    }
    if (LOW == digitalRead(PIN_RIGHT)) {
      Serial.println(F("Right"));
      if (SERVO_X_POS_MIN <= servo_x_pos - STEP) {
        servo_x_pos -= STEP;
      }
    }
    if (LOW == digitalRead(PIN_LASER)) {
      Serial.println(F("Laser"));
      if (laser_on) {
        laser_on = 0;
      } else {
        laser_on = 1;
      }
    }

    command = 1 | (laser_on << 1);
    //servo_x_pos = random(45);
    //servo_y_pos = random(40);
    Serial.print(F("x: "));
    Serial.print(servo_x_pos);
    Serial.print(F(" y: "));
    Serial.println(servo_y_pos);

    tx_data = 0;
    tx_data |= ((uint32_t)command) << 16;
    tx_data |= ((uint32_t)servo_x_pos) << 8;
    tx_data |= (uint32_t)servo_y_pos;

    if (!radio.write(&tx_data, sizeof(uint32_t))) {
      Serial.println(F("Sending failed"));
    }
  }
}

