#include "host.h"
#include "thunk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__attribute__((used)) static uint32_t HostGetApiVersion(void) {
    // 0xMMMMmmmm
    // MAJOR = breaking changes
    // minor = backwards-compatible changes
    return 0x00010000;
}

__attribute__((used)) static const char* HostGetEnv(const char* name) {
    return getenv(name);
}

__attribute__((used)) static void HostVideoPresent(uint8_t const* buffer) {
    FILE* screendump = fopen("/tmp/screen.txt", "wb");

    if (!screendump) return;

    enum { VIDEOCOLS = 80 };
    enum { VIDEOROWS = 25 };

    fprintf(stderr, "HostVideoPresent\n");

//    printf("\n");
    for (int r = 0; r < VIDEOROWS; r++) {
        for (int i = 0; i < VIDEOCOLS; i++) {
//            printf("%c", buffer[(r * VIDEOCOLS + i) * 2]);
            uint8_t attr = buffer[(r * VIDEOCOLS + i) * 2 + 1];
            uint8_t cmap[] = {0, 4, 2, 6, 1, 5, 3, 7};
            int bg_code = ((attr & 0x80) ? 100 : 40) + cmap[(attr & 0x70) >> 4];
            int fg_code = ((attr & 0x08) ? 90 : 30) + cmap[attr & 0x07];
            // 16-color mode
            //fprintf(screendump, "\x1b[%d;%dm", fg_code, bg_code);
            // 256-color mode
            //fprintf(screendump, "\x1b[38;5;%d;48;5;%dm", cmap[attr & 0x07], cmap[(attr & 0x70) >> 4]);
            // 24-bit mode
            fprintf(screendump, "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                    (attr & 0x04) ? 255 : 0, (attr & 0x02) ? 255 : 0, (attr & 0x01) ? 255 : 0,
                    (attr & 0x40) ? 255 : 0, (attr & 0x20) ? 255 : 0, (attr & 0x10) ? 255 : 0
            );
            fwrite(&buffer[(r * VIDEOCOLS + i) * 2], 1, 1, screendump);
        }
//        printf("\n");
//        char c = '\n';
        fprintf(screendump, "\x1b[0m\n");
    }
//    printf("\n");
    fclose(screendump);
    usleep(20*1000);
    // then to view: iconv -f CP437 -t UTF-8 /tmp/screen.txt | sed 's/\x0/ /g'
}

// Generate thunks for translating TempleOS ABI to SysV

MAKE_THUNK0(HostGetApiVersion)
MAKE_THUNK1(HostGetEnv)
MAKE_THUNK1(HostVideoPresent)

// Functions are currently exported as __host_GetEnv etc.,
// at least until we figure out if we can directly re-export kernel-imported symbols for JIT
#define ENTRY(name_) else if (!strcmp(name, "__host_" #name_)) { return (void*) &thunk_Host##name_; }

// TODO: tabularize this
void* host_get_API_by_name(const char* name) {
    if (0) {}
    ENTRY(GetApiVersion)
    ENTRY(GetEnv)
    ENTRY(VideoPresent)

    return 0;
}
