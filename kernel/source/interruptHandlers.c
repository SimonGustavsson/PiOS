#include "interruptHandlers.h"
#include "hardware/arm_interrupts.h"
#include "timer.h"
#include "uart.h"
#include "types/string.h"
#include "util/utilities.h"
#include "debugging.h"
#include "scheduler.h"
#include "stddef.h"
#include "stdbool.h"
#include "stdint.h"

// Because I sometimes mess up and end up getting spammed with
// Abort interrupts of various types, this adds a delay to the abort handlers
// So that I have time to read the error before spam scrolls it out of view
// </Lazymode> :-)
#define INTERRUPT_HANDLER_DELAY 2000 // in ms

extern int gTerminalInitialized;

bool in_fault = false;

swi_handler gSwiHandlers[MAX_SWI_NUM];

void swi_init(void)
{
    for (uint32_t i = 0; i < MAX_SWI_NUM; i++)
        gSwiHandlers[i] = NULL;
}

bool swi_install(unsigned int swiNum, swi_handler handler)
{
    if (swiNum > MAX_SWI_NUM)
    {
        printf("Swi: Can't install swi handler for %d, MAX_SWI_NUL=%d\n", swiNum, MAX_SWI_NUM);
        return false;
    }

    if (gSwiHandlers[swiNum] != NULL)
    {
        printf("Swi: Can't install swi handler for %d, a handler is already present\n", swiNum);
        return false;
    }

    gSwiHandlers[swiNum] = handler;

    return true;
}

void double_fault(void)
{
    printf("Double fault!\n");
    printf("* * * PANIC * * *\n");
    while (1);
}

void c_undefined_handler(void* lr)
{
    bool doubleFault = in_fault == true;
    in_fault = true;

    printf("Undefined instruction at 0x%h. (instruction: %d).\n", lr, *((unsigned int*)lr));

    //unsigned int* instAddr = (unsigned int*)*(r14 - 1) + 4;
    //printf("Instruction that cause abort is at 0x%h (%d) - SPSR: 0x%h.\n", instAddr, *instAddr, 42);// spsr);

    wait(INTERRUPT_HANDLER_DELAY);

    in_fault = false;

    if (doubleFault)
        double_fault();
}

void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr, unsigned int fault_reg)
{
    //bool doubleFault = in_fault == true;
    in_fault = true;

    // NOTE: fault_reg isn't used (yet), we should just use that and extract the Fault status from
    // it here as opposed to doing it in asm and pass it in as a separate argument
    printf("Instruction in %s at 0x%h caused a data abort accessing memory at 0x%h (", Debug_GetClosestPreviousFunction(address), address, accessedAddr);
    print_abort_error(errorType);
    printf(")\n");
    
    //Debug_PrintCallstack(2);
 
    wait(INTERRUPT_HANDLER_DELAY);
    in_fault = false;

    // Temporarily allow double faults due to bug with the translation tables
    // when switching tasks
 /*   if (doubleFault)
        double_fault();*/
}

void c_abort_instruction_handler(unsigned int address, unsigned int errorType)
{
    bool doubleFault = in_fault == true;
    in_fault = true;

    if (*(unsigned int*)address == 0xE1200070)
    {
        printf("* * Breakpoint! * * \n");
    }
    else
    {
        printf("Instruction at 0x%h (value: %d) caused instruction abort ", address, *(unsigned int*)address);

        print_abort_error(errorType);
    }
    
    wait(INTERRUPT_HANDLER_DELAY);

    in_fault = false;

    if (doubleFault)
        double_fault();
}

void c_swi_handler(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int swi)
{
    // This shouldn't happen, but who knows what users do..
    if (swi > MAX_SWI_NUM)
        return;

    printf("SWI %d invoked\n", swi);

    swi_handler handler = gSwiHandlers[swi];
    if (handler != NULL)
    {
        handler(r0, r1, r2);
    }
    else
    {
        printf("swi: Undefined SWI handler for swi %d\n", swi);
    }
}

void c_irq_handler(volatile unsigned int* sp)
{
    unsigned int pendingIrq = Arm_IrqGetPending();

    switch (pendingIrq)
    {
    case interrupt_source_system_timer:
        {
            // Note IRQ has no acccess to peripherals? :(
            
            Scheduler_TimerTick((thread_regs*)(sp - 1));

            break;
        }
        case interrupt_source_uart:
        {
            unsigned char read = Uart_Read();

            // Echo it back
            Uart_Send(read);

            if (read == 'x')
            {
                Uart_SendString("\r\n* * * Rebooting. * * *\r\n");
                reboot();
            }
            break;
        }
        default:
            printf("Unhandled IRQ pending, id:%d.\n", pendingIrq);
            break;
    }
}

void print_abort_error(unsigned int errorType)
{
    // Documentation for the Fault register (Which is where the errorType code is obtained from):
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0290g/Babbcbhc.html
    switch (errorType)
    {
    case 0x0:
        // This is the reset value
        printf("No error");
        break;
    case 0x1:
        printf("Alignment fault");
        break;
    case 0x2:
        printf("Instruction debug event");
        break;
    case 0x3:
        printf("Access bit fault on section");
        break;
    case 0x4:
        printf("No function");
        break;
    case 0x5:
        printf("Translation section fault");
        break;
    case 0x6:
        printf("Access bit fault on page");
        break;
    case 0x7:
        printf("Translation page fault");
        break;
    case 0x8:
        printf("Precise external abort");
        break;
    case 0x9:
        printf("Domain section fault");
        break;
    case 0xA:
        printf("No function");
        break;
    case 0xB:
        printf("Domain page fault");
        break;
    case 0xC:
        printf("External abort on translation, first level table");
        break;
    case 0xD:
        printf("Permission section fault");
        break;
    case 0xE:
        printf("External abort on translation, second level table");
        break;
    case 0xF:
        printf("Permission page fault");
        break;
    default:
        printf("print_abort_error() called with invalid errorType '%d'", errorType);
        break;
    }
}
