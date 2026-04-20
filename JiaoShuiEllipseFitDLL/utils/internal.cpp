// internal.cpp - Simplified Self-Hash Implementation
#include "internal.h"
#if CHECK_EXPIRED
#include <windows.h>
#include <wincrypt.h>
#include <array>
#include <fstream>
#include <intrin.h>

#pragma comment(lib, "advapi32.lib")

#define BUILD_GENCRC   0 // Set to 1 to generate hash, 0 to verify

#if !BUILD_GENCRC
#include "security_hash.h"  // Only included for release builds
#endif

__forceinline bool IsDebuggerPresentEx()
{
    __try {
        unsigned __int64 start = __rdtsc();
        Sleep(10);
        return (__rdtsc() - start) <= 1000000;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

__declspec(noinline) bool CheckMemoryConsistency()
{
    __try {
        BYTE b1 = *(BYTE*)&Security::PerformSecurityChecks;
        BYTE b2 = *((BYTE*)&Security::PerformSecurityChecks + 1);
        return (b1 == 0x40 && b2 == 0x55) ||
            (b1 == 0x48 && (b2 == 0x83 || b2 == 0x89 || b2 == 0x8B));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Security::VerifyCodeIntegrity()
{
    auto* start = reinterpret_cast<BYTE*>(&foo3282_check);
    auto* end = start;// reinterpret_cast<BYTE*>(&crc_end);

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE hash[32] = {};
    DWORD hashLen = sizeof(hash);

    while ((end - start) < 512) {
        if (*end == 0xC3 || *end == 0xC2) {
            ++end;
            break;
        }
        ++end;
    }
    ++end; // include RET
    size_t size = end - start;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        return false;
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
        return false;
    if (!CryptHashData(hHash, start, size, 0))
        return false;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0))
        return false;

    if (hHash)
        CryptDestroyHash(hHash);
    if (hProv)
        CryptReleaseContext(hProv, 0);
    bool res = true;
#if BUILD_GENCRC
    char outStr[256];
    sprintf_s(outStr, "Start: %p, End: %p, Size: %d\n", start, end, (int)size);
    OutputDebugStringA(outStr);
    OutputDebugStringA("Hash: { ");
    for (DWORD i = 0; i < hashLen; ++i) {
        sprintf_s(outStr, "0x%02X%s", hash[i], (i < hashLen - 1) ? ", " : " ");
        OutputDebugStringA(outStr);
    }
    OutputDebugStringA("}\n");
#else
    res = (memcmp(hash, EXPECTED_HASH.data(), hashLen) != 0);
#endif
    return res;
}

void Security::AntiTamperShutdown()
{
    __try {
        volatile int* p = 0;
        *p = 0xDEAD;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        TerminateProcess(GetCurrentProcess(), 0xDEAD);
    }
}

bool Security::PerformSecurityChecks()
{
    __try {
        if (IsDebuggerPresentEx() ||
            !VerifyCodeIntegrity() ||
            !foo3282_check() ||
            !CheckMemoryConsistency()) {
            AntiTamperShutdown();
            return false;
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        AntiTamperShutdown();
        return false;
    }
}
#endif
//.EOF