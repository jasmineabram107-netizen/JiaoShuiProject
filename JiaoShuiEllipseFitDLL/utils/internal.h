#ifndef __DROPLETLIB_INTERBAL_H__
#define __DROPLETLIB_INTERBAL_H__

#define CHECK_EXPIRED 0 // Set to 1 to enable expiration check

#if CHECK_EXPIRED
#define ENCODED_DEAD_DATE  (20271201 ^ MAGIC_SEED)

extern "C" __declspec(noinline) void _security_check_start();
__declspec(noinline) bool foo3282_check();
extern "C" __declspec(noinline) void _security_check_end();

#define foo3381_check Security::PerformSecurityChecks

#define SECURITY_MODULE_EXPORT __declspec(dllexport)
namespace Security {
    SECURITY_MODULE_EXPORT bool PerformSecurityChecks();
    SECURITY_MODULE_EXPORT bool VerifyCodeIntegrity();
    SECURITY_MODULE_EXPORT void AntiTamperShutdown();
}
#else
inline bool foo3381_check() {
    return true;
}
#endif

#endif//__DROPLETLIB_INTERBAL_H__