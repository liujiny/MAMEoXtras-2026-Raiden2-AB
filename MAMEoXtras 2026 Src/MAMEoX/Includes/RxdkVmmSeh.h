#ifndef MAMEOX_RXDK_VMM_SEH_H
#define MAMEOX_RXDK_VMM_SEH_H

/* RXDK GNU-ABI x86 VMM exception scope.
 * Include the platform EXCEPTION_RECORD / EXCEPTION_POINTERS types first.
 * This deliberately implements only the existing VMM filter's two outcomes:
 * resume the faulting instruction, or continue the OS exception search.
 * It is not a general replacement for an __except execute-handler block.
 * No MSVC C++ ABI or _except_handler3 runtime is required.
 */
#if !defined(__clang__) || !defined(__i386__)
#error RxdkVmmSeh requires RXDK Clang targeting 32-bit x86
#endif

#ifndef EXCEPTION_CONTINUE_EXECUTION
#define EXCEPTION_CONTINUE_EXECUTION (-1)
#endif
#ifndef EXCEPTION_CONTINUE_SEARCH
#define EXCEPTION_CONTINUE_SEARCH 0
#endif

namespace mameox_rxdk {
typedef int (__cdecl *VmmFilter)(PEXCEPTION_POINTERS);
struct VmmFrame;
typedef int (__cdecl *FrameHandler)(PEXCEPTION_RECORD, VmmFrame *, void *, void *);
struct VmmFrame {
    VmmFrame *previous;
    FrameHandler handler;
    VmmFilter filter;
    volatile unsigned int handling;
};
static_assert(sizeof(void *) == 4, "Xbox SEH registration is 32-bit");
static_assert(__builtin_offsetof(VmmFrame, handler) == 4, "SEH handler offset");

inline VmmFrame *CurrentFrame() noexcept {
    VmmFrame *p;
    __asm__ __volatile__("movl %%fs:0, %0" : "=r"(p) : : "memory");
    return p;
}
inline void SetFrame(VmmFrame *p) noexcept {
    __asm__ __volatile__("movl %0, %%fs:0" : : "r"(p) : "memory");
}

/* The OS dispatcher expects EXCEPTION_DISPOSITION, not a filter result:
 * ExceptionContinueExecution = 0, ExceptionContinueSearch = 1.
 */
static int __cdecl Dispatch(PEXCEPTION_RECORD exception, VmmFrame *frame,
                           void *context, void *) noexcept {
    const unsigned int cannotResume = 0x01u | 0x02u | 0x04u | 0x08u | 0x20u | 0x40u;
    if (!exception || !frame || !frame->filter || frame->handling ||
        (exception->ExceptionFlags & cannotResume))
        return 1;
    EXCEPTION_POINTERS pointers;
    pointers.ExceptionRecord = exception;
    pointers.ContextRecord = reinterpret_cast<decltype(pointers.ContextRecord)>(context);
    frame->handling = 1;
    const int result = frame->filter(&pointers);
    frame->handling = 0;
    return result == EXCEPTION_CONTINUE_EXECUTION ? 0 : 1;
}

class VmmScope {
    VmmFrame frame_;
public:
    explicit VmmScope(VmmFilter filter) noexcept {
        frame_.previous = CurrentFrame();
        frame_.handler = &Dispatch;
        frame_.filter = filter;
        frame_.handling = 0;
        SetFrame(&frame_);
    }
    ~VmmScope() noexcept {
        // Also runs if a GNU-ABI C++ exception unwinds this lexical scope.
        SetFrame(frame_.previous);
    }
    VmmScope(const VmmScope &) = delete;
    VmmScope &operator=(const VmmScope &) = delete;
};
}
#endif
