#include "datetime.h"
#include "host.h"
#include "thunk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t HostGetApiVersion(void) {
    // 0xMMMMmmmm
    // MAJOR = breaking changes
    // minor = backwards-compatible changes
    return 0x00030000;
}

const char* HostGetEnv(const char* name) {
    return getenv(name);
}

void HostGetLocalTime(struct CDateStruct* ds_out) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timespec_to_CDateStruct_local(ts.tv_sec, ts.tv_nsec, ds_out);
}

// Generate thunks for translating TempleOS ABI to SysV
// (only needed for dynamic loading)

MAKE_THUNK0(HostGetApiVersion)
MAKE_THUNK1(HostGetEnv)
MAKE_THUNK1(HostGetLocalTime)

// Functions are exported as HostGetEnv etc.
#define ENTRY(name_) else if (!strcmp(name, #name_)) { return (void*) &thunk_##name_; }

// TODO: tabularize this, it will be needed for HostGetApiFunctions anyway
void* host_get_API_by_name(const char* name) {
    if (0) {}
    ENTRY(HostGetApiVersion)
    ENTRY(HostGetEnv)
    ENTRY(HostGetLocalTime)

    return 0;
}
