#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <TimerOne.h>

/*
 * Definitions
 */

#undef UART_DEBUG

#define PIN_UP       A0
#define PIN_DOWN     A1
#define PIN_LEFT     A2
#define PIN_RIGHT    A3
#define PIN_TURBO_A  A4
#define PIN_BUTTON_B A5
#define PIN_TURBO_B  A6
#define PIN_BUTTON_A A7

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
  pinMode(PIN_TURBO_A, INPUT_PULLUP);
  pinMode(PIN_TURBO_B, INPUT_PULLUP);  /* TODO: There is no pullup in MCU, we have to add it externally. */
  pinMode(PIN_BUTTON_A, INPUT_PULLUP); /* TODO: There is no pullup in MCU, we have to add it externally. */
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);

  randomSeed(analogRead(6)); /* TODO: Doesn't it block my ADC6 plans? I think yes. */

#ifdef UART_DEBUG
  Serial.begin(115200);
  Serial.println(F("CrazyCat v2 initialized"));
#endif

  // nRF init
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default
  radio.openWritingPipe(addresses[1]);

  Timer1.initialize(125000);
  Timer1.attachInterrupt(blinkLED);
}

/*
 * Main loop
 */

uint32_t tx_data = 0;
uint8_t command = 0;
uint8_t laser_on = 0;
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
#ifdef UART_DEBUG
      Serial.println(F("Up"));
#endif
      if (SERVO_Y_POS_MIN <= servo_y_pos - STEP) {
        servo_y_pos -= STEP;
      }
    }
    if (LOW == digitalRead(PIN_DOWN)) {
#ifdef UART_DEBUG
      Serial.println(F("Down"));
#endif
      if (SERVO_Y_POS_MAX >= servo_y_pos + STEP) {
        servo_y_pos += STEP;
      }
    }
    if (LOW == digitalRead(PIN_LEFT)) {
#ifdef UART_DEBUG
      Serial.println(F("Left"));
#endif
      if (SERVO_X_POS_MAX >= servo_x_pos + STEP) {
        servo_x_pos += STEP;
      }
    }
    if (LOW == digitalRead(PIN_RIGHT)) {
#ifdef UART_DEBUG
      Serial.println(F("Right"));
#endif
      if (SERVO_X_POS_MIN <= servo_x_pos - STEP) {
        servo_x_pos -= STEP;
      }
    }
    if (LOW == digitalRead(PIN_TURBO_A)) {
#ifdef UART_DEBUG
      Serial.println(F("Turbo A"));
#endif
      if (laser_on) {
        laser_on = 0;
      } else {
        laser_on = 1;
      }
    }
    /*if (LOW == digitalRead(PIN_TURBO_B)) {
#ifdef UART_DEBUG
      Serial.println(F("Turbo B"));
#endif
    }*/
    /*if (LOW == digitalRead(PIN_BUTTON_A)) {
#ifdef UART_DEBUG
      Serial.println(F("Button A"));
#endif
    }*/
    if (LOW == digitalRead(PIN_BUTTON_B)) {
#ifdef UART_DEBUG
      Serial.println(F("Button B"));
#endif
    }

    command = 1 | (laser_on << 1);
    //servo_x_pos = random(45);
    //servo_y_pos = random(40);

#ifdef UART_DEBUG
    Serial.print(F("x: "));
    Serial.print(servo_x_pos);
    Serial.print(F(" y: "));
    Serial.println(servo_y_pos);
#endif

    tx_data = 0;
    tx_data |= ((uint32_t)command) << 16;
    tx_data |= ((uint32_t)servo_x_pos) << 8;
    tx_data |= (uint32_t)servo_y_pos;

    if (!radio.write(&tx_data, sizeof(uint32_t))) {
#ifdef UART_DEBUG
      Serial.println(F("Sending failed"));
#endif
    }
  }
}

