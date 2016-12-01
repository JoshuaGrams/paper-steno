// -----------------------------------------------------------------------------
// Config etc.

#define COLUMNS 4
#define ROWS 6
byte column_pins[COLUMNS] = {2, 3, 4, 5};
byte row_pins[ROWS] = {6, 7, 8, 9, 10, 11};

#define DEBOUNCE_MS 50
#define CHECK_MS 5
#define CHECKS (DEBOUNCE_MS/CHECK_MS)

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

// Size of stroke representation/TX Bolt packet.
#define STROKE_BYTES 4


// -----------------------------------------------------------------------------
// TX Bolt byte/bit pairs: `ST` = star, `NN` = number bar, `XX` = none.

#define S_ {0, 0}
#define T_ {0, 1}
#define K_ {0, 2}
#define P_ {0, 3}
#define W_ {0, 4}
#define H_ {0, 5}
#define R_ {1, 0}
#define  A {1, 1}
#define  O {1, 2}
#define ST {1, 3}
#define  E {1, 4}
#define  U {1, 5}
#define _F {2, 0}
#define _R {2, 1}
#define _P {2, 2}
#define _B {2, 3}
#define _L {2, 4}
#define _G {2, 5}
#define _T {3, 0}
#define _S {3, 1}
#define _D {3, 2}
#define _Z {3, 3}
#define NN {3, 4}
#define XX {3, 5}


// -----------------------------------------------------------------------------
// Key Input and Debouncing

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS][2] = {
  // We use the top half of the `S-` key as the number "bar".
  // If you don't need it, you can change it back to another `lS`.
  {_P, _B, XX, ST, NN, S_},
  {_L, _G,  O,  A, H_, R_},
  {_T, _S,  U,  E, P_, W_},
  {_D, _Z, _R, _F, T_, K_}
};

void read_debounced_keys_into(byte *out) {
  static byte past_keys[CHECKS][STROKE_BYTES];
  static byte index = 0;

  // Read current keys.
  byte *keys = past_keys[index];
  if(++index >= LENGTH(past_keys)) index = 0;
  for(byte col=0; col<LENGTH(column_pins); ++col) {
    pinMode(column_pins[col], OUTPUT);
    digitalWrite(column_pins[col], LOW);
    delayMicroseconds(10);
    for(byte row=0; row<LENGTH(row_pins); ++row) {
      const byte *info = key_info[col][row];
      byte i = info[0], b = info[1], m = 1 << b;
      keys[i] = (keys[i] | m) & ~(digitalRead(row_pins[row]) << b);
    }
    pinMode(column_pins[col], INPUT);
  }
  keys[3] &= 0x1f;

  // Return debounced values.
  out[0] = 0x3f;  out[1] = 0x3f;  out[2] = 0x3f;  out[3] = 0x1f;
  for(byte i=0; i<CHECKS; ++i) {
    for(byte j=0; j<STROKE_BYTES; ++j) {
      // Clear keys that are off in any of the checks.
      out[j] &= past_keys[i][j];
    }
  }
}

// -----------------------------------------------------------------------------
// TX Bolt output

void send_keys(byte *keys) {
  static byte last_sent = 0;
  boolean first = true;
  for(byte i=0; i<STROKE_BYTES; ++i) {
    if(keys[i]) {
      if(first && i > last_sent) Serial.write(0);
      Serial.write(keys[i] | (i << 6));
      first = false;  last_sent = i;
    }
  }
  if(first) { Serial.write(0); last_sent = 0; }
}

// -----------------------------------------------------------------------------
// Stroke helper functions

boolean any_released(const byte *prev, const byte *cur, byte *ignore) {
  for(byte i=0; i<STROKE_BYTES; ++i) {
    byte changed = (prev[i] ^ cur[i]) & ~ignore[i];
    if(prev[i] & changed) return true;  // the ones that were down (now up)
  }
  return false;
}

void remove_ups(byte *ignore, const byte *cur) {
  for(byte i=0; i<STROKE_BYTES; ++i)  ignore[i] &= cur[i];
}

void set_keys(byte *dst, const byte *src) {
  for(byte i=0; i<STROKE_BYTES; ++i)  dst[i] = src[i];
}

void clear_keys(byte *keys) {
  for(byte i=0; i<STROKE_BYTES; ++i)  keys[i] = 0;
}

// -----------------------------------------------------------------------------
// Initialization and main loop.

void setup() {
  for(byte i=0; i<LENGTH(column_pins); ++i) pinMode(column_pins[i], INPUT);
  for(byte i=0; i<LENGTH(row_pins); ++i) pinMode(row_pins[i], INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  static byte keys[STROKE_BYTES], prev_keys[STROKE_BYTES];
  static byte ignore[STROKE_BYTES];
  read_debounced_keys_into(keys);
  if(any_released(prev_keys, keys, ignore)) {
    send_keys(prev_keys);
    set_keys(ignore, keys);
  } else remove_ups(ignore, keys);
  set_keys(prev_keys, keys);
  delay(CHECK_MS);
}
