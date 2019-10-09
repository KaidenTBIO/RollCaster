#include <string.h>
#include <stdio.h>

#include "state.h"
#include "dll.h"

//#define USE_GAME_ALLOC

// *********************************************** Buffer management

void State::enlarge(unsigned int size) {
	if (size < m_max_size) {
		return;
	}
	
	unsigned char *buffer;

#ifdef USE_GAME_ALLOC
	buffer = (unsigned char *)game_alloc(size);
#else
	buffer = new unsigned char[size];
#endif

	if (m_buffer) {
		if (m_size > 0) {
			memcpy(buffer, m_buffer, m_size);
		}
		
#ifdef USE_GAME_ALLOC
		game_free(m_buffer);
#else
		delete[] m_buffer;
#endif
	}
	
	m_buffer = buffer;
	m_max_size = size;
}

void State::flush() {
	if (m_buffer) {
#ifdef USE_GAME_ALLOC
		game_free(m_buffer);
#else
		delete[] m_buffer;
#endif
	}
	
#ifdef USE_GAME_ALLOC
	m_buffer = (unsigned char *)game_alloc(32768); 
#else
	m_buffer = new unsigned char[32768];
#endif
	m_max_size = 32768;
	
	m_size = 0;
	m_ptr = 0;
}

void State::add(const void *data, unsigned int size) {
	unsigned int new_size = m_size + size;
	
	while (new_size >= m_max_size) {
		enlarge(m_max_size * 2);
	}
	
	unsigned char *buf = m_buffer + m_size;
	memcpy(buf, data, size);
	
	m_size = new_size;
}

void State::get(void *data, unsigned int size) {
	memcpy(data, m_buffer + m_ptr, size);
	
	m_ptr += size;
}

void State::add_int(int value) {
	add(&value, 4);
}

int State::get_int() {
	int value;
	
	get(&value, 4);
	
	return value;
}

bool State::valid() {
	return m_size > 0;
}


// *********************************************** linked list storage
void State::store_list(const unsigned int *root, unsigned int entry_size) {
	const unsigned int *item = root;
	
	int count = 0;
	
	do {
		add_int((unsigned int)item);
		
		++count;
		
		add(item+2, entry_size-8);
		
		item = *(const unsigned int **)item;
	} while (item != root);
	add_int(0);
}

void State::restore_list(unsigned int *root, unsigned int entry_size) {
	// create our new list, ignoring the old.
	unsigned int **item = (unsigned int **)root;
	
	unsigned int *address = (unsigned int *)get_int();
	if (address) {
		cache_use(entry_size, (unsigned char *)address);
		
		get(item+2, entry_size-8);
		
		while ((address = (unsigned int *)get_int()) != 0) {
			unsigned int *new_item = address;
			cache_use(entry_size, (unsigned char *)new_item);
			
			get(new_item+2, entry_size-8);
		
			new_item[1] = (unsigned int)item;
			item[0] = (unsigned int *)new_item;
		
			item = (unsigned int **)new_item;
		}
	}
	
	item[0] = root;
	root[1] = (unsigned int)item;
}


// *********************************************** map storage
// I know it's not a map, but that's what I'm calling it.
// It's actually std::deque<>
struct th075_map_t {
	unsigned int	zero;
	unsigned int	***data;
	unsigned int	divisions;
	unsigned int	start;
	unsigned int	total;
};

void State::store_map(const unsigned int *data, unsigned int entry_size) {
	if (!data) {
		add_int(0);
		return;
	}
	
	th075_map_t *map = (th075_map_t *)data;

	int count = map->total;
	
	add_int(count);
	
	if (!count) {
		return;
	}
	
	// store all entries
	unsigned int ***m = map->data;
	unsigned int ***m_end = map->data + map->divisions;
	
	int start = map->start;

	m += start/4;
	start &= 3;
	
	for (; count > 0; ++m) {
		if (m == m_end) {
			m = map->data;
		}
		
		unsigned int **n = *m;
		for (int i = start; i < 4 && count > 0; ++i, --count) {
			if (n[i]) {
				add_int((unsigned int)n[i]);
				
				add(n[i], entry_size);
			} else {
				add_int(0);
			}
		}
		
		start = 0;
	}
}

