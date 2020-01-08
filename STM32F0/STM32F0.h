typedef struct GPIO GPIO;

/* page 46 of STM's RM0091 (Reference Manual for STM332F0xx) */
#define GPIOA ((GPIO*)0x48000000)
#define GPIOB ((GPIO*)0x48000400)
#define GPIOC ((GPIO*)0x48000800)

struct GPIO {
	// Each of these has a 1- or 2-bit field per bit of the I/O port.
	// The ones with 1-bit fields use only the low 16 bits.
	unsigned mode;      // in (default), out, alt-function, analog.
	unsigned out_mode;  // push-pull (default) or open-drain.
	unsigned speed;     // x0: low, 01: medium, 11: high.
	unsigned pull;      // none, pull-up, pull-down, reserved.
	unsigned input;     // read data from port *in* to CPU.
	unsigned output;    // write data *out* from CPU to port.
	unsigned set_reset; // write LSB16 to set an output bit, HSB16 to clear.
	unsigned lock_cfg;  // lock configuration of specific port bits.
		// successively write 1, 0, 1 to bit 16 then read twice (should
		// see a 1 the second time) to modify the lock.
	unsigned alt_fn[2]; // select 16 alternate functions for each bit.
	unsigned reset;     // write 1 to reset a bit. redundant??
};
