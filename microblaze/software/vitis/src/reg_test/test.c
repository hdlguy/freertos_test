#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"

int main()
{
    init_platform();

    print("testing AXI register file\n\r");

    uint32_t *regptr;
    regptr = (uint32_t *) XPAR_M06_AXI_BASEADDR;

    for (int i=0; i<16; i++) xil_printf("reg[%d] = 0x%08x\n\r", i, regptr[i]);
    for (int i=0; i<16; i++) regptr[i] = i;
    for (int i=0; i<16; i++) xil_printf("reg[%d] = 0x%08x\n\r", i, regptr[i]);

    cleanup_platform();
    return 0;
}
