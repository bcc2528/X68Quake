#include <stddef.h>

void *himem_malloc(size_t size);
int himem_free(void* ptr);
size_t himem_getsize(void);
int himem_resize(void* ptr, size_t size);