void DataMemoryBarrier(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) );
}

void DataSyncBarrier(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) );
}

void FlushCache(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0) );
}