void State::restore_map(unsigned int *data, unsigned int entry_size) {
	unsigned int new_count = get_int();
	
	th075_map_t *map = (th075_map_t *)data;
	
	// place our new entries in
	unsigned int ***m = map->data;
	
	map->start = 0;
	map->total = new_count;
	
	for (; new_count > 0; ++m) {
		unsigned int **n = *m;
		if (!n) {
			n = (unsigned int **)game_alloc(16);
			m[0] = n;
			
			n[0] = 0;
			n[1] = 0;
			n[2] = 0;
			n[3] = 0;
		}
		
		int max = new_count > 4 ? 4 : new_count;
		new_count -= max;
		
		for (int i = 0; i < max; ++i) {
			unsigned int *address = (unsigned int *)get_int();
			if (address) {
				cache_use(entry_size, (unsigned char *)address);
				
				n[i] = address;
			
				get(n[i], entry_size);
			} else {
				n[i] = 0;
			}
		}
	}
}

// *********************************************** FPU data func

void State::store_fpu() {
	char data[108];
	
	asm volatile ("fnsave %0" : "=m" (data[0]));
	
	// FSAVE implicitly calls FINIT, so we must call FRSTOR to negate that.
	asm volatile ("frstor %0" :: "m" (data[0]));
	
	add((void *)data, 108);
}

void State::restore_fpu() {
	char data[108];
	
	get((void *)data, 108);
	
	asm volatile ("frstor %0" :: "m" (data[0]));
}

// *********************************************** Main state funcs

static const struct {
	int		player_id;
	unsigned int	address;
	unsigned int	size;
} character_table[11] = {
	{	0,	0x6590d8,	0x1000	},
	{	1,	0x6592b8,	0x1830	},
	{	2,	0x6594a8,	0x0FEC	},
	{	3,	0x659608,	0x0FEC	},	
	{	4,	0x659768,	0x0FE8	},
	{	5,	0x6598d0,	0x0FF8	},
	{	6,	0x659a40,	0x0FE4	},
	{	7,	0x659b80,	0x0FE8	},
	{	8,	0x659cc0,	0x1008	},
	{	9,	0x659e30,	0x1000	},
	{	10,	0x659f50,	0x1000	}
};

static const struct {
	unsigned int	address;
	unsigned int	size;
} background_table[34] = {
	{	0x658434,	0x00ec	},	// reimu-day
	{	0x6584a4,	0x00f0	},	// reimu-night
	{	0x6585ec,	0x00ec	},	// marisa-day
	{	0x658634,	0x00ec	},	// marisa-night
	{	0x6586e8,	0x00f0	},	// sak-day
	{	0x65872c,	0x00f0	},	// sak-night
	{	0x658858,	0x00f0	},	// alice-day
	{	0x658894,	0x00f0	},	// alice-night
	{	0x658948,	0x00f0	},	// patch-day
	{	0x6589e4,	0x00f0	},	// patch-night
	{	0x658ac4,	0x00f0	},	// youmu-day
	{	0x658b24,	0x00f8	},	// youmu-night
	{	0x658bfc,	0x00f0	},	// remilia
	{	0x658c90,	0x00f0	},	// yuyuko
	{	0x6584fc,	0x00ec	},	// yukari-day
	{	0x658d38,	0x00f4	},	// yukari-night
	{	0x658dc0,	0x1d28	},	// suika. yes, really 1d28.
	{	0x658538,	0x0068	},	// reimu-day, simplified
	{	0x658574,	0x0068	},	// reimu-night, simplified
	{	0x658670,	0x0068	},	// marisa-day, simplified
	{	0x6586ac,	0x0068	},	// marisa-night, simplified
	{	0x6587a4,	0x00f0	},	// sakuya-day, simplified
	{	0x6587e0,	0x00f0	},	// sakuya-night, simplified
	{	0x6588d0,	0x00f0	},	// alice-day, simplified
	{	0x65890c,	0x00f0	},	// alice-night, simplified
	{	0x658a4c,	0x00f0	},	// patch-day, simplified
	{	0x658a88,	0x00f0	},	// patch-night, simplified
	{	0x658b84,	0x00ec	},	// youmu-day, simplified
	{	0x658bc0,	0x00f0	},	// youmu-night, simplified
	{	0x658c54,	0x00f0	},	// remilia, simplified
	{	0x658cfc,	0x00f0	},	// yuyuko, simplified
	{	0x6585b0,	0x0068	},	// yukari-day, simplified
	{	0x658d84,	0x00f0	},	// yukari-night, simplified
	{	0x658e54,	0x00f0	},	// suika, simplified
};

