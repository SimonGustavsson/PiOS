#define GPIO_BASE 0x20200000

typedef enum {
	fsel_input = 0,  // 000b
	fsel_output = 1, // 001b
	fsel_alt0 = 4,   // 100b
	fsel_alt1 = 5,   // 101b
	fsel_alt2 = 6,   // 110b
	fsel_alt3 = 7,   // 101b
	fsel_alt4 = 3,   // 011b
	fsel_alt5 = 2,   // 010b
} fsel_type;

typedef union {
	unsigned int raw;
	struct {
		unsigned int fsel0 : 3;   // Function Select 0
		unsigned int fsel1 : 3;   // Function Select 1
		unsigned int fsel2 : 3;   // Function Select 2
		unsigned int fsel3 : 3;   // Function Select 3
		unsigned int fsel4 : 3;   // Function Select 4
		unsigned int fsel5 : 3;   // Function Select 5
		unsigned int fsel6 : 3;   // Function Select 6
		unsigned int fsel7 : 3;   // Function Select 7
		unsigned int fsel8 : 3;   // Function Select 8
		unsigned int fsel9 : 3;   // Function Select 9
		unsigned int rserved : 2; // -
	} bits;
} gpfsel;

typedef union {
	unsigned int raw;

	struct {
		unsigned int pin0 : 1;
		unsigned int pin1 : 1;
		unsigned int pin2 : 1;
		unsigned int pin3 : 1;
		unsigned int pin4 : 1;
		unsigned int pin5 : 1;
		unsigned int pin6 : 1;
		unsigned int pin7 : 1;
		unsigned int pin8 : 1;
		unsigned int pin9 : 1;
		unsigned int pin10 : 1;
		unsigned int pin11 : 1;
		unsigned int pin12 : 1;
		unsigned int pin13 : 1;
		unsigned int pin14 : 1;
		unsigned int pin15 : 1;
		unsigned int pin16 : 1;
		unsigned int pin17 : 1;
		unsigned int pin18 : 1;
		unsigned int pin19 : 1;
		unsigned int pin20 : 1;
		unsigned int pin21 : 1;
		unsigned int pin22 : 1;
		unsigned int pin23 : 1;
		unsigned int pin24 : 1;
		unsigned int pin25 : 1;
		unsigned int pin26 : 1;
		unsigned int pin27 : 1;
		unsigned int pin28 : 1;
		unsigned int pin29 : 1;
		unsigned int pin30 : 1;
		unsigned int pin31 : 1;
	} bits;
} gp_pin_reg0;

typedef union {
	unsigned int raw;

	struct {
		unsigned int pin32 : 1;
		unsigned int pin33 : 1;
		unsigned int pin34 : 1;
		unsigned int pin35 : 1;
		unsigned int pin36 : 1;
		unsigned int pin37 : 1;
		unsigned int pin38 : 1;
		unsigned int pin39 : 1;
		unsigned int pin40 : 1;
		unsigned int pin41 : 1;
		unsigned int pin42 : 1;
		unsigned int pin43 : 1;
		unsigned int pin44 : 1;
		unsigned int pin45 : 1;
		unsigned int pin46 : 1;
		unsigned int pin47 : 1;
		unsigned int pin48 : 1;
		unsigned int pin49 : 1;
		unsigned int pin50 : 1;
		unsigned int pin51 : 1;
		unsigned int pin52 : 1;
		unsigned int pin53 : 1;
		unsigned int reserved : 11;
	} bits;
} gp_pin_reg1;

typedef enum {
	pud_off = 0, // Off - Disable pull up/down
	pud_enable_pull_down = 1,
	pud_enable_pull_up = 2,
	pud_reserved = 3
} pud_state;

typedef union {
	unsigned int raw;
	
	struct {
		pud_state pud : 2;
		unsigned int reserved : 29;
	} bits;
} gppud_reg;

typedef volatile struct {
	gpfsel gpfsel0;                // GPIO Function Select 0
	gpfsel gpfsel1;                // GPIO Function Select 1
	gpfsel gpfsel2;                // GPIO Function Select 2
	gpfsel gpfsel3;                // GPIO Function Select 3
	gpfsel gpfsel4;                // GPIO Function Select 4
	unsigned int reserved;         // -
	gp_pin_reg0 gpset0;            // GPIO Pin Output Set 0
	gp_pin_reg1 gpset1;            // GPIO Pin Output Set 1
	unsigned int reserved2;        // -
	gp_pin_reg0 gpclr0;            // GPIO Pin Output clear 0 (Write-Only)
	gp_pin_reg1 gpclr1;            // GPIO Pin Output clear 1 (Write-Only)
	unsigned int reserved3;        // -
	const gp_pin_reg0 gplev0;      // GPIO Pin Level 0 (Read-only)
	const gp_pin_reg1 gplev1;      // GPIO Pin Level 1 (Read-only)
	unsigned int reserved4;        // -
	gp_pin_reg0 gpeds0;           // GPIO Pin Event Detect Status 0
	gp_pin_reg1 gpeds1;           // GPIO Pin Event Detect Status 1
	unsigned int reserved5;        // -
	gp_pin_reg0 gpren0;           // GPIO Pin Rising Edge Detect Enable 0
	gp_pin_reg1 gpren1;           // GPIO Pin Rising Edge Detect Enable 1
	unsigned int reserved6;        // -
	gp_pin_reg0 gpfen0;           // GPIO Pin Falling Edge Detect Enable 0
	gp_pin_reg1 gpfen1;           // GPIO Pin Falling Edge Detect Enable 1
	unsigned int reserved7;        // -
	gp_pin_reg0 gphen0;           // GPIO Pin High Detect Enable 0
	gp_pin_reg1 gphen1;           // GPIO Pin High Detect Enable 1
	unsigned int reserved8;        // -
	gp_pin_reg0 gplen0;           // GPIO Pin Low Detect Enable 0
	gp_pin_reg1 gplen1;           // GPIO Pin Low Detect Enable 1
	unsigned int reserved9;        // -
	gp_pin_reg0 gparen0;          // GPIO Pin Async. Rising Edge Detect 0
	gp_pin_reg1 gparen1;          // GPIO Pin Async. Rising Edge Detect 1
	unsigned int reserved10;       // -
	gppud_reg gppud;            // GPIO Pin Pull-up/Down Enable
	gp_pin_reg0 gppudclk0;        // GPIO Pin Pull-up/Down clock 0
	gp_pin_reg1 gppudclk1;        // GPIO Pin Pull-up/Down clock 1
	unsigned char reserved11[10];  // -
	unsigned char test;      // Test
} gpio_reg;

void LedInit(void);
void LedOn(void);
void LedOff(void);
