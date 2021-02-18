#include "Ctrl.h"

using namespace ctrl;

Encoder::Encoder(uint8_t stable_state, uint8_t l_pin, uint8_t r_pin) {
  this->stable_state = stable_state;
  this->prev_unstable_state = stable_state;
  this->l_pin = l_pin;
  this->r_pin = r_pin;
  this->unstable_signal = 0;
  this->is_stable_state = true;
  this->is_new_unstable_state = false;
  this->tick = false;
  this->positive_tick = false;
  this->negative_tick = false;
}

void Encoder::listen() {
  static Encoder_Event event;

  this->read();

  if (this->tick) {
    event.positive_tick = this->positive_tick;
    event.negative_tick = this->negative_tick;

    if (this->handlers.rotate) {
      this->handlers.rotate(&event);
    }
  }
}

void Encoder::on(const char *event_name, Encoder_Handler callback) {
  if (strcmp(event_name, "rotate") == 0) {
    this->handlers.rotate = callback;

    return;
  }
}

uint8_t Encoder::get_new_state() {
  uint8_t state = 0;

  state = digitalRead(this->l_pin);
  state <<= 1;
  state += digitalRead(this->r_pin);

  return state;
}

void Encoder::reset_stable_state() {
  if (this->is_stable_state) {
    this->is_stable_state = false;
  }
}

void Encoder::set_unstable_signal(uint8_t state) {
  if (!this->is_stable_state && state != this->prev_unstable_state) {
    this->prev_unstable_state = state;
    this->unstable_signal <<= 2;
    this->unstable_signal += state;
  }
}

void Encoder::reset_tick() {
  this->tick = false;
  this->prev_unstable_state = this->stable_state;
  this->unstable_signal = 0;
  this->positive_tick = false;
  this->negative_tick = false;
}

void Encoder::set_tick() {
  this->tick = true;
  this->is_stable_state = true;

  if (this->unstable_signal == 0x12) {
    this->negative_tick = false;
    this->positive_tick = true;
  }

  if (this->unstable_signal == 0x21) {
    this->positive_tick = false;
    this->negative_tick = true;
  }
}

bool Encoder::is_unstable_state(uint8_t state) {
  return state != this->stable_state;
}

bool Encoder::is_new_tick(uint8_t state) {
  return !this->is_stable_state && state == this->stable_state;
}

bool Encoder::is_same_tick(uint8_t state) {
  return this->is_stable_state && state == this->stable_state && this->tick;
}

void Encoder::read() {
  const uint8_t state = this->get_new_state();

  if (this->is_unstable_state(state)) {
    this->reset_stable_state();
    this->set_unstable_signal(state);

    return;
  }

  if (this->is_same_tick(state)) {
    this->reset_tick();

    return;
  }

  if (this->is_new_tick(state)) {
    this->set_tick();

    return;
  }
}

Button::Button(bool stable_state, uint8_t pin) {
  this->pin = pin;
  this->event.target = pin;
  this->stable_state = stable_state;
  this->prev_state = stable_state;
  this->is_pressed = false;
}

void Button::listen() {
  static uint32_t last_timestamp = 0;
  const bool state = digitalRead(this->pin);
  uint32_t curr_timestamp = millis();
  bool is_debounce = (curr_timestamp - last_timestamp) < DEBOUNCE_TIMEOUT;

  if (is_debounce) {
    return;
  }

  if (state == this->prev_state) {
    this->handle_same_state(curr_timestamp - last_timestamp);

    return;
  }

  last_timestamp = curr_timestamp;
  this->prev_state = state;

  this->handle_new_state(state);
}

void Button::on(char const *event_name, Button_Handler handler) {
  if (strcmp(event_name, "keydown") == 0 || strcmp(event_name, "click") == 0) {
    this->handlers.keydown = handler;

    return;
  }

  if (strcmp(event_name, "longkeydown") == 0) {
    this->handlers.longkeydown = handler;

    return;
  }

  if (strcmp(event_name, "keyup") == 0) {
    this->handlers.keyup = handler;

    return;
  }
}

void Button::handle_same_state(uint32_t time) {
  bool is_longkeydown = time >= LONG_KEYDOWN_TIMEOUT;

  if (this->is_pressed && is_longkeydown && this->handlers.longkeydown) {
    this->handlers.longkeydown(&this->event);

    // its not semantic, but it should be cleared after longkeydown
    // but if it starts to interfere it shout be replaced by semantic variable
    this->is_pressed = false;
  }
}

void Button::handle_new_state(bool state) {
  if (state == this->stable_state) {
    this->is_pressed = false;

    if (this->handlers.keyup) {
      this->handlers.keyup(&this->event);
    }
  } else {
    this->is_pressed = true;

    if (this->handlers.keydown) {
      this->handlers.keydown(&this->event);
    }
  }
}
