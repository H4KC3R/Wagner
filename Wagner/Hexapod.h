	#pragma once

#define CONTROL_BLOCK	30000
#define TO_HEXAPOD		30002
#define FROM_HEXAPOD	40003

enum UDP_CONFIG_OP_CODES {
	ENABLE_REMOTE_POWER_CONTROL = 20,
	POWER_ON,
	POWER_OFF,
	ENABLE_LOGICAL_COMMUNICATION_CHANNEL = 10
};


#pragma pack(push, 1)
typedef struct udp_config {
	int mode;
	int op_code;
}udp_config_t;
#pragma pack(pop)


enum UDP_TO_HEXAPOD_OP_CODES{
	FORCE_SET_SYNC_STAT = 20,
	RESET_STAT,
	STOP_MOVING = 30,
	INIT = 40,
	TO_ZERO,
	SET_USER_CS = 50,
	MOVE_TO_POSITION = 60,
	SET_SPIN_PARAMS = 70,
	SPIN_WITH_PARAMS = 72,
	SET_OSCILLATION_PARAMS = 80,
	PREPARE_FOR_OSCILLATION,
	START_OSCILLATION,
	SET_TRACE_PARAMS = 90,
	PREPARE_FOR_TRACE,
	START_TRACE
};

#pragma pack(push, 1)
typedef struct hex_pos_config {
	int op_code;
	int cs;
	int mode;

	union {
		float parameter_float;
		int parameter_int;
	};

	union {
		struct {
			float XYZ[3], UVW[3], XYZ1[3], UVW1[3];
		};

		struct {
			float srg_Axyz[3], srg_Fxyz[3], srg_UVW[3], srg_dUVW[3];
		};

		struct {
			float uro_target_dUVW[3], uro_XYZ[3], uro_UVW[3], uro_dUVW[3];
		};

		struct {
			float uro_target_dUVW[3], uro_XYZ[3], uro_UVW[3], uro_dUVW[3];
		};

		struct {
			float shlim_XYZ0[3], shlim_XYZ1[3], shlim_UVW0[3], shlim_UVW1[3];
		};

		struct {
			float shlimVmax_XYZ[3], shlim_Amax_XYZ[3], shlim_OMmax_UVW[3], shlim_EPSmax_UVW[3];
		};
	} vectors;
}hex_pos_config_t;
#pragma pack(pop)

typedef struct alarm_bits {

}alarm_bits_t;

#pragma pack(push, 1)
typedef struct udp_stat {
	unsigned char init_done;
	unsigned char ptg_ready;
	unsigned char hex_pos_cmd;
	unsigned char power;

	float cur_XYZ[3];
	float cur_UVW[3];

	unsigned int cur_heat_percent;
	float cur_heat_celsius;
} udp_stat_t;
#pragma pack(pop)
