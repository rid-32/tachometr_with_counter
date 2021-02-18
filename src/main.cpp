#include <Arduino.h>
#include <TimerOne.h>

#define HOLL_INT 2
#define DIO 10
#define RCLK 11
#define SCLK 12
#define PRESCALER 4
#define CALC_PERIOD 1500000 // in microseconds
#define MINUTE 60000000     // in microseconds

uint16_t ticks = 0;
uint16_t rpm = 0;

byte getBits(char);
void display(uint16_t);
void calc_ticks();
void calc_rpm();

void setup() {
  Serial.begin(115200);
  pinMode(RCLK, OUTPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(DIO, OUTPUT);

  Timer1.initialize(CALC_PERIOD);
  Timer1.attachInterrupt(calc_rpm);

  attachInterrupt(digitalPinToInterrupt(HOLL_INT), calc_ticks, RISING);
}

void loop() { display(rpm); }

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
  Serial.println(ticks);
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
