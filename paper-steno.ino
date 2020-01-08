// -----------------------------------------------------------------------------
// Configuration

// Max length of chord sequence to auto-repeat.
#define MAX_REPEAT_LENGTH 4

// Hardware key matrix - choose one, comment out the others.
// #define JASON_4x6
// #define JOSH_5x5
// #define JOSH_4x6
#define STENOMOD  // (untested)

// random helper macro
#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

// -----------------------------------------------------------------------------
// TX Bolt byte/bit pairs: `ST` = star, `NN` = number bar, `XX` = none.

// Size of stroke representation (TX Bolt packet).
#define STROKE_BYTES 4
#define FOR_STROKE(i) for(byte i=0; i<STROKE_BYTES; ++i)

#define TX_BOLT(BYTE, BIT) (((BYTE)<<4) | (BIT))
#define TX_BYTE(TX) ((TX)>>4)
#define TX_BIT(TX) ((TX) & 0x7)

#define S_ TX_BOLT(0, 0)
#define T_ TX_BOLT(0, 1)
#define K_ TX_BOLT(0, 2)
#define P_ TX_BOLT(0, 3)
#define W_ TX_BOLT(0, 4)
#define H_ TX_BOLT(0, 5)
#define R_ TX_BOLT(1, 0)
#define  A TX_BOLT(1, 1)
#define  O TX_BOLT(1, 2)
#define ST TX_BOLT(1, 3)
#define  E TX_BOLT(1, 4)
#define  U TX_BOLT(1, 5)
#define _F TX_BOLT(2, 0)
#define _R TX_BOLT(2, 1)
#define _P TX_BOLT(2, 2)
#define _B TX_BOLT(2, 3)
#define _L TX_BOLT(2, 4)
#define _G TX_BOLT(2, 5)
#define _T TX_BOLT(3, 0)
#define _S TX_BOLT(3, 1)
#define _D TX_BOLT(3, 2)
#define _Z TX_BOLT(3, 3)
#define NN TX_BOLT(3, 4)
#define XX TX_BOLT(3, 5)


// -----------------------------------------------------------------------------
// Original 4x6 configuration

#ifdef JASON_4x6

#define COLUMNS 4
#define ROWS 6
const byte column_pins[COLUMNS] = {2, 3, 4, 5};
const byte row_pins[ROWS] = {6, 7, 8, 9, 10, 11};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
  // We use the top half of the `S-` key as the number "bar".
  // If you don't need it, you can change it back to another `S_`.
  {_P, _B, XX, ST, NN, S_},
  {_L, _G,  O,  A, H_, R_},
  {_T, _S,  U,  E, P_, W_},
  {_D, _Z, _R, _F, T_, K_}
};

#endif // JASON_4x6


// -----------------------------------------------------------------------------
// New 5x5 configuration

#ifdef JOSH_5x5

#define COLUMNS 5
#define ROWS 5
const byte column_pins[COLUMNS] = {2, 3, 4, 5, 6};
const byte row_pins[ROWS] = {7, 8, 9, 10, 11};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
  // We use the top half of the `S-` key as the number "bar".
  // If you don't need it, you can change it back to another `S_`.
  {NN, T_, P_, H_, ST},
  {S_, K_, W_, R_,  A},
  {XX, XX,  E,  U,  O},
  {_Z, _S, _G, _B, _R},
  {_D, _T, _L, _P, _F}
};

#endif // JOSH_5x5


// -----------------------------------------------------------------------------
// New 4x6 configuration

#ifdef JOSH_4x6

#define COLUMNS 4
#define ROWS 6
const byte column_pins[COLUMNS] = {2, 3, 4, 5};
const byte row_pins[ROWS] = {6, 7, 8, 9, 10, 11};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
  // We use the top half of the `S-` key as the number "bar".
  // If you don't need it, you can change it back to another `S_`.
  {S_, K_, W_, R_,  A, XX},
  {NN, T_, P_, H_,  O,  E},
  {_D, _T, _L, _P, _F, ST},
  {_Z, _S, _G, _B, _R,  U}
};

#endif // JOSH_4x6


// -----------------------------------------------------------------------------
// Stenomod? (untested)

#ifdef STENOMOD

