MEMORY {
	flash (rx) : ORIGIN = 0x08000000, LENGTH = 128K
	ram   (rx) : ORIGIN = 0x20000000, LENGTH = 16K
}

SECTIONS {
	.interrupt_vectors : { *(.interrupt_vectors) } >flash
	.text : { *(.text) } >flash
	.data : { *(.data) } >ram
	.bss : { *(.bss) } >ram  /* Blank Space Section */
}

ENTRY(_start)
