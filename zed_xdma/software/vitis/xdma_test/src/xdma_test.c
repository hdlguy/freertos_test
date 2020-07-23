#include <stdio.h>
#include <stdlib.h>
#include "xil_io.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xscugic.h"
#include "xaxidma.h"

#define RX_BD_SPACE_BASE	(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR) // bram for BD
#define RX_BD_SPACE_HIGH	(XPAR_AXI_BRAM_CTRL_0_S_AXI_HIGHADDR)
#define DMA_DEV_ID		XPAR_AXI_DMA_0_DEVICE_ID
#define NUM_BD  4
#define MAX_PKT_LEN		0x0800 // 0x100
#define XDMA_INT_ID    61
//#define XDMA_BYTES_PER_FRAME 1024 // 256

XAxiDma AxiDma;
int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config);
void xdma_handler(void *CallbackRef);
uint32_t bufarray[NUM_BD][MAX_PKT_LEN/4] __attribute__((aligned(256)));
volatile static int xdma_intr_detected = FALSE;
uint32_t* BufReadyPtr;

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
// pl_ps interrupt ID[15:0] = 91:84, 68:64, 63:61
#define INTC_DEVICE_INT_ID	84 // 0x0E

int intr_setup(u16 DeviceId);
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);
void DeviceDriverHandler(void *CallbackRef);

XScuGic InterruptController; 	     /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig;    /* The configuration parameters of the controller */

volatile static int InterruptProcessed = FALSE;

static void AssertPrint(const char8 *FilenamePtr, s32 LineNumber){
	xil_printf("ASSERT: File Name: %s ", FilenamePtr);
	xil_printf("Line Number: %d\r\n",LineNumber);
}


int main(void)
{
	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) = 0xff;
	*((uint32_t *)XPAR_AXI_GPIO_1_BASEADDR) = 0xffff;
	*((uint32_t *)XPAR_AXI_GPIO_2_BASEADDR) = MAX_PKT_LEN/4-1;  // sets the frame length of the data generator.

	Xil_AssertSetCallback(AssertPrint);

	xil_printf("\r\nStarting test\r\n");

	intr_setup(INTC_DEVICE_ID);

	XAxiDma_Config *AxiDmaConfig = NULL;
	xdma_setup(&AxiDma, AxiDmaConfig);

	xil_printf("Test running\r\n");

	// do error checking on the data.
	uint32_t temp1, temp2, *checkbuf;	
	int data_error = 0;
	int intr_count = 0;
	for(;;){
	    while(!xdma_intr_detected);  // wait for xdma interrupt.
	    xdma_intr_detected = FALSE;	    	    
	    
		checkbuf = BufReadyPtr;		

		for (int i=0; i<(MAX_PKT_LEN/4); i++) {
			temp2 = checkbuf[i];
			if ((temp2 != (temp1+1)) && (5 < intr_count)) data_error++;
			temp1 = temp2;
		}
		if ((intr_count%4096)==0) xil_printf("data_error = %d\r\n", data_error);
		
	    intr_count++;
	}
}


void xdma_handler(void *CallbackRef) // xdma interrupt handler
{
	*((uint32_t *)XPAR_AXI_GPIO_1_BASEADDR) -= 1;  // flash some LEDs

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

	//int BdCount = NUM_BD;
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

	Status = XAxiDma_BdRingStart(RxRingPtr);    // Start dma running!

    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n",    XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n",    XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));
	xil_printf("RxRingPtr->FirstBdAddr = 0x%08x\r\n", RxRingPtr->FirstBdAddr);
	xil_printf("RxRingPtr->LastBdAddr = 0x%08x\r\n",  RxRingPtr->LastBdAddr);
	xil_printf("RxRingPtr->Cyclic = 0x%08x\r\n",      RxRingPtr->Cyclic);

	return(0);
}


int intr_setup(u16 DeviceId)
{
	int Status;

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &InterruptController);
	Xil_ExceptionEnable();

	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID, (Xil_ExceptionHandler)DeviceDriverHandler, (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status = XScuGic_Connect(&InterruptController, XDMA_INT_ID, (Xil_ExceptionHandler)xdma_handler, (void *)XAxiDma_GetRxRing(&AxiDma));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// make the interrupt rising edge triggered.
	u8 priority_val, trigger_val;
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&priority_val,&trigger_val);
	trigger_val = 0x03;
	XScuGic_SetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,priority_val,trigger_val);
	xil_printf("INTC_DEVICE_INT_ID: priority_val = 0x%08x, trigger_val = 0x%08x\r\n", priority_val, trigger_val);

	XScuGic_GetPriorityTriggerType(&InterruptController,XDMA_INT_ID,&priority_val,&trigger_val);
	trigger_val = 0x03; priority_val = 0xA8;
	XScuGic_SetPriorityTriggerType(&InterruptController,XDMA_INT_ID,priority_val,trigger_val);
	xil_printf("XDMA_INT_ID: priority_val = 0x%08x, trigger_val = 0x%08x\r\n", priority_val, trigger_val);

	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);
	XScuGic_Enable(&InterruptController, XDMA_INT_ID);

	return(0);
}


void DeviceDriverHandler(void *CallbackRef)  // counter interrupt handler
{
	InterruptProcessed = TRUE;	
	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) -= 1;
}






