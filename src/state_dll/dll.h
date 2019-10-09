#ifndef DLL_H
#define DLL_H

typedef void * (*game_alloc_t)(unsigned int size);
typedef void (*game_free_t)(void *addr);

static const game_alloc_t game_alloc = (game_alloc_t)0x6416a2;
static const game_free_t game_free = (game_free_t)0x64169d;

extern unsigned int *render_address;

extern unsigned char *cache_allocate(unsigned int size);
extern void cache_use(unsigned int size, unsigned char *address);
extern void cache_free(unsigned char *address, unsigned int size);
extern void cache_flush();

#endif
