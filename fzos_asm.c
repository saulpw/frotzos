
void yield()
{
    asm volatile ("hlt");
}

void halt()
{
    asm volatile ("1: hlt; jmp 1b");
}

unsigned long long rdtsc(void)
{
    unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}


