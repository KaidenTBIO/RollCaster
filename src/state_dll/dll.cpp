#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "dll.h"
#include "state.h"

/*

 Explanation of partial wait timing system:
 
 ---
 
 Normal frame:
 
 0ms  Xms                                           16ms
 |Game|-Wait----------------------------------------|Input|
 
 Rollback without partial wait:
 
 |Rollback---------|Game|-Wait----------------------|Input|
 
 = RANDOM +0.x input delay. Rollback time is inconsistent.
 
 ---
 
 Normal with partial wait:

 |Game|-Wait--------------|Input|-Wait--------------------|
 
 Rollback with partial wait:
  
 |Game|-Wait--------------|Input|Rollback---------|-Wait--|
 
 = CONSTANT +0.x input delay
 
 Therefore, partial wait system makes for a smoother experience.
 
 ---
 
 Ideal, but too much work Caster-side:
 
 |Game|-Wait--------|Rollback---------|-Wait--------|Input|
 
 */

// ************************************************* Helper funcs for writing

// all these calls to VirtualProtect are horribly inefficient,
// but it's only done once at startup, so who cares.

static void write_code(DWORD address, const char *data, int len) {
	DWORD old;
	
	VirtualProtect((void *)address, len, PAGE_EXECUTE_READWRITE, &old);
	
	memcpy((char *)address, data, len);

	VirtualProtect((void *)address, len, PAGE_EXECUTE_READ, &old);
}

static void write_byte(DWORD address, unsigned char value) {
	write_code(address, (const char *)&value, 1);
}

static void write_call(DWORD address, void *ptr) {
	DWORD value = (DWORD)ptr - (address+0x4);
	write_code(address, (const char *)&value, 4);
}

// ************************************************* Cache

#include <map>

class Cache {
private:
	typedef std::map<unsigned char *,bool>	m_cache_map_t;
	
	m_cache_map_t		m_cache_map;
	
	int			m_total;
	int			m_used;
	
	int			m_entry_size;
public:
	unsigned char *allocate() {
		if (m_used != m_total) {
			m_cache_map_t::iterator i = m_cache_map.begin();
			for (; i != m_cache_map.end(); ++i) {
				if (!i->second) {
					i->second = 1;
					
					++m_used;
					
					return i->first;
				}
			}
		}
		
		// create a new cache entry
		++m_used;
		++m_total;
		
		unsigned char *v = new unsigned char[m_entry_size];
		
		m_cache_map[v] = 1;
		
		return v;
	}
	
	void free(unsigned char *address) {
		m_cache_map_t::iterator i = m_cache_map.find(address);
		
		if (i != m_cache_map.end()) {
			if (i->second) {
				i->second = 0;
				--m_used;
			}
		}
	}
	
	void use(unsigned char *address) {
		m_cache_map_t::iterator i = m_cache_map.find(address);
		
		if (i != m_cache_map.end()) {
			if (!i->second) {
				i->second = 1;
				++m_used;
			}
		}
	}
	
	void free_all() {
		m_cache_map_t::iterator i = m_cache_map.begin();
		
		for (; i != m_cache_map.end(); ++i) {
			i->second = 0;
		}
		
		m_used = 0;
	}
	
	void clear() {
		m_cache_map_t::iterator i;
		
		while ((i = m_cache_map.begin()) != m_cache_map.end()) {
			delete[] i->first;
			
			m_cache_map.erase(i);
		}
	}

	Cache(int entry_size) {
		m_total = 0;
		m_used = 0;
		
		m_entry_size = entry_size;
	}
	
	~Cache() {
		clear();
	}
};

static Cache cache_ac(0xac);
static Cache cache_94(0x94);
static Cache cache_e8(0xe8);
static Cache cache_18(0x18);

static unsigned char *cache_allocate_ac() {
	return cache_ac.allocate();
}

static unsigned char *cache_allocate_94() {
	return cache_94.allocate();
}

static unsigned char *cache_allocate_e8() {
	return cache_e8.allocate();
}

static unsigned char *cache_allocate_18() {
	return cache_18.allocate();
}

static void cache_free_ac(unsigned char *address) {
	cache_ac.free(address);
}

static void cache_free_94(unsigned char *address) {
	cache_94.free(address);
}

static void cache_free_e8(unsigned char *address) {
	cache_e8.free(address);
}

static void cache_free_18(unsigned char *address) {
	cache_18.free(address);
}

