/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9

static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );

static TaskHandle_t xTxTask;
static TaskHandle_t xRxTask;
static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;
char HWstring[15] = "Hello World";
long RxtaskCntr = 0;

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	XPAR_FABRIC_PL_PS_IRQ10_INTR // 0x0E

volatile static int InterruptProcessed = FALSE;

extern XScuGic xInterruptController;  /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig;    /* The configuration parameters of the controller */
void DeviceDriverHandler(void *CallbackRef);
int HwIntSetup(XScuGic *InterruptController, u16 DeviceId);


int main( void )
{
	const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_10_SECONDS );

	xil_printf( "Hello from Freertos example main\r\n" );

	(*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) = 0xff;
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR)) = 0xffff;

	xTaskCreate(prvTxTask, ( const char * ) "Tx", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xTxTask);

	xTaskCreate(prvRxTask, ( const char * ) "GB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xRxTask );

	xQueue = xQueueCreate( 	1, sizeof( HWstring ) );
	configASSERT( xQueue );

	xTimer = xTimerCreate( (const char *) "Timer", x10seconds, pdFALSE, (void *)TIMER_ID, vTimerCallback);
	configASSERT( xTimer );

	xTimerStart( xTimer, 0 );

	int IntStatus = HwIntSetup(&xInterruptController, INTC_DEVICE_ID);
	xil_printf("IntStatus = 0x%x\r\n", IntStatus);

	vTaskStartScheduler();

	for( ;; );
}


static void prvTxTask( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );

	for( ;; ) {
		vTaskDelay( x1second );
		xQueueSend( xQueue,	HWstring, 0UL );
	}
}

static void prvRxTask( void *pvParameters )
{
	char Recdstring[15] = "";

	for( ;; ) {
		xQueueReceive( xQueue,	Recdstring,	portMAX_DELAY );
		xil_printf( "Rx task received string from Tx task: %s\r\n", Recdstring );
	    (*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR)) -= 1;  // flash the PMOD LEDs
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


int HwIntSetup(XScuGic *InterruptController, u16 DeviceId)
{
	int Status;

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = XScuGic_SelfTest(InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, InterruptController);

	Xil_ExceptionEnable();

	Status = XScuGic_Connect(InterruptController, INTC_DEVICE_INT_ID, (Xil_ExceptionHandler)DeviceDriverHandler, (void *)InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// make rising edge triggered
	u8 iPriority, iTrigger;
	XScuGic_GetPriorityTriggerType(InterruptController,INTC_DEVICE_INT_ID,&iPriority,&iTrigger);
	iTrigger = 0x03;
	XScuGic_SetPriorityTriggerType(InterruptController,INTC_DEVICE_INT_ID, iPriority, iTrigger);
	xil_printf("iPriority = 0x%x, iTrigger = 0x%x\r\n", iPriority, iTrigger);

	// Enable the interrupt for the device and then cause (simulate) an interrupt so the handlers will be called
	XScuGic_Enable(InterruptController, INTC_DEVICE_INT_ID);

	return XST_SUCCESS;
}




void DeviceDriverHandler(void *CallbackRef)
{
    InterruptProcessed = TRUE; // Indicate the interrupt has been processed using a shared variable
    (*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) -= 1;  // flash the LEDs
}

