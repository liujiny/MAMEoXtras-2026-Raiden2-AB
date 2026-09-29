#pragma code_seg("CC55")
#pragma data_seg("CD55")
#pragma bss_seg("CB55")
#pragma const_seg("CK55")
#pragma comment(linker, "/merge:CD55=CPU55")
#pragma comment(linker, "/merge:CC55=CPU55")
#pragma comment(linker, "/merge:CB55=CPU55")
#pragma comment(linker, "/merge:CK55=CPU55")

#include "m37710cm.h"
#define EXECUTION_MODE EXECUTION_MODE_M0X1
#include "m37710op.h"

#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()