unsigned char *cache_allocate(unsigned int size) {
	switch(size) {
	case 0xe8: return cache_e8.allocate(); break;
	case 0x94: return cache_94.allocate(); break;
	case 0xac: return cache_ac.allocate(); break;
	case 0x18: return cache_18.allocate(); break;
	}
	
	return 0;
}

void cache_use(unsigned int size, unsigned char *address) {
	switch(size) {
	case 0xe8: return cache_e8.use(address); break;
	case 0x94: return cache_94.use(address); break;
	case 0xac: return cache_ac.use(address); break;
	case 0x18: return cache_18.use(address); break;
	}
}

void cache_free(unsigned char *address, unsigned int size) {
	switch(size) {
	case 0xe8: cache_e8.free(address); break;
	case 0x94: cache_94.free(address); break;
	case 0xac: cache_ac.free(address); break;
	case 0x18: cache_18.free(address); break;
	}
}

void cache_flush() {
	cache_e8.free_all();
	cache_94.free_all();
	cache_ac.free_all();
	cache_18.free_all();
}

// ************************************************* Timer yeah!

static DWORD * const th075_speed = (DWORD *)0x66c23c;
static DWORD fps_counter = 0;

class Timer {
private:
	bool		first;
	DWORD		last_ticks;
	DWORD		tick_diff;
	
	void		wait_int(DWORD timeout);
public:
	void		wait();
	
	void		wait_partial(DWORD sub);
	
			Timer();
};

void Timer::wait_int(DWORD timeout) {
	DWORD ticks = timeGetTime();
	tick_diff += (ticks - last_ticks) * 1000;
	last_ticks = ticks;
	
	if (tick_diff < timeout) {
		HANDLE h = GetCurrentThread();
		
		int priority = GetThreadPriority(h);
		
		SetThreadPriority(h, THREAD_PRIORITY_TIME_CRITICAL);
		
		while (tick_diff < timeout) {
			Sleep(1);
			
			ticks = timeGetTime();
			int sleep_amount = (ticks - last_ticks) * 1000;
			
			tick_diff += sleep_amount;
			last_ticks = ticks;
		}
		
		SetThreadPriority(h, priority);
	}
}

void Timer::wait() {
	if (first) {
		last_ticks = timeGetTime();
		
		tick_diff = 0;
		
		first = 0;
	}
	
	DWORD wait;
	
	if (*th075_speed == 16) {
		//wait = 16393;
		wait = 16608;
	} else {
		wait = *th075_speed * 1042;
	}
	
	wait_int(wait);
	
	if (tick_diff >= wait) {
		tick_diff -= wait;
		
		if (wait >= 16000 && tick_diff > wait) {
			int n = tick_diff / wait;
			tick_diff -= n * wait;
		}
	}
}

void Timer::wait_partial(DWORD sub) {
	DWORD wait;
	
	if (*th075_speed == 16) {
		//wait = 16393;
		wait = 16608;
	} else {
		wait = *th075_speed * 1042;
	}
	
	if (sub > wait) {
		return;
	}
	
	//wait -= sub;
	wait = sub;
	
	wait_int(wait);
}

Timer::Timer() {
	first = 1;
}

static Timer timer;

// ************************************************* FPS nonsense

static bool first_fps = 1;

static void __stdcall fps_call(unsigned int *table) {
	// Custom FPS counter
	static DWORD last_fps_tick = 0;
	static DWORD fps_tick_diff = 0;
	if (first_fps) {
		first_fps = 0;
		fps_counter = 0;
		
		last_fps_tick = timeGetTime();
	}
	
	DWORD fps_tick = timeGetTime();
	fps_tick_diff += fps_tick - last_fps_tick;
	last_fps_tick = fps_tick;
	
	if (fps_tick_diff > 1000) {
		if (table) {
			DWORD count = ((fps_counter * 1000) + 1000) / fps_tick_diff;
			table[0x20/4] = count;
		}
		
		fps_counter = 0;
		fps_tick_diff = 0;
	}
}

// ************************************************* Replacement code

// for external access!
int rewind_frame_count = 0;
int caster_lock_mode = 0;

unsigned int *render_address = 0;

static struct state_t {
	State		state;
	
	struct sound_t {
		unsigned int	id;
		unsigned int	address;
		int		play_count;
	}		sound[20];
	int		sound_count;
} *state_table;
static int current_state = -1;

