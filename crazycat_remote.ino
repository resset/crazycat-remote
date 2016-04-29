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

  randomSeed(analogRead(6));

  // nRF init
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default
  radio.openWritingPipe(addresses[1]);

  Serial.begin(115200);

  Timer1.initialize(2000000);
  Timer1.attachInterrupt(blinkLED);

  Serial.println(F("CrazyCat v2 initialized"));
}

/*
 * Main loop
 */

uint32_t tx_data = 0;
uint8_t command = 0;
uint8_t servo_x_pos = 0;
uint8_t servo_y_pos = 0;

void loop(void)
{
  if (timer_timeout) {
    timer_timeout = 0;

    command = 1;
    servo_x_pos = random(45);
    servo_y_pos = random(40);

    tx_data = 0;
    tx_data |= ((uint32_t)command) << 16;
    tx_data |= ((uint32_t)servo_x_pos) << 8;
    tx_data |= (uint32_t)servo_y_pos;

    if (!radio.write(&tx_data, sizeof(uint32_t))) {
      Serial.println(F("Sending failed"));
    }

    if (LOW == digitalRead(PIN_UP)) {
      Serial.println(F("Up"));
    }
    if (LOW == digitalRead(PIN_DOWN)) {
      Serial.println(F("Down"));
    }
    if (LOW == digitalRead(PIN_LEFT)) {
      Serial.println(F("Left"));
    }
    if (LOW == digitalRead(PIN_RIGHT)) {
      Serial.println(F("Right"));
    }
  }
}

