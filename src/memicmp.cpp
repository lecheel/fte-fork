#include <stddef.h>
#include <ctype.h>
#include "sysdep.h"

#ifdef __cplusplus
extern "C"
#endif
int memicmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            int c = tolower(p1[i]) - tolower(p2[i]);
            if (c != 0)
                return c;
        }
    }
    return 0;
}

