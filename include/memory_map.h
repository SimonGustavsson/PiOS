//
// NOTE: This file needs to be kept to strictly defines as it's shared between C and asm.
//

// Physical addresses
#define FIQ_STACK_PA_START   0x00004000
#define IRQ_STACK_PA_START   0x00007900
#define SVC_STACK_PA_START   0x00C08000
#define SM_STACK_PA_START    0x0A827000
#define UD_STACK_PA_START    0x01008000
#define ABORT_STACK_PA_START 0x01208000

#define KERNEL_PA_START 0x00000000
#define KERNEL_PA_PT 0x000F8000
#define PERIHERAL_PA_START 0x20000000

// Virtual addresses
#define PERIPHERAL_VA_START 0xA0000000
#define KERNEL_VA_START 0x80000000
#define USR_VA_START 0x00000000
