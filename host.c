#include "datetime.h"
#include "host.h"
#include "thunk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

__attribute__((used)) static uint32_t HostGetApiVersion(void) {
    // 0xMMMMmmmm
    // MAJOR = breaking changes
    // minor = backwards-compatible changes
    return 0x00020000;
}

__attribute__((used)) static const char* HostGetEnv(const char* name) {
    return getenv(name);
}

__attribute__((used)) static void HostGetLocalTime(struct CDateStruct* ds_out) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timespec_to_CDateStruct_local(ts.tv_sec, ts.tv_nsec, ds_out);
}

// Generate thunks for translating TempleOS ABI to SysV

MAKE_THUNK0(HostGetApiVersion)
MAKE_THUNK1(HostGetEnv)
MAKE_THUNK1(HostGetLocalTime)

// Functions are currently exported as __host_GetEnv etc.,
// at least until we figure out if we can directly re-export kernel-imported symbols for JIT
#define ENTRY(name_) else if (!strcmp(name, "__host_" #name_)) { return (void*) &thunk_Host##name_; }

// TODO: tabularize this
void* host_get_API_by_name(const char* name) {
    if (0) {}
    ENTRY(GetApiVersion)
    ENTRY(GetEnv)
    ENTRY(GetLocalTime)

    return 0;
}
