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
#define MAX_PKT_LEN		0x100

XAxiDma AxiDma;
int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config);
uint32_t bufarray[NUM_BD][256/4];

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
// pl_ps interrupt ID[15:0] = 91:84, 68:64, 63:61
#define INTC_DEVICE_INT_ID	84 // 0x0E

int ScuGicExample(u16 DeviceId);
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
	int Status;

	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) = 0xff;
	*((uint32_t *)XPAR_AXI_GPIO_1_BASEADDR) = 0xffff;

	Xil_AssertSetCallback(AssertPrint);

	XAxiDma_Config *AxiDmaConfig = NULL;
	xdma_setup(&AxiDma, AxiDmaConfig);

	xil_printf("\r\nGIC Example Test\r\n");

	Status = ScuGicExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GIC Example Test\r\n");

}

int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config)
{
	int Status;

	Config = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
	} else {
		xil_printf("Yes! config found for %d\r\n", DMA_DEV_ID);
	}

	XAxiDma_CfgInitialize(InstancePtr, Config);
	if(!XAxiDma_HasSg(InstancePtr)) {
		xil_printf("Device configured as Simple mode \r\n");
	} else {
		xil_printf("Device configured with Scatter Gather \r\n");
	}

	xil_printf("AxiDma.RegBase = 0x%08x\r\n", AxiDma.RegBase);
    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));

    // now setup the buffer descriptors (BD)
    // Let's create a cyclic ring of four descriptors. Each one moves 256 bytes of data.
	XAxiDma_BdRing *RxRingPtr;
	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	int BdCount = NUM_BD;
	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE, RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd create failed with %d\r\n", Status);
	} else {
		xil_printf("Rx bd create passed! %d\r\n", Status);
	}

	xil_printf("RxRingPtr->FirstBdAddr = 0x%08x\r\n", RxRingPtr->FirstBdAddr);
	xil_printf("RxRingPtr->LastBdAddr = 0x%08x\r\n", RxRingPtr->LastBdAddr);
	xil_printf("RxRingPtr->BdaRestart = 0x%08x\r\n", RxRingPtr->BdaRestart);
	xil_printf("RxRingPtr->CyclicBd = 0x%08x\r\n", RxRingPtr->CyclicBd);
	xil_printf("RxRingPtr->Cyclic = 0x%08x\r\n", RxRingPtr->Cyclic);

	XAxiDma_Bd BdTemplate;
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd clone failed with %d\r\n", Status);
	} else {
		xil_printf("Rx bd clone passed!  %d\r\n", Status);
	}

	int FreeBdCount =  RxRingPtr->FreeCnt;
	xil_printf("RxRingPtr->FreeCnt = %d\r\n", RxRingPtr->FreeCnt);

	XAxiDma_Bd *BdPtr, *BdCurPtr;
	UINTPTR RxBufferPtr;
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd alloc failed with %d\r\n", Status);
	} else {
		xil_printf("Rx bd alloc Passed! %d\r\n", Status);
	}
	BdCurPtr = BdPtr;         // pointer to buffer descriptor.
	RxBufferPtr = (UINTPTR)bufarray;   // pointer to buffer.

	for (int Index = 0; Index < FreeBdCount; Index++) {

		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set buffer addr %x on BD %x failed %d\r\n", (unsigned int)RxBufferPtr, (UINTPTR)BdCurPtr, Status);
		} else {
			xil_printf("Rx set buffer addr %x on BD %x passed %d\r\n", (unsigned int)RxBufferPtr, (UINTPTR)BdCurPtr, Status);
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN, RxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n", MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);
		} else {
			xil_printf("Rx set length %d on BD %x passed %d\r\n", MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);
		}

		XAxiDma_BdSetCtrl(BdCurPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);
		RxBufferPtr += MAX_PKT_LEN;
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);

	}


    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx ToHw failed with %d\r\n", Status);
	} else {
		xil_printf("Rx ToHw passed with %d\r\n", Status);
	}

    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));
    xil_printf("XAXIDMA_TDESC_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_TDESC_OFFSET));

	XAxiDma_Bd *JunkBdPtr;
	JunkBdPtr = BdPtr;
    for (int i=0; i<NUM_BD; i++){
    	JunkBdPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, JunkBdPtr);
    	xil_printf("JunkBdPtr = 0x%08x\r\n", JunkBdPtr);
    }

	XAxiDma_BdRingEnableCyclicDMA(RxRingPtr);
	XAxiDma_SelectCyclicMode(InstancePtr, XAXIDMA_DEVICE_TO_DMA, 1);

    // Start it dma running!
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx start BD ring failed with %d\r\n", Status);
	} else {
		xil_printf("Rx start BD ring passed with %d\r\n", Status);
	}

    xil_printf("XAXIDMA_CR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET));
    xil_printf("XAXIDMA_SR_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET));
    xil_printf("XAXIDMA_TDESC_OFFSET = 0x%08x\r\n", XAxiDma_ReadReg(InstancePtr->RegBase, XAXIDMA_RX_OFFSET+XAXIDMA_TDESC_OFFSET));

	return(0);
}


int ScuGicExample(u16 DeviceId)
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


	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
			   (Xil_ExceptionHandler)DeviceDriverHandler,
			   (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// make the interrupt rising edge triggered.
	u8 priority_val, trigger_val;
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&priority_val,&trigger_val);
	xil_printf("priority = 0x%x, trigger = 0x%x\r\n", priority_val, trigger_val);
	trigger_val = 0x03;
	XScuGic_SetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,priority_val,trigger_val);
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&priority_val,&trigger_val);
	xil_printf("priority = 0x%x, trigger = 0x%x\r\n", priority_val, trigger_val);

	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

//	// Simulate the Interrupt
//	Status = XScuGic_SoftwareIntr(&InterruptController,	INTC_DEVICE_INT_ID, XSCUGIC_SPI_CPU0_MASK);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}

	while (1) {
		if (InterruptProcessed) {
			break;
		}
	}
	return XST_SUCCESS;
}


int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, 
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;
}


void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;	
	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) -= 1;
}


