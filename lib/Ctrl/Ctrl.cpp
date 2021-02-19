#include "Ctrl.h"

using namespace ctrl;

Encoder::Encoder(uint8_t l_pin, uint8_t r_pin) {
  pinMode(l_pin, INPUT_PULLUP);
  pinMode(r_pin, INPUT_PULLUP);

  this->l_pin = l_pin;
  this->r_pin = r_pin;
  this->prev_unstable_state = 0;
  this->unstable_state = 0;
  this->positive_tick = false;
  this->negative_tick = false;

  this->set_target();
}

void Encoder::listen() {
  static Encoder_Event event;

  const bool is_new_tick = this->read();

  if (is_new_tick) {
    event.positive_tick = this->positive_tick;
    event.negative_tick = this->negative_tick;
    event.target = this->target;

    if (this->handlers.rotate) {
      this->handlers.rotate(&event);
    }

    this->reset_tick();
  }
}

void Encoder::on(Event_Types event_type, Encoder_Handler callback) {
  if (event_type == ROTATE) {
    this->handlers.rotate = callback;

    return;
  }
}

void Encoder::set_target() {
  uint8_t target = this->l_pin;
  target <<= 4;
  target += this->r_pin;

  this->target = target;
}

uint8_t Encoder::get_new_state() {
  uint8_t state = 0;

  state = digitalRead(this->l_pin);
  state <<= 1;
  state += digitalRead(this->r_pin);

  return state;
}

void Encoder::set_unstable_state(uint8_t state) {
  if (state != this->prev_unstable_state) {
    this->prev_unstable_state = state;
    this->unstable_state <<= 2;
    this->unstable_state += state;
  }
}

void Encoder::reset_tick() {
  this->prev_unstable_state = 0;
  this->unstable_state = 0;
  this->positive_tick = false;
  this->negative_tick = false;
}

void Encoder::set_tick(uint8_t state) {
  const bool is_positive_tick =
      this->unstable_state == 0x12 ||
      (state == 0x00 && this->unstable_state == 0x01) ||
      (state == 0x03 && this->unstable_state == 0x02);
  const bool is_negative_tick =
      this->unstable_state == 0x21 ||
      (state == 0x00 && this->unstable_state == 0x02) ||
      (state == 0x03 && this->unstable_state == 0x01);

  if (is_positive_tick) {
    this->negative_tick = false;
    this->positive_tick = true;
  }

  if (is_negative_tick) {
    this->positive_tick = false;
    this->negative_tick = true;
  }
}

bool Encoder::is_stable_state(uint8_t state) {
  return state == 0x00 || state == 0x03;
}

bool Encoder::is_new_tick() { return (bool)this->unstable_state; }

bool Encoder::read() {
  const uint8_t state = this->get_new_state();

  if (!this->is_stable_state(state)) {
    this->set_unstable_state(state);

    return false;
  }

  if (this->is_new_tick()) {
    this->set_tick(state);

    return true;
  }

  return false;
}

Button::Button(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);

  this->pin = pin;
  this->event.target = pin;
  this->stable_state = digitalRead(pin);
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

void Button::on(Event_Types event_type, Button_Handler handler) {
  if (event_type == KEYDOWN || event_type == CLICK) {
    this->handlers.keydown = handler;

    return;
  }

  if (event_type == LONG_KEYDOWN) {
    this->handlers.longkeydown = handler;

    return;
  }

  if (event_type == KEYUP) {
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
