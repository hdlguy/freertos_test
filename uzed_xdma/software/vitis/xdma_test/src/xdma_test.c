/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xaxidma.h"

// xdma stuff
#define RX_BD_SPACE_BASE	(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR) // bram for BD
#define RX_BD_SPACE_HIGH	(XPAR_AXI_BRAM_CTRL_0_S_AXI_HIGHADDR)
#define DMA_DEV_ID		XPAR_AXI_DMA_0_DEVICE_ID
#define NUM_BD  4                  // number of buffer descriptors
#define MAX_PKT_LEN		0x0800 // 0x100
#define XDMA_INT_ID     XPAR_FABRIC_AXIDMA_0_VEC_ID
XAxiDma AxiDma;
int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config);
void xdma_handler(void *CallbackRef);
uint32_t bufarray[NUM_BD][MAX_PKT_LEN/4] __attribute__((aligned(256)));
volatile static int xdma_intr_detected = FALSE;
uint32_t* BufReadyPtr;
static QueueHandle_t xDmaQueue = NULL;
static void prvRxDmaTask( void *pvParameters );
static TaskHandle_t xRxDmaTask;
char DmaHWstring[15] = "xRxDmaTask";


#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9

static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void prvInitTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );

static TaskHandle_t xTxTask;
static TaskHandle_t xRxTask;
static TaskHandle_t xInitTask;
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
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR)) = 0xff;
	(*((uint32_t*)XPAR_AXI_GPIO_2_BASEADDR)) = 0xff;

	xTaskCreate(prvTxTask,    ( const char * ) "Tx",   configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 0, &xTxTask );
	xTaskCreate(prvRxTask,    ( const char * ) "GB",   configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xRxTask );
	xTaskCreate(prvInitTask,  ( const char * ) "Init", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &xInitTask );
	xTaskCreate(prvRxDmaTask, ( const char * ) "xdma", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 0, &xRxDmaTask );

	xQueue = xQueueCreate( 	1, sizeof( HWstring ) );
	configASSERT( xQueue );

	xDmaQueue = xQueueCreate( 	1, sizeof( HWstring ) );
	configASSERT( xDmaQueue );

	xTimer = xTimerCreate( (const char *) "Timer", x10seconds, pdFALSE, (void *)TIMER_ID, vTimerCallback);
	configASSERT( xTimer );

	xTimerStart( xTimer, 0 );

	int IntStatus = HwIntSetup(&xInterruptController, INTC_DEVICE_ID);
	xil_printf("IntStatus = 0x%x\r\n", IntStatus);

	XAxiDma_Config *AxiDmaConfig = NULL;
	xdma_setup(&AxiDma, AxiDmaConfig);

	vTaskStartScheduler();

	for( ;; );  // this never executes.
}


void xdma_handler(void *CallbackRef) // xdma interrupt handler
{

	(*((uint32_t *)XPAR_AXI_GPIO_1_BASEADDR)) -= 1;  // flash some LEDs

	XAxiDma_BdRing *RxRingPtr = (XAxiDma_BdRing *) CallbackRef;
    
    // ack the interrupt
    u32 IrqStatus;
	IrqStatus = XAxiDma_BdRingGetIrq(RxRingPtr);
	XAxiDma_BdRingAckIrq(RxRingPtr, IrqStatus);
	
	// determine the pointer to the buffer with new data.
	uint32_t *CurrBdPtr, *PrevBdPtr, *PrevBufAddr;
	CurrBdPtr = XAxiDma_BdRingGetCurrBd(RxRingPtr);
	PrevBdPtr = XAxiDma_BdRingPrev(RxRingPtr, CurrBdPtr);
	PrevBufAddr = XAxiDma_BdGetBufAddr(PrevBdPtr);
	BufReadyPtr = PrevBufAddr;
	xil_printf("0x%08x\r\n",CurrBdPtr);

	*DmaHWstring = BufReadyPtr;
	xQueueSendToBackFromISR(xDmaQueue, DmaHWstring, NULL);

	xdma_intr_detected = TRUE;  // set the semaphore
}


