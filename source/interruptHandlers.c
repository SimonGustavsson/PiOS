#include "interruptHandlers.h"
#include "hardware/interrupts.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "types/string.h"
#include "util/utilities.h"
#include "debugging.h"
#include "scheduler.h"

// Because I sometimes mess up and end up getting spammed with
// Abort interrupts of various types, this adds a delay to the abort handlers
// So that I have time to read the error before spam scrolls it out of view
// </Lazymode> :-)
#define INTERRUPT_HANDLER_DELAY 20 // in ms

void c_undefined_handler(void* lr)
{
    printf("Undefined instruction at 0x%h. (instruction: %d).\n", lr, *((unsigned int*)lr));

    //unsigned int* instAddr = (unsigned int*)*(r14 - 1) + 4;
    //printf("Instruction that cause abort is at 0x%h (%d) - SPSR: 0x%h.\n", instAddr, *instAddr, 42);// spsr);

    wait(INTERRUPT_HANDLER_DELAY);
}

void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr, unsigned int fault_reg)
{
    // NOTE: fault_reg isn't used (yet), we should just use that and extract the Fault status from
    // it here as opposed to doing it in asm and pass it in as a separate argument
    printf("Instruction in %s at 0x%h caused a data abort accessing memory at 0x%h (", Debug_GetClosestPreviousFunction(address), address, accessedAddr);
    print_abort_error(errorType);
    printf(")\n");
    
    Debug_PrintCallstack();
 
    wait(INTERRUPT_HANDLER_DELAY);
}

void c_abort_instruction_handler(unsigned int address, unsigned int errorType)
{
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
}

void c_swi_handler(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int swi)
{
    switch (swi)
    {
    case 12: // sys_print
        // r0 should be a char*, TODO: add validation
        printf("%s", (char*)(r0));
        break;
    case 95:
        // Print example
        printf("Swi example print call(95)\n");
        break;
    case 96:
        printf("Second printf SVC call(96)!\n");
        break;
    default:
        printf("Unhandled SWI call: %d.\n", swi);
        break;
    }
}

void c_irq_handler(volatile unsigned int* r0)
{
    unsigned int pendingIrq = Arm_IrqGetPending();

    switch (pendingIrq)
    {
    case interrupt_source_system_timer:
        {
            // Note IRQ has no acccess to peripherals? :(
            
            Scheduler_TimerTick((registers*)r0);

            //gTaskSchedulerTick = 1;
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
