/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xintc.h"
#include "xil_exception.h"

#define TIMER_ID	1
#define DELAY_15_SECONDS	15000UL
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9

/* The Tx and Rx tasks as described at the top of this file. */
static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );

/* The queue used by the Tx and Rx tasks, as described at the top of this file. */
static TaskHandle_t  xTxTask;
static TaskHandle_t  xRxTask;
static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;
char HWstring[15] = "Hello!";
long RxtaskCntr = 0;

int SetUpInterruptSystem(XIntc *XIntcInstancePtr, u16 DeviceId, u8 pps_device_id);
void DeviceDriverHandler(void *CallbackRef);

//extern XScuGic xInterruptController
static XIntc InterruptController; 
volatile static int InterruptProcessed = FALSE;

int main( void )
{
	const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_15_SECONDS );

	xil_printf( "Hello from Freertos example main\r\n" );

	(*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) = 0xff; // let's set the LED's
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR)) = 0xaaaa;

	xTaskCreate( prvTxTask, ( const char * ) "Tx", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY,     &xTxTask );
	xTaskCreate( prvRxTask, ( const char * ) "GB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xRxTask );

	xQueue = xQueueCreate( 	1, sizeof( HWstring ) );
	configASSERT( xQueue );

	xTimer = xTimerCreate( (const char *) "Timer", x10seconds, pdFALSE, (void *) TIMER_ID, vTimerCallback);
	configASSERT( xTimer );

	xTimerStart( xTimer, 0 );



	u16 DeviceId 		= XPAR_INTC_0_DEVICE_ID; 			// i think this is the ID of the INTC core.
	u8  pps_device_id 	= XPAR_AXI_INTC_0_SYSTEM_PPS_INTR;	// i think this is the ID of the device wired into the INTC core.
	int Status = SetUpInterruptSystem(&InterruptController, DeviceId, pps_device_id);
	if (Status != XST_SUCCESS) xil_printf("SetUpInterruptSystem: FAILURE\n\r");


	vTaskStartScheduler(); // Start the tasks and timer running.

	for( ;; );
}




static void prvTxTask( void *pvParameters )
{
    const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );

	for( ;; ) {
		vTaskDelay( x1second );
		xQueueSend( xQueue, HWstring, 0UL );	
	}
}



static void prvRxTask( void *pvParameters )
{
    char Recdstring[15] = "";

	for( ;; ) {
		xQueueReceive( 	xQueue, Recdstring,	portMAX_DELAY );
		xil_printf( "Rx task received string from Tx task: %s\r\n", Recdstring );
		(*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR))--;
		RxtaskCntr++;
	}
}



static void vTimerCallback( TimerHandle_t pxTimer )
{
	long lTimerId;
	configASSERT( pxTimer );

	lTimerId = ( long ) pvTimerGetTimerID( pxTimer );

	if (lTimerId != TIMER_ID) {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	if (RxtaskCntr >= TIMER_CHECK_THRESHOLD) {
		xil_printf("Successfully ran FreeRTOS Hello World Example");
	} else {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	vTaskDelete( xRxTask );
	vTaskDelete( xTxTask );
}

int SetUpInterruptSystem(XIntc *XIntcInstancePtr, u16 DeviceId, u8 pps_device_id)
{
	int Status;

	Status = XIntc_Initialize(&InterruptController, DeviceId);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = XIntc_Connect(XIntcInstancePtr, pps_device_id, (XInterruptHandler)DeviceDriverHandler, (void *)0);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = XIntc_Start(XIntcInstancePtr, XIN_REAL_MODE); // XIN_SIMULATION_MODE);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	XIntc_Enable(XIntcInstancePtr, pps_device_id);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, XIntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR))--;  // decrement the PMOD LEDs
}

