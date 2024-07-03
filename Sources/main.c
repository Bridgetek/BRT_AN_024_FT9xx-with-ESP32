/**
  @file main.c
  @brief Free RTOS with D2XX
  An example of thread-safe D2XX library usage with FreeRTOS
*/
/*
 * ============================================================================
 * History
 * =======
 * 11 Sep 2016 : Created
 *
 * (C) Copyright,  Bridgetek Pte. Ltd.
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of FTDI liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on FTDI's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, FTDI
 * has no liability in relation to those amendments.
 * ============================================================================
 */

/* INCLUDES ************************************************************************/

#include <stdint.h>
#include <ft900.h>
#include <ft900_startup_dfu.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uartrb.h"

#include "messages.h"

#define USER_TASK_STACK_SIZE 1000	/**< 32-bit words */

/* GLOBAL VARIABLES ****************************************************************/

TaskHandle_t	hTask_AT_Task;		/**< handles to user tasks */
TaskHandle_t	hTask_EVE_Task;		/**< handles to user tasks */

void 		vATmonitorTask (void *intf);		/**< user task */
void 		vEVEmonitorTask (void *intf);		/**< user task */

struct eve_setup_s config;

/* LOCAL FUNCTIONS / INLINES *******************************************************/

void setup(void);
void debug_uart_init(void);
void tfp_putc(void* p, char c);

/* FUNCTIONS ***********************************************************************/
int main(void)
{
	/* Setup UART */
    setup();

    //config.xSemaphoreATReady = xSemaphoreCreateBinary();
    //config.xSemaphoreEVEReady = xSemaphoreCreateBinary();
    config.at2eve_q = xQueueCreate( 10, sizeof(struct at2eve_messages_s) );
    config.eve2at_q = xQueueCreate( 10, sizeof(struct eve2at_messages_s) );

	hTask_AT_Task = NULL;
	xTaskCreate(
		vATmonitorTask, 		/* Task */
		"ESP32 Task",               	/* Name for the task */
		USER_TASK_STACK_SIZE,		/* Stack depth  */
		(void *) &config,			/* Task parameters */
		tskIDLE_PRIORITY,		/* Priority */
		&hTask_AT_Task		/* Task Handles for each interface */
		);

    hTask_EVE_Task = NULL;
	xTaskCreate(
			vEVEmonitorTask, 		/* Task */
		"EVE Task",               	/* Name for the task */
		USER_TASK_STACK_SIZE,		/* Stack depth  */
		(void *) &config,			/* Task parameters */
		tskIDLE_PRIORITY,		/* Priority */
		&hTask_EVE_Task		/* Task Handles for each interface */
		);

	tfp_printf("TaskScheduler started\r\n");

	vTaskStartScheduler();
	// function never returns 
	for (;;) ;
}

void setup(void)
{
	// UART initialisation
	debug_uart_init();

	/* Print out a welcome message... */
	tfp_printf ("(C) Copyright, Bridgetek Pte. Ltd. \r\n \r\n");
	tfp_printf ("---------------------------------------------------------------- \r\n");
	tfp_printf ("Welcome to ESP32 AT command demo with FreeRTOS\r\n");
}

/** @name tfp_putc
 *  @details Machine dependent putc function for tfp_printf (tinyprintf) library.
 *  @param p Parameters (machine dependent)
 *  @param c The character to write
 */
void tfp_putc(void* p, char c)
{
	uartrb_putc((ft900_uart_regs_t*)p, (uint8_t)c);
}

/* Initializes the UART for the testing */
void debug_uart_init(void)
{

	/* Enable the UART Device... */
	sys_enable(sys_device_uart0);
#if defined(__FT930__)
    /* Make GPIO23 function as UART0_TXD and GPIO22 function as UART0_RXD... */
    gpio_function(23, pad_uart0_txd); /* UART0 TXD */
    gpio_function(22, pad_uart0_rxd); /* UART0 RXD */
#else
	/* Make GPIO48 function as UART0_TXD and GPIO49 function as UART0_RXD... */
	gpio_function(48, pad_uart0_txd); /* UART0 TXD MM900EVxA CN3 pin 4 */
	gpio_function(49, pad_uart0_rxd); /* UART0 RXD MM900EVxA CN3 pin 6 */
	gpio_function(50, pad_uart0_rts); /* UART0 RTS MM900EVxA CN3 pin 8 */
	gpio_function(51, pad_uart0_cts); /* UART0 CTS MM900EVxA CN3 pin 10 */
#endif

	// Open the UART using the coding required.
	uart_open(UART0,                    /* Device */
			1,                        /* Prescaler = 1 */
			UART_DIVIDER_115200_BAUD,  /* Divider = 1302 */
			uart_data_bits_8,         /* No. buffer Bits */
			uart_parity_none,         /* Parity */
			uart_stop_bits_1);        /* No. Stop Bits */

	/* Print out a welcome message... */
	uart_puts(UART0,
			"\x1B[2J" /* ANSI/VT100 - Clear the Screen */
			"\x1B[H\r\n"  /* ANSI/VT100 - Move Cursor to Home */
	);

#ifdef DEBUG
	/* Enable tfp_printf() functionality... */
	init_printf(UART0, tfp_putc);
#endif

}