static bool ran_update = 0;

extern "C" {

typedef unsigned int __stdcall (*th075_sound_func_t)(unsigned int);
typedef unsigned int __stdcall (*th075_sound_get_max_t)();
typedef unsigned int * __stdcall (*th075_sound_has_sound_t)(unsigned int);
typedef unsigned int __stdcall (*th075_sound_get_sound_t)(unsigned int);
typedef unsigned int __stdcall (*th075_sound_stop_t)(unsigned int);

static const th075_sound_func_t th075_sound_func = (th075_sound_func_t)0x4079c0;
static const th075_sound_get_max_t th075_sound_get_max = (th075_sound_get_max_t)0x4093a0;
static const th075_sound_has_sound_t th075_sound_has_sound = (th075_sound_has_sound_t)0x409430;
static const th075_sound_get_sound_t th075_sound_get_sound = (th075_sound_get_sound_t)0x4093e0;

static bool sound_trap = 0;
static bool need_new_render = 0;
static DWORD rewind_time = 2;

static bool first_run = 0;

static const DWORD sound_address_table[] = {
	0x428f35,
	0x428f82,
	0x428ff9,
	0x42904d,
	0x42917b,
	0x429198,
	0x429243,
	0x42925d,
	0x4292a0,
	0x42a368,
	0x42a4f6,
	0x42ac54,
	0x42acaa,
	0x42ad00,
	0x42b3b7,
	0x42b4d0,
	0x42b526,
	0x42bd94,
	0x42bdff,
	0x42be6a,
	0x42bec0,
	0x42c6b3,
	0x42c6f1,
	0x42c746,
	0x42c783,
	0x42c7dc,
	0x42c832,
	0x42c8dc,
	0x42c9cc,
	0x42ca34,
	0x42ca4a,
	0x42ecde,
	0x42ed0e,
	0x42ed3e,
	0x42ed6e,
	0x42ed87,
	0x42eeae,
	0x42f9b9,
	0x42fa09,
	0x42fa5e,
	0x42fb80,
	0x42fca4,
	0x42fd0e,
	0x42fd7d,
	0x42feab,
	0x430015,
	0x43003a,
	0x430068,
	0x433648,
	0x4336c8,
	0x43377c,
	0x4337d2,
	0x433805,
	0x43385e,
	0x433a6c,
	0x433add,
	0x433b2b,
	0x433b69,
	0x433ba7,
	0x433c05,
	0x433c55,
	0x433ccb,
	0x433f19,
	0x4346db,
	0x43485f,
	0x4348a7,
	0x434adb,
	0x434b15,
	0x434b44,
	0x434b98,
	0x434bba,
	0x434ca8,
	0x434eac,
	0x434ee6,
	0x434f15,
	0x434fc6,
	0x434fe4,
	0x435096,
	0x435263,
	0x4357be,
	0x435804,
	0x4359f1,
	0x43a306,
	0x43a32e,
	0x43a350,
	0x43a36d,
	0x43a390,
	0x43a3af,
	0x43a3cc,
	0x43a3e2,
	0x43a3f8,
	0x43a455,
	0x43a499,
	0x43a4b6,
	0x43a4f0,
	0x44383f,
	0x443fb6,
	0x44400e,
	0x444392,
	0x44463e,
	0x44471b,
	0x44483b,
	0x444e53,
	0x445813,
	0x44621b,
	0x446341,
	0x446375,
	0x44842d,
	0x4485b6,
	0x4485cf,
	0x4485e8,
	0x448621,
	0x44865e,
	0x453d56,
	0x453d73,
	0x45b873,
	0x5fac53,
	0x0
};

static unsigned int __stdcall sound_passthrough(unsigned int sound_id) {
	unsigned int ecx_value;
	
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
	
	if (sound_id < 0) {
		return 0;
	}

	if (sound_trap && current_state >= 0) {
		state_t *state = &state_table[current_state];
		
		for (int i = 0; i < state->sound_count; ++i) {
			if (state->sound[i].id == sound_id &&
			    state->sound[i].address == ecx_value) {
			    	state->sound[i].play_count++;
	
				return 0;
			}
		}
		
		if (state->sound_count < 20) {
			state->sound[state->sound_count].id = sound_id;
			state->sound[state->sound_count].address = ecx_value;
			state->sound[state->sound_count].play_count = 1;

			++state->sound_count;
		}
	}
	
	asm volatile("movl %0, %%ecx" : : "r" (ecx_value) : "%ecx");

	return th075_sound_func(sound_id);
}

static void sound_clear_playcount() {
	state_t *state = &state_table[current_state];
	
	for (int i = 0; i < state->sound_count; ++i) {
		state->sound[i].play_count = 0;
	}
}

static void sound_stop(state_t::sound_t *sound) {
	asm volatile("movl %0, %%ecx" : : "r" (sound->address) : "%ecx");
	unsigned int max = th075_sound_get_max();
	
	if (sound->id < max) {
		asm volatile("movl %0, %%ecx" : : "r" (sound->address) : "%ecx");
		if (*th075_sound_has_sound(sound->id)) {
			asm volatile("movl %0, %%ecx" : : "r" (sound->address) : "%ecx");
			
			unsigned int address = th075_sound_get_sound(sound->id);
			
			unsigned int **table = *(unsigned int ***)address;
			
			th075_sound_stop_t func = (th075_sound_stop_t)(table[0][0x48/4]);
			
			func((unsigned int)table);
		}
	}
}

static void cancel_sounds(int frames) {
	int start_state = (current_state + 12 - frames) % 12;
	int state_no = start_state;
	
	// test all states in the slowest, most idiotic manner possible
	state_no = start_state;
	for (int i = 0; i < frames; ++i) {
		state_t *state = &state_table[state_no];
		
		for (int j = 0; j < state->sound_count; ++j) {
			state_t::sound_t *sound = &state->sound[j];
			
			if (sound->play_count > 0 || !sound->address) {
				continue;
			}
			
			// check backwards a couple frames, in case we rewound
			// oddly. this fixes sound halting on some attacks.
			// this will probably bug out on 10+ rewind. Oh well.
			bool no_cancel = 0;
			int sub_state_no = (state_no + 10)%12;
			int k = i - 2;
			for (; k < frames && !no_cancel; ++k) {
				state_t *sub_state = &state_table[sub_state_no];
				sub_state_no = (sub_state_no + 1) % 12;
				
				for (int l = 0; l < sub_state->sound_count; ++l) {
					state_t::sound_t *sub_sound = &sub_state->sound[l];
					
					if (sub_sound->id == sound->id
					    && sub_sound->address == sound->address
					    && sub_sound->play_count > 0) {
					    	no_cancel = 1;
					    	break;
					}
				}
			}
			
			if (!no_cancel) {
				// this sound never happened, and no
				// successive sound ever occured either.
				// so stop playing it.
				sound_stop(sound);
				
				// remove the entry entirely
				--state->sound_count;
				
				if (j != state->sound_count) {
					int count = state->sound_count - j;
					--j;
					
					while (count-- > 0) {
						sound[0] = sound[1];
						++sound;
					}
				}
				
				// old: cancel the sound
				//sound->id = -1;
				//sound->address = 0;
			}
		}
		
		state_no = (state_no + 1) % 12;
	}
}

// adding a useless parameter here forces gcc to not go into tail call mode.
// tail optimizations are bad because we use different calling conventions,
// and gcc decides to do the dumb thing and use up ecx in the process.
// This does not show up on asm output: It optimizes this later. This is why
// I could not figure it out at first, and had to find out what it was doing
// with ollydbg.
//
// bonus: using __fastcall here causes gcc to bail with a hilarious error.
typedef void (*th075_shadow_func_t)(unsigned int **dummy_value);
static const th075_shadow_func_t th075_shadow_func = (th075_shadow_func_t)0x453f70;

static void do_shadows() {
	// This causes very inconsistent results with the FPU, so the FPU state
	// is saved and restored around the calls.
	// FRSTOR must be called immediately after FSAVE,
	// because FSAVE implicitly calls FINIT, which is stupid and bad.
	
	unsigned char fpu_data[108];
	
	asm volatile ("fnsave %0" : "=m" (fpu_data[0]));
	asm volatile ("frstor %0" :: "m" (fpu_data[0]));
	
	unsigned int **p1struct = (unsigned int **)0x671418;
	asm volatile("movl %0, %%ecx" : : "r" (*p1struct) : "%ecx");
	th075_shadow_func(p1struct);
	
	unsigned int **p2struct = (unsigned int **)0x67141C;
	asm volatile("movl %0, %%ecx" : : "r" (*p2struct) : "%ecx");
	th075_shadow_func(p2struct);
	
	asm volatile ("frstor %0" :: "m" (fpu_data[0]));
}

// render speedup fix for combo counter
typedef void (*th075_combo_func_t)(unsigned int *dummy_value);
static const th075_combo_func_t th075_combo_func = (th075_combo_func_t)0x43d570;

static void update_combo_counter() {
	unsigned int *a;
	
	a = (unsigned int *)(render_address[0x50/4]);
	asm volatile("movl %0, %%ecx" : : "r" (a) : "%ecx");
	th075_combo_func(a);
	
	a = (unsigned int *)(render_address[0x54/4]);
	asm volatile("movl %0, %%ecx" : : "r" (a) : "%ecx");
	th075_combo_func(a);
}

// __fastcall because gcc puts the first param into ecx
// this is actually vc++ thiscall, but gcc has no analogue
typedef unsigned int __fastcall (*update_func_t)(unsigned int *);

static bool rewind_frames(int frames) {
	if (frames > 11) {
		return 0;
	}
	
	int prev = (current_state + 13 - frames) % 12;
	
	if (state_table[prev].state.valid()) {
		state_table[prev].state.restore();
		
		current_state = prev;
		
		return 1;
	}
	
	return 0;
}

static unsigned int __stdcall update_passthrough(unsigned int *update_struct) {
 	update_func_t func = ((update_func_t *)*update_struct)[1];
	
	sound_trap = 1;
	
	if (first_run) {
		timeBeginPeriod(1);
		
		first_run = 0;
	}
	
	if (update_struct[0] == 0x658334) {
		// this is the one we want
		unsigned int time = *(unsigned int *)0x6718b4;
		
		// value contains the menu pointer, don't run if we're in
		// a menu
		unsigned short value = update_struct[0xa0/4];
		
		if (!(value & 0xffff) && time > 180) {
			if (current_state == -1) {
				current_state = 0;
			}
			
			// switch around comments for testing
			//int rewind_count = 4;
			int rewind_count = rewind_frame_count;
			int rewinded = 0;
			
			// more testing code
			//if ((time%12) == 0) {
			//	rewind_count = 5;
			//}
			
			rewind_frame_count = 0;
			
			DWORD ticks = timeGetTime();
			
			if (rewind_count > 0 && rewind_frames(rewind_count)) {
				rewinded = rewind_count;
				
				// disable rendering
				unsigned char *render_flg = (unsigned char *)0x671430;
				unsigned char *render_flg2 = (unsigned char *)0x66c237;
				unsigned char orig_flg = *render_flg;
				unsigned char orig_flg2 = *render_flg2;
				
				*render_flg = 1;  // 30fps option OFF
				*render_flg2 = 0; // no render
				write_byte(0x43b3f0, 0);
				
				// disable hud except combo counter
				write_call(0x43de48, (void *)update_combo_counter);
				
				// run rewind
				while (--rewind_count > 0) {
					sound_clear_playcount();
					
					func(update_struct);
					
					do_shadows();
					
					current_state = (current_state+1)%12;
					state_table[current_state].state.store();
				}
				
				sound_clear_playcount();
				
				func(update_struct);
				
				do_shadows();
				
				// restore rendering
				write_call(0x43de48, (void *)0x43e040);
				
				write_byte(0x43b3f0, 1);
				*render_flg = orig_flg;
				*render_flg2 = orig_flg2;
			}
			
			// not paused. store current state for rewind
			current_state = (current_state + 1)%12;
			
			state_table[current_state].state.store();
			state_table[current_state].sound_count = 0;
			
			// process removed sounds.
			if (rewinded > 0) {
				cancel_sounds(rewinded);
			
				// calculate time taken for use with timing
				ticks = timeGetTime() - ticks;
				
				int n = ticks + 1;
				if (n < 2) {
					n = 2;
				}
				if (n > 12) {
					n = 12;
				}
				rewind_time = n;
			}
		}
	} else {
		// no longer in game, flush our state cache
		if (current_state != -1) {
			for (int i = 0; i < 12; ++i) {
				state_table[i].state.flush();
				state_table[i].sound_count = 0;
			}
			
			current_state = -1;
		}
		
		render_address = 0;
		
		need_new_render = 1;
		
		first_fps = 1;
	}
	
	timer.wait_partial(rewind_time*1042);
	
	++fps_counter;
	
	ran_update = 1;
	
	unsigned int retval = func(update_struct);
		
	sound_trap = 0;
	
	return retval;
}

static void *render_alloc(unsigned int size) {
	void *retval = game_alloc(size);
	
	render_address = (unsigned int *)retval;
	
	return retval;
}

typedef void (*render_pos_func_t)(unsigned int a, unsigned int b, unsigned int c);
static const render_pos_func_t render_pos_func = (render_pos_func_t)0x401b00;

static float last_camera_x = 0.0;
static float last_camera_y = 0.0;
static float last_dx = 0.0;
static float last_dy = 0.0;

static unsigned char render_fpu_data[108];
static bool render_fpu_saved = false;

static void render_pos_passthrough(unsigned int _a, unsigned int _b, unsigned int _c) {
	// store render fpu data for later usage.
	asm volatile ("fnsave %0" : "=m" (render_fpu_data[0]));
	asm volatile ("frstor %0" :: "m" (render_fpu_data[0]));
	render_fpu_saved = true;
	
	unsigned char *render_flg2 = (unsigned char *)0x66c237;
	float *sx = (float *)0x671404;
	float *sy = (float *)0x6713c4;
	//float *a = (float *)&_a;
	float *b = (float *)&_b;
	float *c = (float *)&_c;
	
	if (*render_flg2 == 1) {
		float x = *sx;
		float y = *sy;
		float dx = x - last_camera_x;
		float dy = y - last_camera_y;
		
		if (need_new_render) {
			dx = 0;
			dy = 0;
			need_new_render = 0;
		} else {
			float ddx = dx - last_dx;
			
			if (fabs(ddx) > 1) {
				float diff = fabs(ddx) - 1;
				if (ddx < 0) {
					diff = -diff;
				}
				
				float ndx = dx - diff;
				
				// not allowed to change direction sign
				if (!(diff < 0 && ndx > 0)
				    && !(ndx < 0 && diff > 0)) {
					*b -= diff;
					x -= diff;
					dx -= diff;
				}
			}
			
			float ddy = dy - last_dy;
			
			if (fabs(ddy) > 1) {
				float diff = fabs(ddy) - 1;
				if (ddy < 0) {
					diff = -diff;
				}
				
				float ndy = dy - diff;
				
				// not allowed to change direction sign
				if (!(diff < 0 && ndy > 0)
				    && !(ndy < 0 && diff > 0)) {
					*c -= diff;
					y -= diff;
					dy -= diff;
				}
			}
		}
		
		last_camera_x = x;
		last_camera_y = y;
		
		last_dx = dx;
		last_dy = dy;
	} else {
		return;
	}

	render_pos_func(_a, _b, _c);
}

typedef void __fastcall (*render_dostuff_func_t)(unsigned int a);
static const render_dostuff_func_t render_dostuff_func = (render_dostuff_func_t)0x4406c0;

static void __fastcall render_fpu_restore(unsigned int a) {
	render_dostuff_func(a);
	
	// restore fpu state from before the camera.
	// this is effectively equivalent to having frameskip on.
	if (render_fpu_saved) {
		asm volatile ("frstor %0" :: "m" (render_fpu_data[0]));
		render_fpu_saved = false;
	}
}

// new timing loop
static void __stdcall timing_call() {
	if (caster_lock_mode) {
		//Sleep(1);
		//return;
	}
	
	timer.wait();
	
	ran_update = 0;
}

typedef DWORD (*orig_random_t)();
static const orig_random_t orig_random = (orig_random_t)0x6418AB;

static DWORD stage_random_value = 0;

static DWORD __stdcall stage_random_hook(unsigned char *blah) {
	int stage = blah[0xf1];
	DWORD value;
	
	while (1) {
		value = orig_random()%10;
		
		if ((stage + value) % 10 != 0) {
			break;
		}
	}
	
	stage_random_value = (stage + value) % 10;
	
	return value + 0x1E;
}

static DWORD stage_time_random_hook() {
	if (stage_random_value == 6 || stage_random_value == 3) {
		return 0x00000001;
	}
	
	return orig_random();
}

};

