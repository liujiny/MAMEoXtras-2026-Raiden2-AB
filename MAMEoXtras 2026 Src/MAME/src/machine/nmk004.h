#pragma code_seg("C531")
#pragma data_seg("D531")
#pragma bss_seg("B531")
#pragma const_seg("K531")
#pragma comment(linker, "/merge:D531=531")
#pragma comment(linker, "/merge:C531=531")
#pragma comment(linker, "/merge:B531=531")
#pragma comment(linker, "/merge:K531=531")
void NMK004_init(void);
void NMK004_irq(int irq);
READ16_HANDLER( NMK004_r );
WRITE16_HANDLER( NMK004_w );

#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()