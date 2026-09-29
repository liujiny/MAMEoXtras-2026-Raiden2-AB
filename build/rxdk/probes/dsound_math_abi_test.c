/* Host-only calling-convention test. These synthetic math functions are
 * NEVER linked into the Xbox program; they make arguments/results observable.
 */
extern __declspec(dllimport) __attribute__((noreturn)) void __stdcall ExitProcess(unsigned int);
extern double __stdcall test_sin(double) __asm__("_sin@8");
extern double __stdcall test_sinh(double) __asm__("_sinh@8");
extern double __stdcall test_pow(double, double) __asm__("_pow@16");

double __attribute__((noinline)) sin(double x) { return x + 2.0; }
double __attribute__((noinline)) sinh(double x) { return x + 3.0; }
double __attribute__((noinline)) pow(double x, double y) { return x * y; }

static unsigned int stack_pointer(void)
{
    unsigned int value;
    __asm__ __volatile__("movl %%esp, %0" : "=r"(value) : : "memory");
    return value;
}
void __attribute__((noreturn)) mainCRTStartup(void)
{
    int i;
    const unsigned int before = stack_pointer();
    for (i = 0; i < 10000; ++i) {
        volatile double a = 0.25;
        volatile double b = 8.0;
        if (test_sin(a) != 2.25) ExitProcess(31);
        if (test_sinh(a) != 3.25) ExitProcess(32);
        if (test_pow(a, b) != 2.0) ExitProcess(33);
        if (stack_pointer() != before) ExitProcess(34);
    }
    ExitProcess(0);
}
