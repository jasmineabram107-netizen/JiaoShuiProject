#include "internal.h"

#if CHECK_EXPIRED

#include <Windows.h>
#define  MAGIC_SEED  0x159A3D2B
#define MULTIPLIER  400

extern "C" __declspec(noinline) void _security_check_start()
{
    volatile int dummy = 0x1234;  // prevent empty function
    (void)dummy;
}

#pragma optimize("", off)
// Consistent encoding function using same XOR logic
__forceinline int encode_date(int year, int month, int day)
{
    int raw = year * 10000 + month * 100 + day;
    return raw ^ MAGIC_SEED;
}

__declspec(noinline) bool foo3282_check()
{
    SYSTEMTIME s;
    GetLocalTime(&s);

    int now_encoded = encode_date(s.wYear, s.wMonth, s.wDay);
    if ((now_encoded ^ MAGIC_SEED) > (ENCODED_DEAD_DATE ^ MAGIC_SEED)) {
        // Expired
        return false;
    }

    return true; // Not expired
}
#pragma optimize("", on)

extern "C" __declspec(noinline) void _security_check_end()
{
    volatile int dummy = 0x5678;
    (void)dummy;
}
#endif//CHECK_EXPIRED