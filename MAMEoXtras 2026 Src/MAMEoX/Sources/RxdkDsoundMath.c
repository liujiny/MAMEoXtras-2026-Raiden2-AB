/* RXDK 1.3.2 libdsound's dsapi object imports these math functions using
 * stdcall names. picolibc correctly exports the C math API as cdecl.
 * Do not rely on ld.lld's name-only stdcall fixup: provide real bridges
 * which restore the 8/16-byte argument stack exactly as the caller expects.
 * The assembler names below are the exact COFF imports, not C aliases.
 */
#if !defined(__clang__) || !defined(__i386__)
#error This RXDK compatibility unit requires 32-bit x86 Clang
#endif

#include <math.h>

double __attribute__((stdcall)) RxdkDsoundSin(double x) __asm__("_sin@8");
double __attribute__((stdcall)) RxdkDsoundSinh(double x) __asm__("_sinh@8");
double __attribute__((stdcall)) RxdkDsoundPow(double x, double y) __asm__("_pow@16");

double __attribute__((stdcall)) RxdkDsoundSin(double x)
{
    return sin(x);
}

double __attribute__((stdcall)) RxdkDsoundSinh(double x)
{
    return sinh(x);
}

double __attribute__((stdcall)) RxdkDsoundPow(double x, double y)
{
    return pow(x, y);
}
