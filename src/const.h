#ifndef CASTER_CONST
#define CASTER_CONST

//uMsg string
#define umsg_string "th075_result"

//main
#define main_default	0
#define main_file		1
#define main_arg		2
#define main_wait		3
#define main_end		0xF


//buf_size
#define inputBuf_size	512000
#define stask_buf_size	700
#define recv_buf_size	5120
#define z_buf_size	2048

//address
#define body_int3_address 0x602FA0
#define char_int3_address 0x445831
#define rand_int3_address 0x45558F
#define rand_int3_address2 0x4555A8
#define rand_address 0x66C0C6
#define replay_int3_address 0x413921
#define palette_int3_address 0x40BC87
#define render_scale_int3_address 0x401B01
#define fader_scale_int3_address1 0x432361
#define fader_scale_int3_address2 0x432411
#define fader_scale_int3_address3 0x432407
#define fader_scale_int3_address4 0x4324CC
#define high_res_fix_int3_address 0x431f61
#define sprite_filter_int3_address_start 0x43dca8
#define sprite_filter_int3_address_end 0x43dd5e
#define sprite_filter_int3_address_end2 0x43dde6
#define dllhook_int3_address 0x64232c

//シングルステップモードフラグ
#define EFLAGS_TF 0x00000100

//de
#define de_body	1
#define de_char	2

//timeout[ms]
#define timeout_time	10000

//auto_menu
#define auto_menu	1

//task
#define task_main		0
#define task_recv		1
#define task_manage	2

#define stask_data	1
#define stask_area	2

//debug_mode
#define debug_mode_main		0
#define debug_mode_mainRoop	0
#define debug_mode_thread	0
#define debug_mode_close 	0
#define debug_mode_func 	0

//status
#define status_default	0
#define status_ok		1
#define status_error	2
#define status_bad	3

//mode
#define mode_root		1
#define mode_branch	2
#define mode_subbranch	3
#define mode_broadcast	4
#define mode_access	5
#define mode_wait		6
#define mode_wait_target 7
#define mode_debug	9
#define mode_default	0

//phase
#define phase_none	0
#define phase_default	1
#define phase_menu	2
#define phase_read	3
#define phase_battle	4

//dest
#define dest_here 1
#define dest_away 2
#define dest_root 3
#define dest_branch 4
#define dest_subbranch 5
#define dest_leaf 6
#define dest_addr 7
#define dest_access 8

//header
#define cmd_version 1
// cmd_casters describes the game that this Caster uses.
// currently known values: 0 = IaMP, 1 = SWR, 2 = MB
#define cmd_casters 0
#define cmd_space_2 0
#define cmd_space_3 0
#define cmd_version_error 0xFF

//command
#define cmd_echo		0
#define cmd_state		1
#define cmd_sendinput	2
#define cmd_time		3
#define cmd_gameInfo	4
#define cmd_dataInfo	5
#define cmd_input_req	6
#define cmd_reject	8
#define cmd_exit		9

#define res_echo		10
#define res_state		11
#define res_time		13
#define res_gameInfo	14
#define res_dataInfo	15
#define res_input_req	16

#define res_inputdata_area 17
#define cmd_inputdata_req 18
#define res_inputdata_req 19

#define cmd_access	20
#define res_access	23
#define cmd_delay		24
#define res_delay		25

#define cmd_continue	26
#define res_continue	27

#define cmd_rand		30
#define res_rand		31
#define cmd_playerside	32
#define res_playerside	33
#define cmd_delayobs	35
#define res_delayobs	36
#define cmd_session	37
#define res_session	38
#define cmd_initflg	40
#define res_initflg	41

#define cmd_join		45
#define res_join		46
#define cmd_cast		50

#define cmd_addr_branch 60
#define cmd_addr_leaf	61

#define cmd_standby	70
#define cmd_ready		71

#define res_inputdata_z	75

#define cmd_testport	80
#define res_testport	81

#define cmd_echoes	85
#define res_echoes	86

#define cmd_seek_leaf	87
#define res_seek_leaf	88

#define cmd_req_palette 0x90
#define res_req_palette 0x91

// cowcaster identifier - fifth byte is protocol version, update when changed
#define cowcaster_protocol_version 0x05
static const char cowcaster_id[5] = { 'C', 'C', 0xa0, 0xa1, cowcaster_protocol_version };
static const char cowcaster_version_string[] = "120221";

// keybind identifiers
enum {
	KEY_AUTOSAVE_ON,
	KEY_AUTOSAVE_OFF,
	KEY_AUTOSAVE_TOGGLE,
	KEY_AUTONEXT_ON,
	KEY_AUTONEXT_OFF,
	KEY_AUTONEXT_TOGGLE,
	KEY_ROUNDCOUNT_CYCLE,
	KEY_BGM_TOGGLE,
	KEY_NOFAST_TOGGLE,
	KEY_SAVE_PALETTE,
	KEY_INFINITE_SPIRIT_CHEAT,
	KEY_REMOTE_PALETTE_CYCLE,
	KEY_STAGE_FREE,
	KEY_STAGE_MANUAL,
	KEY_ALWAYSONTOP_TOGGLE,
	KEY_DELAY1,
	KEY_DELAY2,
	KEY_DELAY3,
	KEY_DELAY4,
	KEY_DELAY5,
	KEY_DELAY6,
	KEY_DELAY7,
	KEY_DELAY8,
	KEY_DELAY9,
	KEY_DELAY10,
	KEY_REWIND_MODIFIER,
	KEY_COUNT
};

#endif
