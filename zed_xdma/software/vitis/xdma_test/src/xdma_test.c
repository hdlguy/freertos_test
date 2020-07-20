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
XAxiDma AxiDma;
int xdma_setup(XAxiDma * InstancePtr, XAxiDma_Config *Config);

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

	xil_printf("GIC Example Test\r\n");

	Status = ScuGicExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GIC Example Test\r\n");
	//return XST_SUCCESS;


	// ********* now lets set up the xdma
	XAxiDma_Config *AxiDmaConfig = NULL;
	xdma_setup(&AxiDma, AxiDmaConfig);

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
	int BdCount = 4;
	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE, RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);

	xil_printf("RxRingPtr->FirstBdAddr = 0x%08x\r\n", RxRingPtr->FirstBdAddr);
	xil_printf("RxRingPtr->LastBdAddr = 0x%08x\r\n", RxRingPtr->LastBdAddr);


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


//static int TxSetup(XAxiDma * AxiDmaInstPtr)
//{
//	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(&AxiDma);
//	XAxiDma_Bd BdTemplate;
//	int Status;
//	u32 BdCount;
//
//	/* Disable all TX interrupts before TxBD space setup */
//	XAxiDma_BdRingIntDisable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);
//
//	/* Setup TxBD space  */
//	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
//			(UINTPTR)TX_BD_SPACE_HIGH - (UINTPTR)TX_BD_SPACE_BASE + 1);
//
//	Status = XAxiDma_BdRingCreate(TxRingPtr, TX_BD_SPACE_BASE,
//				     TX_BD_SPACE_BASE,
//				     XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
//	if (Status != XST_SUCCESS) {
//
//		xil_printf("Failed create BD ring\r\n");
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Like the RxBD space, we create a template and set all BDs to be the
//	 * same as the template. The sender has to set up the BDs as needed.
//	 */
//	XAxiDma_BdClear(&BdTemplate);
//	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
//	if (Status != XST_SUCCESS) {
//
//		xil_printf("Failed clone BDs\r\n");
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Set the coalescing threshold, so only one transmit interrupt
//	 * occurs for this example
//	 *
//	 * If you would like to have multiple interrupts to happen, change
//	 * the COALESCING_COUNT to be a smaller value
//	 */
//	Status = XAxiDma_BdRingSetCoalesce(TxRingPtr, COALESCING_COUNT, DELAY_TIMER_COUNT);
//	if (Status != XST_SUCCESS) {
//
//		xil_printf("Failed set coalescing"
//		" %d/%d\r\n",COALESCING_COUNT, DELAY_TIMER_COUNT);
//		return XST_FAILURE;
//	}
//
//	/* Enable all TX interrupts */
//	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);
//
//	/* Start the TX channel */
//	Status = XAxiDma_BdRingStart(TxRingPtr);
//	if (Status != XST_SUCCESS) {
//
//		xil_printf("Failed bd start\r\n");
//		return XST_FAILURE;
//	}
//
//	return XST_SUCCESS;
//}

