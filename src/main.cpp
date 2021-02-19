#include <Arduino.h>
#include <Ctrl.h>
#include <TimerOne.h>

#define HOLL_INT 2
#define RELE_PIN 6
#define SMALL_BUTTON_PIN 7
#define MEDIUM_BUTTON_PIN 8
#define BIG_BUTTON_PIN 9
#define DIO 10
#define RCLK 11
#define SCLK 12
#define PRESCALER 4
#define CALC_PERIOD 1000000 // in microseconds
#define MINUTE 60000000     // in microseconds
/* Counter Settings */
#define SMALL_COUNTER 100
#define MEDIUM_COUNTER 250
#define BIG_COUNTER 500

ctrl::Button small_button(SMALL_BUTTON_PIN);
ctrl::Button medium_button(MEDIUM_BUTTON_PIN);
ctrl::Button big_button(BIG_BUTTON_PIN);
uint16_t ticks = 0;
uint16_t rpm = 0;
bool counter_mode_enabled = false;
int16_t counter = 0;

byte getBits(char);
void display(uint16_t);
void calc_ticks();
void calc_rpm();
void handle_click(ctrl::Button_Event *);

void setup() {
  pinMode(RCLK, OUTPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  pinMode(SMALL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MEDIUM_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BIG_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELE_PIN, OUTPUT);

  digitalWrite(RELE_PIN, LOW);

  Timer1.initialize(CALC_PERIOD);
  Timer1.attachInterrupt(calc_rpm);

  attachInterrupt(digitalPinToInterrupt(HOLL_INT), calc_ticks, RISING);

  small_button.on(ctrl::CLICK, handle_click);
  medium_button.on(ctrl::CLICK, handle_click);
  big_button.on(ctrl::CLICK, handle_click);
}

void loop() {
  small_button.listen();
  medium_button.listen();
  big_button.listen();

  display(rpm);
}

byte getBits(char val) {
  byte b;

  if (val != 'c') {
    val = toupper(val);
  }

  switch (val) {
  case '0':
    b = 0b11000000;
    break;
  case '1':
    b = 0b11111001;
    break;
  case '2':
    b = 0b10100100;
    break;
  case '3':
    b = 0b10110000;
    break;
  case '4':
    b = 0b10011001;
    break;
  case '5':
    b = 0b10010010;
    break;
  case '6':
    b = 0b10000010;
    break;
  case '7':
    b = 0b11111000;
    break;
  case '8':
    b = 0b10000000;
    break;
  case '9':
    b = 0b10010000;
    break;
  default:
    b = 0b11111111;
  }

  return b;
}

void display(uint16_t value) {
  const String str = String(value);
  const uint8_t digitsAmount = str.length();
  uint8_t counter = 0;
  uint8_t shift = digitsAmount - 1;

  while (counter < digitsAmount) {
    digitalWrite(RCLK, LOW);

    shiftOut(DIO, SCLK, MSBFIRST, getBits(str[counter++]));
    shiftOut(DIO, SCLK, MSBFIRST, 1 << shift--);

    digitalWrite(RCLK, HIGH);
  }
}

void calc_ticks() {
  ++ticks;

  if (counter_mode_enabled && --counter <= 0) {
    counter_mode_enabled = false;
    counter = 0;
    digitalWrite(RELE_PIN, LOW);
  }
}

/*
 * The func is executed of 0.1 sec. When the ticks is multiplied to 10, it is
 * rotation per second. When the result is multiplied to 60, it is rotation per
 * minute. PRESCALER says if there is more than one tick per rotation.
 */
void calc_rpm() {
  const uint16_t saved_ticks = ticks;

  rpm = saved_ticks * MINUTE / (PRESCALER * CALC_PERIOD);
  ticks = 0;
}

void handle_click(ctrl::Button_Event *event) {
  if (!counter_mode_enabled) {
    digitalWrite(RELE_PIN, HIGH);

    switch (event->target) {
    case SMALL_BUTTON_PIN:
      counter = SMALL_COUNTER * PRESCALER;
      break;
    case MEDIUM_BUTTON_PIN:
      counter = MEDIUM_COUNTER * PRESCALER;
      break;
    case BIG_BUTTON_PIN:
      counter = BIG_COUNTER * PRESCALER;
      break;
    }

    counter_mode_enabled = true;
  }
}
