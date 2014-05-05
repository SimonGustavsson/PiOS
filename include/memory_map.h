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
#define KERNEL_PA_PT 0x000A8000 // was F
#define KERNEL_PA_TMP_TTB0 (KERNEL_PA_PT + 0x4000)
#define PERIHERAL_PA_START 0x20000000

// Virtual addresses
#define KERNEL_VA_START 0x80000000
#define USR_VA_START 0x00000000
#define PERIPHERAL_VA_START (KERNEL_VA_START + PERIHERAL_PA_START)
