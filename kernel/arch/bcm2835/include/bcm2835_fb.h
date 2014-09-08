// The raspberry pi framebuffer, this maps directly onto the mailbox buffer
// to send to retrieve the framebuffer
typedef struct {
	unsigned int width;    // Requested screen width
	unsigned int height;   // Requested screen height
	unsigned int v_width;  // Requested virtual screen width
	unsigned int v_height; // Requested virtual screen height
	unsigned int pitch;    // Initialize to 0, VC fills out with value
	unsigned int depth;    // BPP - Initialize to 0, VC fills out with value
	unsigned int offset_x; // X offset of virtual framebuffer
	unsigned int offset_y; // Y offset of virtual framebuffer
	unsigned int address;  // Initialize to 0, VC fills out with value
	unsigned int size;     // Initialize to 0, VC fills out with value
} rpi_fb;
