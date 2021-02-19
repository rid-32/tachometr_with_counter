#include <Arduino.h>

#define DEBOUNCE_TIMEOUT 50
#define LONG_KEYDOWN_TIMEOUT 750

#ifndef CTRL
#define CTRL

namespace ctrl {
enum Event_Types { ROTATE, KEYDOWN, CLICK, KEYUP, LONG_KEYDOWN };

struct Encoder_Event {
  uint8_t target;
  bool positive_tick;
  bool negative_tick;
};

typedef void (*Encoder_Handler)(Encoder_Event *event);

struct Encoder_Handlers {
  Encoder_Handler rotate;
};

class Encoder {
private:
  uint8_t l_pin, r_pin, target, prev_unstable_state, unstable_state;
  bool positive_tick, negative_tick;
  Encoder_Handlers handlers;

  uint8_t get_new_state();
  void set_unstable_state(uint8_t);
  void reset_tick();
  void set_tick(uint8_t);
  bool is_stable_state(uint8_t);
  bool is_new_tick();
  bool read();
  void set_target();

public:
  Encoder(uint8_t, uint8_t);

  void listen();
  void on(Event_Types, Encoder_Handler);
};

struct Button_Event {
  uint8_t target;
};

typedef void (*Button_Handler)(Button_Event *);

struct Button_Handlers {
  Button_Handler keydown;
  Button_Handler longkeydown;
  Button_Handler keyup;
};

class Button {
private:
  uint8_t pin;
  bool stable_state, prev_state, is_pressed;
  Button_Handlers handlers;
  Button_Event event;

  void handle_same_state(uint32_t);
  void handle_new_state(bool);

public:
  Button(uint8_t);

  void listen();
  void on(Event_Types, Button_Handler);
};
} // namespace ctrl

#endif