void State::store() {
	unsigned int *p1struct = *(unsigned int **) 0x671418;
	unsigned int *p2struct = *(unsigned int **) 0x67141C;
	
	if (!*p1struct || !*p2struct) {
		return;
	}
	
	if (!m_buffer) {
		enlarge(32768);
	}
	
	m_size = 0;
	
	unsigned int *effect_table;

	for (int n = 0; n < 2; ++n) {
		unsigned int *player;
		if (n == 0) {
			player = p1struct;
		} else {
			player = p2struct;
		}
		
		// figure out which player struct we are
		// this should be cached or sped up somehow
		unsigned int size = 0;
		int p_id = 0;
		for (; p_id < 11; ++p_id) {
			if (character_table[p_id].address == player[0]) {
				break;
			}
		}
		add_int(p_id);
		
		if (p_id < 11) {
			size = character_table[p_id].size;
		} else {
			size = 0x1000;
		}
		
		add(player, size);
		
		// store effect data
		effect_table = *(unsigned int **)(player + (0x2ec/4));
		store_map(effect_table + 1, 0xe8);
		store_map(effect_table + 6, 0xe8);
		store_map(effect_table + 11, 0xe8);
		store_map(effect_table + 16, 0xe8);
		
		// store input buffer from 0x514
		effect_table = *(unsigned int **)(player + (0x51c/4));
		for (int i = 0; i < 8; ++i) {
			if (effect_table[i]) {
				add_int(1);
				add((void *)effect_table[i], 16);
			} else {
				add_int(0);
			}
		}
		
		// store 0x480 effect list
		effect_table = ((unsigned int **)player)[0x480/4];
		add(effect_table, 0x18);
		
		store_list(((unsigned int **)effect_table)[2], 0xac);
		
		// youmu has an extra list, for clones
		if (p_id == 5) {
			store_list((unsigned int *)(player[0xff0/4]), 0x18);
		}
	}
	
	// store system effect table
	store_map((unsigned int *)0x6716c8, 0x94);
	store_map((unsigned int *)0x6716dc, 0x94);
	
	// store system data
	add((void *)0x6713c0, 0x58);
	
	// misc render data
	add(render_address, 0x11c);
	add(((char *)render_address[0x50/4])+4, 0x10);
	add(((char *)render_address[0x54/4])+4, 0x10);
	
	// background graphics
	unsigned int *bg_address = *(unsigned int **)0x671424;
	unsigned int bg_vtable = *bg_address;
	unsigned int bg_size = 0xec;
	unsigned int bg_id;
	
	for (bg_id = 0; bg_id < 34; ++bg_id) {
		if (background_table[bg_id].address == bg_vtable) {
			bg_size = background_table[bg_id].size;
			break;
		}
	}
	add_int(bg_id);
	
	add(*(unsigned int **)0x671424, bg_size);

	// replay data
	add((void *)0x6718b0, 4);
	add((void *)0x6718b4, 4);
	add((void *)0x6718b8, 4);
	
	add((void *)0x6718c4, 4);
	add((void *)0x6718c8, 4);
	add((void *)0x6718d8, 4);
	add((void *)0x6718dc, 4);
	
	// caster rng
	add((void *)0x66c0c6, 4);
	
	// FPU state
	store_fpu();
}

