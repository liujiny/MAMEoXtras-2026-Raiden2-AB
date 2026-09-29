/* Host-only 32-bit Windows SEH smoke test. Not an Xbox executable. */
typedef unsigned int U32;
struct _EXCEPTION_RECORD {
    U32 ExceptionCode, ExceptionFlags;
    _EXCEPTION_RECORD *ExceptionRecord;
    void *ExceptionAddress;
    U32 NumberParameters;
    U32 ExceptionInformation[15];
};
typedef _EXCEPTION_RECORD *PEXCEPTION_RECORD;
struct EXCEPTION_POINTERS { PEXCEPTION_RECORD ExceptionRecord; void *ContextRecord; };
typedef EXCEPTION_POINTERS *PEXCEPTION_POINTERS;
#include "../source/MAMEoX/Includes/RxdkVmmSeh.h"
extern "C" {
__declspec(dllimport) void *__stdcall VirtualAlloc(void *, U32, U32, U32);
__declspec(dllimport) int __stdcall VirtualFree(void *, U32, U32);
__declspec(dllimport) void __stdcall RaiseException(U32, U32, U32, const U32 *);
__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(U32);
}
static volatile U32 *page;
static volatile int pageFaults, continued, rejected;
static int __cdecl filter(PEXCEPTION_POINTERS p) {
    const _EXCEPTION_RECORD *r = p->ExceptionRecord;
    if (r->ExceptionCode == 0xe04d414dU) { ++continued; return -1; }
    if (r->ExceptionCode != 0xc0000005U || r->NumberParameters < 2 ||
        r->ExceptionInformation[1] < reinterpret_cast<U32>(page) ||
        r->ExceptionInformation[1] >= reinterpret_cast<U32>(page) + 4096)
        return 0;
    if (!VirtualAlloc(const_cast<U32 *>(page), 4096, 0x1000, 0x04)) return 0;
    ++pageFaults;
    return -1;
}
static int __cdecl reject(PEXCEPTION_POINTERS) { ++rejected; return 0; }
extern "C" __declspec(noreturn) void mainCRTStartup() {
    auto *original = mameox_rxdk::CurrentFrame();
    page = static_cast<volatile U32 *>(VirtualAlloc(0, 4096, 0x2000, 0x01));
    if (!page) ExitProcess(10);
    {
        mameox_rxdk::VmmScope outer(filter);
        auto *outerFrame = mameox_rxdk::CurrentFrame();
        *page = 0x11223344u; // demand commit: handler must resume this very write
        if (*page != 0x11223344u || pageFaults != 1) ExitProcess(11);
        {
            mameox_rxdk::VmmScope inner(reject);
            RaiseException(0xe04d414dU, 0, 0, 0); // search inner, continue outer
        }
        if (mameox_rxdk::CurrentFrame() != outerFrame || continued != 1 || rejected != 1)
            ExitProcess(12);
        if (!VirtualFree(const_cast<U32 *>(page), 4096, 0x4000)) ExitProcess(13);
        *page = 0x55667788u; // second fault after decommit
        if (*page != 0x55667788u || pageFaults != 2) ExitProcess(14);
        _EXCEPTION_RECORD record = {};
        record.ExceptionCode = 0xc0000005u;
        record.ExceptionFlags = 2; // unwind must never call the demand-pager
        if (mameox_rxdk::Dispatch(&record, outerFrame, 0, 0) != 1 || pageFaults != 2)
            ExitProcess(15);
    }
    if (mameox_rxdk::CurrentFrame() != original) ExitProcess(16);
    if (!VirtualFree(const_cast<U32 *>(page), 0, 0x8000)) ExitProcess(17);
    ExitProcess(0);
}
