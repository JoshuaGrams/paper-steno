// -----------------------------------------------------------------------------
// Config etc.

// Hardware key matrix - choose one, comment out the others.
// #define JASON_4x6
// #define JOSH_5x5
// #define JOSH_4x6
#define STENOMOD

#define MAX_REPEAT 8

// Percent variation in chord timing that causes a sequence break.
#define BREAK_MARGIN 25

// Percent of chord delay to use for repeat (smaller is faster).
#define REPEAT_DELAY 67

#define DEBOUNCE_MS 50
#define CHECK_MS 5
#define CHECKS (DEBOUNCE_MS/CHECK_MS)


// -----------------------------------------------------------------------------
// Helper macro.

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

// -----------------------------------------------------------------------------
// TX Bolt byte/bit pairs: `ST` = star, `NN` = number bar, `XX` = none.

// Size of stroke representation (TX Bolt packet).
#define STROKE_BYTES 4

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
// Layouts

#ifdef JASON_4x6
// Paper Steno layout

#define COLUMNS 4
#define ROWS 6
byte column_pins[COLUMNS] = {2, 3, 4, 5};
byte row_pins[ROWS] = {6, 7, 8, 9, 10, 11};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
	// We use the top half of the `S-` key as the number "bar".
	// If you don't need it, you can change it back to another `S_`.
	{_P, _B, XX, ST, NN, S_},
	{_L, _G,  O,  A, H_, R_},
	{_T, _S,  U,  E, P_, W_},
	{_D, _Z, _R, _F, T_, K_}
};

#endif

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


#ifdef STENOMOD

#define COLUMNS 4
#define ROWS 6
byte column_pins[COLUMNS] = {11, 10, 9, 8};
byte row_pins[ROWS] = {14, 15, 16, 17, 18, 19};

// map (col, row) to (byte, bit) of TX Bolt stroke.
const byte key_info[COLUMNS][ROWS] = {
	{S_, T_, K_, P_, W_, H_},
	{R_,  A,  O, ST,  E,  U},
	{_F, _R, _P, _B, _L, _G},
	{_T, _S, _D, _Z, NN, S_}
};

#endif  // STENOMOD


// -----------------------------------------------------------------------------
// Key Input and Debouncing

void read_debounced_keys_into(byte *out) {
	static byte past_keys[CHECKS][STROKE_BYTES];
	static byte index = 0;

	// Read current keys.
	byte *keys = past_keys[index];
	if(++index >= LENGTH(past_keys)) index = 0;
	for(byte i=0; i<STROKE_BYTES; ++i) keys[i] = 0;
	for(byte col=0; col<LENGTH(column_pins); ++col) {
		pinMode(column_pins[col], OUTPUT);
		digitalWrite(column_pins[col], LOW);
		delayMicroseconds(10);
		for(byte row=0; row<LENGTH(row_pins); ++row) {
			const byte info = key_info[col][row];
			const byte i = TX_BYTE(info), b = 1 << TX_BIT(info);
			if(digitalRead(row_pins[row]) == LOW) keys[i] |= b;
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

boolean empty, changed, triggered;

void compare_keys(const byte *prev, const byte *cur, const byte *ignore) {
	empty = true; changed = false; triggered = false;
	for(byte i=0; i<STROKE_BYTES; ++i) {
		const byte different = prev[i] ^ cur[i];  // XOR is a bitwise not-equal.
		const byte released = different & prev[i];  // previously pressed.
		empty &= !cur[i];
		changed |= different;
		triggered |= released & ~ignore[i];
	}
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

boolean same_keys(byte *a, byte *b) {
	for(byte i=0; i<STROKE_BYTES; ++i) {
		if(a[i] != b[i]) return false;
	}
	return true;
}

// -----------------------------------------------------------------------------
// Repeat chord sequences.

byte chords[MAX_REPEAT][STROKE_BYTES];
unsigned int times[MAX_REPEAT];
byte first, count, last;  // Use chords/times as circular buffers.
byte repeat;
unsigned int per_repeat;

byte next(byte n) {
	n += first; if(n >= MAX_REPEAT) n -= MAX_REPEAT;
	return n;
}
void record(byte keys[STROKE_BYTES], unsigned int now) {
	if(count == MAX_REPEAT) { last = first;  first = next(1); }
	else { last = next(count);  ++count; }

	set_keys(chords[last], keys);
	times[last] = now;
}

void break_sequence(unsigned int last_press) {
	const unsigned int per_stroke = last_press - times[last];
	const unsigned int margin = per_stroke * BREAK_MARGIN / 100;
	for(byte i=count; i>0; --i) {
		byte c = next(i-1);
		const unsigned int dt = last_press - times[c];
		if(dt < per_stroke - margin || dt > per_stroke + margin) {
			first = next(i);  count = count - i;
			break;
		}
		last_press = times[c];
	}
}

boolean choose_loop(byte chord[STROKE_BYTES]) {
	for(byte i=0; i<count; ++i) {
		byte c = next(i);
		if(same_keys(chord, chords[c])) {
			first = c;  count = count - i;
			return true;
		}
	}
	return false;
}


// -----------------------------------------------------------------------------
// Initialization and main loop.

void setup() {
	for(byte i=0; i<LENGTH(column_pins); ++i) pinMode(column_pins[i], INPUT);
	for(byte i=0; i<LENGTH(row_pins); ++i) pinMode(row_pins[i], INPUT_PULLUP);
	first = 0; count = 0; last = 0;
	repeat = 0; per_repeat = 0;
	Serial.begin(9600);
}

void loop() {
	static byte keys[STROKE_BYTES], prev_keys[STROKE_BYTES];
	static byte ignore[STROKE_BYTES];
	static unsigned int last_change;

	const unsigned int now = millis();
	read_debounced_keys_into(keys);
	compare_keys(prev_keys, keys, ignore);

	if(triggered) {
		send_keys(prev_keys);
		record(prev_keys, last_change);
		set_keys(ignore, keys);
	} else remove_ups(ignore, keys);

	if(changed) {
		last_change = now; per_repeat = 0;
		set_keys(prev_keys, keys);
	} else if(!empty) {
		const unsigned int hold = now - last_change;
		const unsigned int threshold = last_change - times[last];
		if(per_repeat == 0 && count > 0 && hold >= threshold) {
			break_sequence(last_change);
			if(choose_loop(keys)) {
				repeat = 0;  per_repeat = threshold * REPEAT_DELAY / 100;
				set_keys(ignore, keys);
			}
		}
		if(per_repeat != 0 && hold >= per_repeat) {
			send_keys(chords[next(repeat)]);
			++repeat; if(repeat == count) repeat = 0;
			last_change = now;
		}
	}

	delay(CHECK_MS);
}