#define COLUMNS 4
#define ROWS 6
const byte column_pins[COLUMNS] = {11, 10, 9, 8};
const byte row_pins[ROWS] = {14, 15, 16, 17, 18, 19};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
  {S_, T_, K_, P_, W_, H_},
  {R_,  A,  O, ST,  E,  U},
  {_F, _R, _P, _B, _L, _G},
  {_T, _S, _D, _Z, NN, S_}
};

#endif // STENOMOD


// -----------------------------------------------------------------------------
// Stroke history and auto-repeat

byte chords[2*MAX_REPEAT_LENGTH][STROKE_BYTES];

// Do we have a repeating sequence of length `n` that extends to length `m`?
bool has_repeat(byte n, byte m) {
  for(byte i=0; i<m-n; ++i) {
    FOR_STROKE(j) if(chords[i][j] != chords[i+n][j]) return false;
  }
  return true;
}

// Find the length of the repeat (if any).
// Can't just choose the longest one because we want 2 (not 4) for abababab.
// Can't just choose the shortest one because we want 3 (not 1) for baabaa.
byte repeat_length() {
  byte repeat = 0;
  for(byte i=MAX_REPEAT_LENGTH; i>0; --i) {
    // If there is a longer repeat, a shorter repeat has to
    // duplicate the entire longer one to be chosen.
    byte m = 2 * (repeat ? repeat : i);
    if(has_repeat(i, m)) repeat = i;
  }
  return repeat;
}


// -----------------------------------------------------------------------------
// Key Input and Debouncing

#define DEBOUNCE_MS 50
#define CHECK_MS 5
#define CHECKS (DEBOUNCE_MS/CHECK_MS)

void read_debounced_keys_into(byte *out) {
  // `static` makes these global but only visible within this function.
  static byte past_keys[CHECKS][STROKE_BYTES];
  static byte index = 0;

  // Get the next slot and clear it.
  byte *keys = past_keys[index];
  if(++index >= LENGTH(past_keys)) index = 0;
  FOR_STROKE(i) keys[i] = 0;

  // Read the keys into it.
  for(byte col=0; col<LENGTH(column_pins); ++col) {
    pinMode(column_pins[col], OUTPUT);
    digitalWrite(column_pins[col], LOW);  // drive column LOW
    delayMicroseconds(10);
    for(byte row=0; row<LENGTH(row_pins); ++row) {  // then check each row
      const byte info = key_info[col][row];
      const byte i = TX_BYTE(info), b = 1 << TX_BIT(info);
      if(digitalRead(row_pins[row]) == LOW) keys[i] |= b;
    }
    pinMode(column_pins[col], INPUT); // TODO - go HIGH output instead?
  }
  keys[3] &= 0x1f;

  // Debounce values and store in output array.
  out[0] = 0x3f;  out[1] = 0x3f;  out[2] = 0x3f;  out[3] = 0x1f;
  for(byte i=0; i<CHECKS; ++i) {
    FOR_STROKE(j) {
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
  FOR_STROKE(i) {
    if(keys[i]) {
      if(first && i > last_sent) Serial.write(0);
      Serial.write(keys[i] | (i << 6));
      first = false;  last_sent = i;
    }
  }
  if(first) { Serial.write(0); last_sent = 0; }
}


// -----------------------------------------------------------------------------
// Initialization and main loop.

static byte prev_keys[STROKE_BYTES];
static byte can_trigger[STROKE_BYTES];

void setup() {
  for(byte i=0; i<LENGTH(column_pins); ++i) pinMode(column_pins[i], INPUT);
  for(byte i=0; i<LENGTH(row_pins); ++i) pinMode(row_pins[i], INPUT_PULLUP);
  Serial.begin(9600);
  FOR_STROKE(i) {
    prev_keys[i] = 0;
    can_trigger[i] = ~0;
  }
}

void loop() {
  byte keys[STROKE_BYTES];
  read_debounced_keys_into(keys);

  // Were any keys released which can fire a chord?
  bool fire = false;
  FOR_STROKE(i) fire = fire || (prev_keys[i] & ~keys[i]) & can_trigger[i];
  if(fire) {
    send_keys(prev_keys);
    // These keys do not fire a chord the next time they are released.
    FOR_STROKE(i) can_trigger[i] = ~prev_keys[i];
  }

  FOR_STROKE(i) {
    prev_keys[i] = keys[i];
    // Keys which have been released can now fire a chord again.
    can_trigger[i] |= ~keys[i];
  }

  delay(CHECK_MS);
}