// ************************************************* DLL startup

extern "C" {

BOOL APIENTRY DllMain(HANDLE hMod, DWORD reason, LPVOID nothing) {
	switch(reason) {
		case DLL_PROCESS_DETACH: {
			break;
		}
		case DLL_PROCESS_ATTACH: {
			// initialize some variables
			first_run = 1;
			
			state_table = new state_t[12];
			
			// kill original timing regulation thread
			write_byte(0x4239fe, 0xeb);
			
			// timing code
			write_byte(0x603539, 0xe8); // call . ..
			write_call(0x60353a, (void *)timing_call);
			
			for (DWORD i = 0x60353a+4; i < 0x603545; ++i) {
				write_byte(i, 0x90);
			}
			
			// render allocation call
			write_call(0x6030ab, (void *)render_alloc);
			
			// render positioning call
			write_call(0x43db3e, (void *)render_pos_passthrough);
			
			// post-render fpu restore
			write_call(0x43dde2, (void *)render_fpu_restore);
			
			// sound passthrough hooks
			for (int i = 0; sound_address_table[i]; ++i) {
				write_call(sound_address_table[i], (void *)sound_passthrough);
			}
			
			// update passthrough
			unsigned char *test = (unsigned char *)0x602fa6;
			if (*test != 0xff) {
				// if we're NOT in caster, 0x90 this.
				write_byte(0x602fa6, 0x90);
			}
			write_byte(0x602fa7, 0x90);
			write_byte(0x602fa8, 0x90);
			write_byte(0x602fa9, 0x90);
			write_byte(0x602faa, 0x90);
			write_byte(0x602fab, 0x50); // push eax
			write_byte(0x602fac, 0xe8); // call ...
			write_call(0x602fad, (void *)update_passthrough);
			
			// send addresses to caster.
			*(int **)0x68fffc = &rewind_frame_count;
			*(int **)0x68fff8 = &caster_lock_mode;
			
			// stage random hook
			static const char random_hook[16] = {
				0x8b, 0x45, 0xf4,		// mov eax, dword ptr ss:[ebp-c]
				0x50,				// push eax
				0xe8, 0x00, 0x00, 0x00, 0x00,	// call stage_random_hook
				0x8b, 0xd0,			// mov edx, eax
				0x90, 0x90, 0x90, 0x90, 0x90	// nop
			};
			write_code(0x435819, random_hook, 16);
			
			write_call(0x43581e, (void *)stage_random_hook);
			
			write_call(0x4354d4, (void *)stage_time_random_hook);
			
			// new fps hook
			write_byte(0x43dfde, 0x90);
			write_byte(0x43dfdf, 0x90);
			
			write_byte(0x43dfe6, 0x50); // push eax
			write_byte(0x43dfe7, 0xe8);
			write_call(0x43dfe8, (void *)fps_call);
			for (DWORD i = 0x43dfe8 + 4; i < 0x43e002; ++i) {
				write_byte(i, 0x90);
			}
			
			// shadow bug
			write_byte(0x454637, 0x90);
			write_byte(0x454638, 0x90);

			// cache addresses.
			
			// 0xe8
			write_call(0x46d184, (void *)cache_allocate_e8);
			write_call(0x4930d4, (void *)cache_allocate_e8);
			write_call(0x4c0944, (void *)cache_allocate_e8);
			write_call(0x4e2db4, (void *)cache_allocate_e8);
			write_call(0x5022e4, (void *)cache_allocate_e8);
			write_call(0x52d0b4, (void *)cache_allocate_e8);
			write_call(0x551ac4, (void *)cache_allocate_e8);
			write_call(0x574234, (void *)cache_allocate_e8);
			write_call(0x5974a4, (void *)cache_allocate_e8);
			write_call(0x5c0bd4, (void *)cache_allocate_e8);
			write_call(0x5e2194, (void *)cache_allocate_e8);
			
			write_call(0x45be8a, (void *)cache_free_e8);
			write_call(0x45bae9, (void *)cache_free_e8);
			
			// 0x94
			write_call(0x5f7b1b, (void *)cache_allocate_94);
			write_call(0x5f792b, (void *)cache_allocate_94);
			
			write_call(0x5f7e4a, (void *)cache_free_94);
			write_call(0x5f71ca, (void *)cache_free_94);
			
			// 0xac
			write_call(0x4125ae, (void *)cache_allocate_ac);
			
			write_call(0x41218c, (void *)cache_free_ac);
			
			// 0x18
			write_call(0x5324ab, (void *)cache_allocate_18);
			
			write_call(0x5320dc, (void *)cache_free_18);
			
			break;
		}
	}

	return TRUE;
}

};
