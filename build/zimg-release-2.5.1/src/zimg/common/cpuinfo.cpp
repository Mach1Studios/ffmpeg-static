#include "cpuinfo.h"

#ifdef ZIMG_X86

#if defined(_MSC_VER)
  #include <intrin.h>
#elif defined(__GNUC__)
  #include <cpuid.h>
#endif

namespace zimg {
namespace {

/**
 * Execute the CPUID instruction.
 *
 * @param regs array to receive eax, ebx, ecx, edx
 * @param eax argument to instruction
 * @param ecx argument to instruction
 */
void do_cpuid(int regs[4], int eax, int ecx)
{
#if defined(_MSC_VER)
	__cpuidex(regs, eax, ecx);
#elif defined(__GNUC__)
	__cpuid_count(eax, ecx, regs[0], regs[1], regs[2], regs[3]);
#else
	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;
	regs[3] = 0;
#endif
}


/**
 * Execute the XGETBV instruction.
 *
 * @param ecx argument to instruction
 * @return (edx << 32) | eax
 */
unsigned long long do_xgetbv(unsigned ecx)
{
#if defined(_MSC_VER)
	return _xgetbv(ecx);
#elif defined(__GNUC__)
	unsigned eax, edx;
	__asm("xgetbv" : "=a"(eax), "=d"(edx) : "c"(ecx) : );
	return (static_cast<unsigned long long>(edx) << 32) | eax;
#else
	return 0;
#endif
}

} // namespace


X86Capabilities query_x86_capabilities() noexcept
{
	X86Capabilities caps = { 0 };
	unsigned long long xcr0 = 0;
	int regs[4] = { 0 };

	do_cpuid(regs, 1, 0);
	caps.sse   = !!(regs[3] & (1 << 25));
	caps.sse2  = !!(regs[3] & (1 << 26));
	caps.sse3  = !!(regs[2] & (1 << 0));
	caps.ssse3 = !!(regs[2] & (1 << 9));
	caps.fma   = !!(regs[2] & (1 << 12));
	caps.sse41 = !!(regs[2] & (1 << 19));
	caps.sse42 = !!(regs[2] & (1 << 20));

	// osxsave
	if (regs[2] & (1 << 27))
		xcr0 = do_xgetbv(0);

	// XMM and YMM state.
	if ((xcr0 & 0x06) != 0x06)
		return caps;

	caps.avx   = !!(regs[2] & (1 << 28));
	caps.f16c  = !!(regs[2] & (1 << 29));

	do_cpuid(regs, 7, 0);
	caps.avx2  = !!(regs[1] & (1 << 5));

	return caps;
}

bool cpu_has_fast_f16(CPUClass cpu) noexcept
{
	// Although F16C is supported on Ivy Bridge, the latency penalty is too great before Haswell.
	if (cpu == CPUClass::AUTO) {
		X86Capabilities caps = query_x86_capabilities();
		return caps.fma && caps.f16c && caps.avx2;
	} else {
		return cpu >= CPUClass::X86_AVX2;
	}
}

} // namespace zimg

#else // ZIMG_X86

namespace zimg {

bool cpu_has_fast_f16(CPUClass) noexcept { return false; }

} // namespace zimg

#endif // ZIMG_X86
