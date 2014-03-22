#include "interruptHandlers.h"
#include "hardware/interrupts.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "types/string.h"
#include "util/utilities.h"

// Imported via extern in taskScheduler
volatile unsigned int gTaskSchedulerTick;

void c_undefined_handler(void* lr)
{
    printf("Undefined instruction at 0x%h. (instruction: %d).\n", lr, *((unsigned int*)lr));

    //unsigned int* instAddr = (unsigned int*)*(r14 - 1) + 4;
    //printf("Instruction that cause abort is at 0x%h (%d) - SPSR: 0x%h.\n", instAddr, *instAddr, 42);// spsr);

    wait(2000);
}

void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr, unsigned int* sp)
{
    printf("Instruction at 0x%h caused a data abort accessing memory at 0x%h, \n", address, accessedAddr);

    print_abort_error(errorType);
 
    wait(3000);
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
    
    wait(3000);
}

void c_swi_handler(unsigned int swi)
{
    switch (swi)
    {
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

            //taskScheduler_TimerTick((registers*)r0);

            //// Restart the timer again
            //timer_sp_clearmatch();
            //timer_sp_setinterval(TASK_SCHEDULER_TICK_MS);

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
    switch (errorType)
    {
    case 0x5:
        printf("Translation fault.\n");
        break;
    case 0x8:
        printf("External abort on noncachable.\n");
        break;
    case 0x9:
        printf("Failed to access memory, domain error.\n");
        break;
    case 0xD:
        printf("Permission denied accessing memory\n");
        break;
    default:
        if ((errorType >> 0x2) == 0)
            printf("Abort misaligned memory access.\n");
        else
            printf("Unknown abort exception error code: %d.\n", errorType);
        break;
    }
}
