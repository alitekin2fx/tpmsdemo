#ifndef __TPMS_TRACKER_H__
#define __TPMS_TRACKER_H__
#include "tpms_storage.h"

#define TMPS_MAX_LINE_LENGTH	32

// Packet timeout is 500ms
#define TMPS_PACKET_TIMEOUT		500

struct tpms_packet
{
	int8_t battery_state;		// battery state (1/0)
	uint16_t battery_voltage;	// battery voltage (mV)
	uint8_t sensor_id[4];		// sensor ID
	int8_t rssi;				// RSSI
};

struct tpms_tracker
{
	int line_len;
	int skip_line;
	int last_char;
	uint32_t last_char_ticks;
	struct tpms_storage *storage;
	char line_buffer[TMPS_MAX_LINE_LENGTH + 1];
};

void tpms_tracker_init(struct tpms_tracker *tracker, struct tpms_storage *storage);
void tpms_tracker_process(struct tpms_tracker *tracker, int data);

#endif //__TPMS_TRACKER_H__
