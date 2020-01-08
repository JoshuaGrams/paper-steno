/* For STM32F0 Discovery board. */

#define GPIO_MODE      0x00
#define GPIO_OUT_MODE  0x04
#define GPIO_SPEED     0x08
#define GPIO_PULL      0x0C
#define GPIO_IN        0x10
#define GPIO_OUT       0x14
#define GPIO_SET_RESET 0x18
#define GPIO_LOCK_CFG  0x1C
#define GPIO_ALT_FN    0x20

#define GPIOA 0x48000000
#define GPIOB 0x48000400
#define GPIOC 0x48000800


#define RCC_PERIPHERALS 0x14
#define PORTC_ENABLE (1<<19)

#define RCC 0x40021000

.section .interrupt_vectors

	.word 0x20000000 + 16*1024  @ initial stack pointer
	.word _start
	.rept 14
	.word spinlock              @ interrupt vectors
	.endr


	.text

#define BIT 7

spinlock:
	b spinlock

	.global _start
_start:
	mov r0,#0
0:	add r0,#1
	b 0b

	@ Leave clock at default (8 MHz?).

	@ Activate port C clock.
	ldr r0,rcc
	ldr r1,port_clock_enable
	ldr r2,[r0,#RCC_PERIPHERALS]
	orr r2,r1
	str r2,[r0,#RCC_PERIPHERALS]

	@ Set an I/O pin (PC6-9 are LEDs) to push-pull output.
		@ Default is floating input.
		@ - Set Mode to 1 (output).
	ldr r3,gpio_port
	ldr r4,[r3,#GPIO_MODE]
	ldr r5,mode_mask
	ldr r6,mode
	and r4,r4,r5
	orr r4,r4,r6
	str r4,[r3,#GPIO_MODE]

	@ Initialize LED state to 0
	ldr r0,led_state_ptr
	mov r1,#0
	str r1,[r0]

0:
	@ Toggle the LED state
	ldr r0,led_state_ptr
	ldr r1,[r0]
	mov r2,#1
	sub r1,r2,r1
	str r1,[r0]

	@ Send the current state
	ldr r0,gpio_port
	ldr r2,[r0,#GPIO_OUT]
	mov r3,#1<<BIT
	mvn r3,r3
	and r2,r2,r3
	lsl r1,#BIT
	orr r2,r2,r1
	str r2,[r0,#GPIO_OUT]

	ldr r0,delay_count
1:  sub r0,#1
	bne 1b

	b 0b

	.align
delay_count: .word 1000000
led_state_ptr: .word led_state
gpio_port: .word GPIOC
mode_mask: .word ~(0x3 << (2*BIT))
mode: .word 0x1 << (2*BIT)

rcc: .word RCC
port_clock_enable: .word PORTC_ENABLE

	.bss
led_state: .word 0
