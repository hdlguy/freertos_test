/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "xil_printf.h"
#include "xparameters.h"
#include "fpga.h"
//#include "xstatus.h"
#include "xuartlite_l.h"

#include <stdio.h>

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9
/*-----------------------------------------------------------*/

/* The Tx and Rx tasks as described at the top of this file. */
static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );
/*-----------------------------------------------------------*/

/* The queue used by the Tx and Rx tasks, as described at the top of this
file. */
static TaskHandle_t xTxTask;
static TaskHandle_t xRxTask;
static TaskHandle_t xCommTask;
static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;
char HWstring[15] = "Hello World";
long RxtaskCntr = 0;

BaseType_t write_led ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
BaseType_t read_led  ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
void vCommandConsoleTask( void *pvParameters );

static const CLI_Command_Definition_t write_led_cmd =
{
    "write_led",
    "write_led <val>: Writes four-bit <val> to the Arty LEDsrn",
	write_led,
    1
};
static const CLI_Command_Definition_t read_led_cmd =
{
    "read_led",
    "read_led : Reads the four-bit value assigned to the Arty LEDsrn",
	read_led,
    0
};

int main( void )
{
	const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_10_SECONDS );

	xil_printf("\n\r");
	xil_printf( "Hello from Freertos example main\n\r" );

	FreeRTOS_CLIRegisterCommand( &write_led_cmd );
	FreeRTOS_CLIRegisterCommand( &read_led_cmd );

	/* Create the two tasks.  The Tx task is given a lower priority than the
	Rx task, so the Rx task will leave the Blocked state and pre-empt the Tx
	task as soon as the Tx task places an item in the queue. */

	//xTaskCreate(prvTxTask, ( const char * ) "Tx", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xTxTask );
	//xTaskCreate(prvRxTask, ( const char * ) "GB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &xRxTask );

	xTaskCreate( vCommandConsoleTask, ( const char * ) "COMM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 0, &xCommTask );

	/* Create the queue used by the tasks.  The Rx task has a higher priority
	than the Tx task, so will preempt the Tx task and remove values from the
	queue as soon as the Tx task writes to the queue - therefore the queue can
	never have more than one item in it. */
	xQueue = xQueueCreate( 	1,						/* There is only one space in the queue. */
							sizeof( HWstring ) );	/* Each space in the queue is large enough to hold a uint32_t. */

	/* Check the queue was created. */
	configASSERT( xQueue );

	/* Create a timer with a timer expiry of 10 seconds. The timer would expire
	 after 10 seconds and the timer call back would get called. In the timer call back
	 checks are done to ensure that the tasks have been running properly till then.
	 The tasks are deleted in the timer call back and a message is printed to convey that
	 the example has run successfully.
	 The timer expiry is set to 10 seconds and the timer set to not auto reload. */
	xTimer = xTimerCreate( (const char *) "Timer",x10seconds, pdFALSE, (void *) TIMER_ID, vTimerCallback);
	configASSERT( xTimer ); 	/* Check the timer was created. */

	/* start the timer with a block time of 0 ticks. This means as soon
	   as the schedule starts the timer will start running and will expire after
	   10 seconds */
	xTimerStart( xTimer, 0 );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}


/*-----------------------------------------------------------*/
static void prvTxTask( void *pvParameters )
{
const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );

	for( ;; )
	{
		/* Delay for 1 second. */
		vTaskDelay( x1second );

		/* Send the next value on the queue.  The queue should always be
		empty at this point so a block time of 0 is used. */
		xQueueSend( xQueue,			/* The queue being written to. */
					HWstring, /* The address of the data being sent. */
					0UL );			/* The block time. */
	}
}

/*-----------------------------------------------------------*/
static void prvRxTask( void *pvParameters )
{
char Recdstring[15] = "";

	for( ;; )
	{
		/* Block to wait for data arriving on the queue. */
		xQueueReceive( 	xQueue,				/* The queue being read. */
						Recdstring,	/* Data is read into this address. */
						portMAX_DELAY );	/* Wait without a timeout for data. */

		/* Print the received data. */
		xil_printf( "Rx task received string from Tx task: %s\r\n", Recdstring );
		RxtaskCntr++;
	}
}

/*-----------------------------------------------------------*/
static void vTimerCallback( TimerHandle_t pxTimer )
{
	long lTimerId;
	configASSERT( pxTimer );

	lTimerId = ( long ) pvTimerGetTimerID( pxTimer );

	if (lTimerId != TIMER_ID) {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	/* If the RxtaskCntr is updated every time the Rx task is called. The
	 Rx task is called every time the Tx task sends a message. The Tx task
	 sends a message every 1 second.
	 The timer expires after 10 seconds. We expect the RxtaskCntr to at least
	 have a value of 9 (TIMER_CHECK_THRESHOLD) when the timer expires. */
	if (RxtaskCntr >= TIMER_CHECK_THRESHOLD) {
		xil_printf("Successfully ran FreeRTOS Hello World Example\n\r");
	} else {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	vTaskDelete( xRxTask );
	vTaskDelete( xTxTask );
}


BaseType_t write_led( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
	int8_t *pcParameter1;
	BaseType_t xParameter1StringLength;
	uint32_t *regptr = (uint32_t *)XPAR_M00_AXI_BASEADDR;

	pcParameter1 = FreeRTOS_CLIGetParameter (pcCommandString, 1, &xParameter1StringLength);

	pcParameter1[ xParameter1StringLength ] = 0x00;

	snprintf( pcWriteBuffer, xWriteBufferLen, pcCommandString);
	regptr[LED_CONTROL] = atoi(pcParameter1);

	return(pdFALSE);
}

BaseType_t read_led ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
	uint32_t *regptr = (uint32_t *)XPAR_M00_AXI_BASEADDR;
	char tempstr[10];

	sprintf(tempstr, "led = %d", regptr[LED_CONTROL]);
	snprintf( pcWriteBuffer, xWriteBufferLen, tempstr);



	return(pdFALSE);
}


// Let's make a task that takes input from the console.
#define TEST_BUFFER_SIZE 100

void vCommandConsoleTask( void *pvParameters )
{
    uint32_t Index;
    u32 uart_ptr = XPAR_UARTLITE_0_BASEADDR;
    char char_temp;
    BaseType_t xMoreDataToFollow;
    static char pcOutputString[TEST_BUFFER_SIZE], pcInputString[TEST_BUFFER_SIZE];

    print("Type commands.\n\r");

    Index = 0;
    for(;;){
        char_temp = XUartLite_RecvByte(uart_ptr);  // this waits until there is a character available.
        pcInputString[Index] = char_temp;
        if (Index != (TEST_BUFFER_SIZE-1)) Index++;

        if (char_temp=='\r'){ // carriage return detected
            Index = 0;

            //process the command line.
            do
            {
                /* Send the command string to the command interpreter.  Any
                output generated by the command interpreter will be placed in the
                pcOutputString buffer. */
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand (
                	pcInputString,   /* The command string.*/
					pcOutputString,   /* The output buffer. */
					TEST_BUFFER_SIZE/* The size of the output buffer. */
                );

                /* Write the output generated by the command interpreter to the console. */
                xil_printf("%s\n\r", pcOutputString);

            } while( xMoreDataToFollow != pdFALSE );

        }
    }

}