void State::restore() {
	unsigned int *p1struct = *(unsigned int **) 0x671418;
	unsigned int *p2struct = *(unsigned int **) 0x67141C;
	
	if (!*p1struct || !*p2struct || !render_address) {
		return;
	}
	
	if (!m_size) {
		return;
	}
	
	cache_flush();
	
	m_ptr = 0;
	
	unsigned int *effect_table;
	
	for (int n = 0; n < 2; ++n) {
		unsigned int *player;
		if (n == 0) {
			player = p1struct;
		} else {
			player = p2struct;
		}
		
		// get which player struct we are
		int p_id = get_int();
		unsigned int size;
		
		if (p_id < 11) {
			size = character_table[p_id].size;
		} else {
			size = 0x1000;
		}
		
		get(player, size);
		
		// restore effects
		effect_table = *(unsigned int **)(player + (0x2ec/4));
		restore_map(effect_table + 1, 0xe8);
		restore_map(effect_table + 6, 0xe8);
		restore_map(effect_table + 11, 0xe8);
		restore_map(effect_table + 16, 0xe8);
		
		// restore input buffer
		effect_table = *(unsigned int **)(player + (0x51c/4));
		for (int i = 0; i < 8; ++i) {
			int n = get_int();
			if (n != 0) {
				if (!effect_table[i]) {
					effect_table[i] = (unsigned int)game_alloc(16);
				}
				get((void *)(effect_table[i]), 16);
			} else {
				if (effect_table[i]) {
					game_free((unsigned int *)effect_table[i]);
					effect_table[i] = 0;
				}
			}
		}
		
		// restore 0x480 effect list
		effect_table = ((unsigned int **)player)[0x480/4];
		
		get(effect_table, 0x18);
		
		restore_list(((unsigned int **)effect_table)[2], 0xac);

		// youmu has an extra list, for clones
		if (p_id == 5) {
			restore_list((unsigned int *)(player[0xff0/4]), 0x18);
		}
	}
	
	// restore system effects
	restore_map((unsigned int *)0x6716c8, 0x94);
	restore_map((unsigned int *)0x6716dc, 0x94);
	
	// restore all misc system data
	get((void *)0x6713c0, 0x58);
	
	// misc render data
	// keep fps value as it uses ours instead of the game's
	unsigned int fps = render_address[0x20/4];
	get(render_address, 0x11c);
	render_address[0x20/4] = fps;
	
	get(((char *)render_address[0x50/4])+4, 0x10);
	get(((char *)render_address[0x54/4])+4, 0x10);
	
	// background graphics
	unsigned int bg_id = get_int();
	unsigned int bg_size = 0xec;
	
	if (bg_id < 34) {
		bg_size = background_table[bg_id].size;
	}
	
	get(*(unsigned int **)0x671424, bg_size);

	// replay data
	*(unsigned int *)0x6718b0 = get_int();
	*(unsigned int *)0x6718b4 = get_int();
	*(unsigned int *)0x6718b8 = get_int();

	*(unsigned int *)0x6718c4 = get_int();
	*(unsigned int *)0x6718c8 = get_int();
	*(unsigned int *)0x6718d8 = get_int();
	*(unsigned int *)0x6718dc = get_int();
	
	// caster rng
	*(unsigned int *)0x66c0c6 = get_int();
	
	// FPU state
	restore_fpu();
}

// *********************************************** Constructor/deconstructor

State::State() {
	m_buffer = 0;
	m_max_size = 0;
	m_size = 0;
	m_ptr = 0;
}

State::~State() {
#ifdef USE_GAME_ALLOC
	game_free(m_buffer);
#else
	delete[] m_buffer;
#endif
}
