#include <Arduino.h>

#define DEBOUNCE_TIMEOUT 50
#define LONG_KEYDOWN_TIMEOUT 750

#ifndef CTRL
#define CTRL

namespace ctrl {
struct Encoder_Event {
  bool positive_tick;
  bool negative_tick;
};

typedef void (*Encoder_Handler)(Encoder_Event const *event);

struct Encoder_Handlers {
  Encoder_Handler rotate;
};

class Encoder {
private:
  uint8_t stable_state, l_pin, r_pin, prev_unstable_state, unstable_signal;
  bool is_stable_state, is_new_unstable_state, tick, positive_tick,
      negative_tick;
  Encoder_Handlers handlers;

  uint8_t get_new_state();
  void reset_stable_state();
  void set_unstable_signal(uint8_t);
  void reset_tick();
  void set_tick();
  bool is_unstable_state(uint8_t);
  bool is_new_tick(uint8_t);
  bool is_same_tick(uint8_t);
  void read();

public:
  Encoder(uint8_t, uint8_t, uint8_t);

  void listen();
  void on(const char *event_name, Encoder_Handler);
};

struct Button_Event {
  uint8_t target;
};

typedef void (*Button_Handler)(Button_Event const *);

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
  Button(bool, uint8_t);

  void listen();
  void on(char const *, Button_Handler);
};
} // namespace ctrl

#endif
