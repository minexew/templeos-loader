#include "host.h"
#include "thunk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t HostGetApiVersion(void) {
    // 0xMMMMmmmm
    // MAJOR = breaking changes
    // minor = backwards-compatible changes
    return 0x00010000;
}

static const char* HostGetEnv(const char* name) {
    return getenv(name);
}

// Generate thunks for translating TempleOS ABI to SysV

MAKE_THUNK0(HostGetApiVersion)
MAKE_THUNK1(HostGetEnv)

// Functions are currently exported as __host_GetEnv etc.,
// at least until we figure out if we can directly re-export kernel-imported symbols for JIT
#define ENTRY(name_) else if (!strcmp(name, "__host_" #name_)) { return (void*) &thunk_Host##name_; }

// TODO: tabularize this
void* host_get_API_by_name(const char* name) {
    if (0) {}
    ENTRY(GetApiVersion)
    ENTRY(GetEnv)

    return 0;
}
