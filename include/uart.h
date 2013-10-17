#define UART_BASE 0x20215000

typedef union {
	unsigned int raw;
	
	struct {
		unsigned int mini_uart : 1;
		unsigned int spi1 : 1;
		unsigned int spi2 : 1;
		unsigned int reserved : 29;
	} bits;
} aux_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int data : 8;
		unsigned int reserved : 24;
	} bits;
} aux_mu_io_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int enable_transmit_interrupt : 1;
		unsigned int enable_receive_interrupt : 1;
		unsigned int reserved : 30;
	} bits;
} aux_mu_iir_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int interrupt_pending : 1;
		unsigned int interrupt_id_fifo_clear : 2; // On write: bit 1 will clear receive FIFO and bit 2 will clear. On read 0 = no interrupt, 1 = transmit holding registry empty, 2 = Receiver holds valid byte
		unsigned int timeout : 1; // NOTE: Always 0 as mini uart has no timeout function

	} bits;
} aux_mu_ier_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int data_size : 1; // 1 = 8-bit, 0 = 7-bit
		unsigned int reserved : 5;
		unsigned int break_field : 1; //
		unsigned int dlab_access : 1;
		unsigned int reserved2 : 24;
	} bits;
} uax_mu_lcr_reg;

typedef union {
	unsigned int raw;
	
	struct {
		unsigned int reserved : 1;
		unsigned int rts : 1;
		unsigned int reserved2 : 30;
	} bits;
} uax_mu_mcr_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int data_read : 1;
		unsigned int receiver_overrun : 1;
		unsigned int reserved : 3;
		unsigned int transmitter_empty : 1;
		unsigned int transmitter_idle : 1;
		unsigned int reserved2: 25;
	} bits;
} aux_mu_lsr_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int reserved : 4;
		unsigned int cts_status : 1;
		unsigned int reserved2 : 27;
	} bits;
} aux_mu_msr_reg;

typedef union { 
	unsigned int raw;

	struct {
		unsigned int scratch : 8;
		unsigned int reserved : 24;
	} bits;
} aux_mu_scratch;

typedef union {
	unsigned int raw;

	struct {
		unsigned int receiver_enabled : 1;
		unsigned int transmitter_enabled : 1;
		unsigned int enable_receive_auto_flow : 1;
		unsigned int enable_transmit_auto_flow : 1;
		unsigned int rts_auto_flow_level : 2; // 0 = 3, 1 = 2, 2 = 1, 3 = 4
		unsigned int rts_assert_level : 1;
		unsigned int cts_assert_level : 1;
		unsigned int reserved : 24;
	} bits;
} aux_mu_cntl_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int symbol_available : 1;
		unsigned int space_available : 1;
		unsigned int receiver_idle : 1;
		unsigned int transmitter_idle : 1;
		unsigned int receiver_overrun : 1;
		unsigned int transmit_fifo_full : 1;
		unsigned int rts_status : 1;
		unsigned int cts_status : 1;
		unsigned int transmit_fifo_empty : 1;
		unsigned int transmitter_done : 1;
		unsigned int reserved : 6;
		unsigned int receive_fifo_fill_level : 4;
		unsigned int reserved2 : 4;
		unsigned int transmit_fifo_fill_level : 4;
		unsigned int reserved3 : 4;
	} bits;
} aux_mu_stat_reg;

typedef union {
	unsigned int raw;
	struct {
		unsigned int baud_rate : 16; // Mini Uart baudrate counter
		unsigned int reserved : 16;
	} bits;
} aux_mu_baud_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int shift_length : 6;
		unsigned int shift_out_ms : 1;
		unsigned int invert_spi_clk : 1;
		unsigned int out_rising : 1;
		unsigned int clear_fifos : 1;
		unsigned int in_rising : 1;
		unsigned int enable : 1;
		unsigned int dout_hold_time : 2;
		unsigned int variable_width : 1;
		unsigned int variable_cs : 1;
		unsigned int post_input_mode : 1;
		unsigned int chip_selects : 3;
		unsigned int speed : 12;
	} bits;
} aux_spi_cntl0_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int keep_input : 1;
		unsigned int shift_in_ms_bit : 1;
		unsigned int reserved : 4;
		unsigned int done_irq : 1;
		unsigned int tx_empty_irq : 1;
		unsigned int cs_high_time : 3;
		unsigned int reserved2 : 20;
	} bits;
} aux_spi_cntl1_reg;

typedef union {
	unsigned int raw;

	struct {
		unsigned int bit_count : 6;
		unsigned int busy : 1;
		unsigned int reserved : 5;
		unsigned int rx_fifo_level : 12;
		unsigned int tx_fifo_level : 7;
	} bits;
} aux_spi_stat;

typedef union {
	unsigned int raw;

	struct {
		unsigned int data : 16;
		unsigned int reserved : 16;
	} bits;
} aux_spi_peek;

typedef union {
	unsigned int raw;

	struct {
		unsigned int data : 16;
		unsigned int reserved : 16;
	} bits;
} aux_spi_io;

typedef volatile struct { // Located at 0x20215000
	aux_reg irq;                   // Auxiliary Interrupt status
	aux_reg enables;               // Auxiliary enables
	unsigned int reserved[9];      // -
	aux_mu_io_reg mu_io;           // Mini Uart I/O Data 
	aux_mu_ier_reg mu_ier;         // Mini Uart Interrupt Enable
	aux_mu_iir_reg mu_iir;         // Mini Uart Interrupt Identify
	uax_mu_lcr_reg mu_lcr;         // Mini Uart Line Control
	uax_mu_mcr_reg mu_mcr;         // Mini uart Modem Control
	aux_mu_lsr_reg mu_lsr;         // Mini Uart Line Status
	aux_mu_msr_reg mu_msr;         // Mini Uart Modem Status
	aux_mu_scratch mu_scratch;     // Mini Uart Scratch
	aux_mu_cntl_reg mu_cntl;       // Mini Uart Extra control
	aux_mu_stat_reg mu_stat;       // Mini Uart Extra Status
	aux_mu_baud_reg baud_rate;     // Mini Uart Baudrate
	aux_spi_cntl0_reg spi0_cntl0;  // SPI 1 Control register 0
	aux_spi_cntl1_reg spi0_cntl1;  // SPI 1 Control register 1
	aux_spi_stat spi0_stat;        // SPI 1 Status
	aux_spi_io spi0_io;            // SPI 1 Data
	aux_spi_peek spi0_peek;        // SPI 1 Peek
	aux_spi_cntl0_reg spi1_cntl0;  // SPI 2 Control register 0
	aux_spi_cntl1_reg spi1_cntl1;  // SPI 2 Control register 1
} Uart;

unsigned int uart_initialize(void);