int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config)
{
	int Status;

	Config = XAxiDma_LookupConfig(DMA_DEV_ID);

	XAxiDma_CfgInitialize(InstancePtr, Config);

	xil_printf("AxiDma.RegBase = 0x%08x\r\n", AxiDma.RegBase);

	XAxiDma_BdRing *RxRingPtr;
	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE, RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, NUM_BD);

	XAxiDma_Bd BdTemplate;
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);

	int FreeBdCount =  RxRingPtr->FreeCnt;
	xil_printf("RxRingPtr->FreeCnt = %d\r\n", RxRingPtr->FreeCnt);

	XAxiDma_Bd *BdPtr, *BdCurPtr;
	UINTPTR RxBufferPtr;
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);

	BdCurPtr = BdPtr;         // pointer to buffer descriptor.
	RxBufferPtr = (UINTPTR)bufarray;   // pointer to buffer.
	for (int Index = 0; Index < FreeBdCount; Index++) {
		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN, RxRingPtr->MaxTransferLen);
		xil_printf("length: %d, BD: %x, buffer: %x\r\n", MAX_PKT_LEN, (UINTPTR)BdCurPtr, (unsigned int)RxBufferPtr);

		XAxiDma_BdSetCtrl(BdCurPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);
		RxBufferPtr += MAX_PKT_LEN;
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);

	XAxiDma_Bd *JunkBdPtr;
	JunkBdPtr = BdPtr;
    for (int i=0; i<NUM_BD; i++){
    	JunkBdPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, JunkBdPtr);
    	xil_printf("Ptr[%d] = 0x%08x\r\n", i, JunkBdPtr);
    }

	XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);
	XAxiDma_BdRingEnableCyclicDMA(RxRingPtr);
	XAxiDma_SelectCyclicMode(InstancePtr, XAXIDMA_DEVICE_TO_DMA, 1);

	//Status = XAxiDma_BdRingStart(RxRingPtr);    // Start dma running!

    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n",    XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n",    XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));
	xil_printf("RxRingPtr->FirstBdAddr = 0x%08x\r\n", RxRingPtr->FirstBdAddr);
	xil_printf("RxRingPtr->LastBdAddr = 0x%08x\r\n",  RxRingPtr->LastBdAddr);
	xil_printf("RxRingPtr->Cyclic = 0x%08x\r\n",      RxRingPtr->Cyclic);

	return(Status);
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
		xil_printf("Rx task received string from Tx task: %s\r\n", Recdstring);
		xil_printf("xdma_intr_detected = %d\r\n", xdma_intr_detected);
		xil_printf("InterruptProcessed = %d\r\n", InterruptProcessed);

	    (*((uint32_t*)XPAR_AXI_GPIO_2_BASEADDR)) -= 1;
		RxtaskCntr++;
	}
}

static void prvRxDmaTask( void *pvParameters )
{
	char Recdstring[15] = "";

	for( ;; ) {
		xQueueReceive( xDmaQueue, Recdstring, portMAX_DELAY );
		//xil_printf("%s\r\n", Recdstring);
		//xil_printf("0x%08x\r\n", *((uint32_t *)Recdstring));
	}
}

static void prvInitTask( void *pvParameters )
{
	for( ;; ) {
		xil_printf("InitTask\r\n");
		XAxiDma_BdRingStart(XAxiDma_GetRxRing(&AxiDma));    // Start dma running!
		vTaskDelete( xInitTask );                           // this task runs once and deletes itself.
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
    Status = XScuGic_Connect(InterruptController, XDMA_INT_ID,        (Xil_ExceptionHandler)xdma_handler,        (void *)XAxiDma_GetRxRing(&AxiDma));
    xil_printf("XDMA_INT_ID = %d, xdma_handler = 0x%08x, (void *)XAxiDma_GetRxRing(&AxiDma) = 0x%08x\r\n", XDMA_INT_ID, xdma_handler, (void *)XAxiDma_GetRxRing(&AxiDma));
    if (Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    // make rising edge triggered
    u8 iPriority, iTrigger;
    XScuGic_GetPriorityTriggerType(InterruptController,INTC_DEVICE_INT_ID,&iPriority,&iTrigger);
    iTrigger = 0x03;
    XScuGic_SetPriorityTriggerType(InterruptController,INTC_DEVICE_INT_ID, iPriority, iTrigger);
    xil_printf("INTC_DEVICE_INT_ID: iPriority = 0x%08x\r\n", iPriority);

    XScuGic_GetPriorityTriggerType(InterruptController,XDMA_INT_ID,&iPriority,&iTrigger);
    iTrigger = 0x03;
    XScuGic_SetPriorityTriggerType(InterruptController,XDMA_INT_ID, iPriority, iTrigger);
    xil_printf("XDMA_INT_ID: iPriority = 0x%08x\r\n", iPriority);

    // Enable the interrupt for the device
    XScuGic_Enable(InterruptController, INTC_DEVICE_INT_ID);
    XScuGic_Enable(InterruptController, XDMA_INT_ID);

    return XST_SUCCESS;
}



void DeviceDriverHandler(void *CallbackRef)
{
    InterruptProcessed = TRUE; // Indicate the interrupt has been processed using a shared variable
    (*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) -= 1;  // flash the LEDs
}

