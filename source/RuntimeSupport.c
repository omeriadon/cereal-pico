#include <errno.h>
#include <stdlib.h>

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    (void)alignment;

    if (memptr == NULL) {
        return EINVAL;
    }

    void *memory = malloc(size);
    if (memory == NULL) {
        return ENOMEM;
    }

    *memptr = memory;
    return 0;
}
