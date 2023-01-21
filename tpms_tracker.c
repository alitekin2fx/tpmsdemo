#include "tpmsdemo.h"
#include "tpms_tracker.h"

#define CHECK_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))

static int tpms_tracker_handle_line(struct tpms_tracker *tracker);
static int tpms_tracker_handle_packet(struct tpms_tracker *tracker, const struct tpms_packet *packet);
static int tpms_tracker_parse_packet(struct tpms_tracker *tracker, struct tpms_packet *packet);
static int tpms_tracker_parse_number(struct tpms_tracker *tracker, int buf_pos, int base, 
	int allow_negative, int min_digits, int max_digits, int *result);
static struct tpms_record* tpms_convert_packet_to_record(const struct tpms_packet *packet, 
	struct tpms_record *record);

void tpms_tracker_init(struct tpms_tracker *tracker, struct tpms_storage *storage) {
	tracker->line_len = 0;
	tracker->skip_line = 0;
	tracker->last_char = '\0';
	tracker->storage = storage;
	tracker->last_char_ticks = sys_get_ticks();
}

void tpms_tracker_process(struct tpms_tracker *tracker, int data) {
	if (sys_get_ticks() - tracker->last_char_ticks > TMPS_PACKET_TIMEOUT) {
// Timeout occurred, we will start over
		tracker->skip_line = 0;
		tracker->line_len = 0;
	}
 
	if (data == '\r' || data == '\n') {
		if (!tracker->skip_line && tracker->line_len > 0) {
			tracker->line_buffer[tracker->line_len] = '\0';
			tpms_tracker_handle_line(tracker);
		}
		tracker->skip_line = 0;
		tracker->line_len = 0;
	} else if (!tracker->skip_line) {
		if (!isprint(data)) {
// Invalid character received, so we will skip this line
			tracker->skip_line = 1;
// Also we ignore multiple spaces not to fulfill the buffer
		} else if (!isspace(data) || !isspace(tracker->last_char)) {
			if (tracker->line_len < TMPS_MAX_LINE_LENGTH)
				tracker->line_buffer[tracker->line_len++] = (char)data;
			else
				tracker->skip_line = 1;
		}
	}

	tracker->last_char_ticks = sys_get_ticks();
	tracker->last_char = data;
}

int tpms_tracker_handle_line(struct tpms_tracker *tracker) {
	struct tpms_packet packet;

	if (!tpms_tracker_parse_packet(tracker, &packet))
		return(0);

	return(tpms_tracker_handle_packet(tracker, &packet));
}

int tpms_tracker_handle_packet(struct tpms_tracker *tracker, const struct tpms_packet *packet) {
	struct tpms_record record;
	
// Remove the oldest record if there is no space left
	if (tpms_storage_is_full(tracker->storage))
		tpms_storage_pop(tracker->storage, NULL);

	return(tpms_storage_push_back(tracker->storage, tpms_convert_packet_to_record(packet, &record)));
}

int tpms_tracker_parse_packet(struct tpms_tracker *tracker, struct tpms_packet *packet) {
	int value, buf_pos = 0;

// Parse battery state
	if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 16, 0, 2, 2, &value)) < 0)
		return(0);
	packet->battery_state = value;

// Parse battery voltage
	packet->battery_voltage = 0;
	for(int i = 0; i < 2; i++) {
		if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 16, 0, 2, 2, &value)) < 0)
			return(0);
		packet->battery_voltage <<= 8;
		packet->battery_voltage |= (uint16_t)(value & 0xff);
	}

// Parse sensor id
	for(int i = 0; i < sizeof(packet->sensor_id); i++) {
		if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 16, 0, 2, 2, &value)) < 0)
			return(0);
		packet->sensor_id[i] = (int8_t)value;
	}

// Parse rssi
	if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 10, 1, 1, 3, &value)) < 0)
		return(0);
	packet->rssi = value;

// Check line ending
	while(isspace(tracker->line_buffer[buf_pos]))
		buf_pos++;
	if (tracker->line_buffer[buf_pos] != '\0')
		return(0);

// Check values
	return(CHECK_RANGE(packet->battery_state, 0, 1)/* && CHECK_RANGE(packet->battery_voltage, 0, 5000)*/);
}

int tpms_tracker_parse_number(struct tpms_tracker *tracker, int buf_pos, int base, 
	int allow_negative, int min_digits, int max_digits, int *result) {
// Skip leading spaces
	while(isspace(tracker->line_buffer[buf_pos]))
		buf_pos++;

	int negative = 0;
	if (tracker->line_buffer[buf_pos] == '-') {
		if (!allow_negative)
			return(-1);
		negative = 1;
		buf_pos++;
	}

	int chr, value = 0, digit_count = 0;
	while((chr = tracker->line_buffer[buf_pos++]) != '\0') {
		if (isspace(chr))
			break;
		else if (isdigit(chr))
			chr = chr - '0';
		else if (isalpha(chr))
			if (isupper(chr))
				chr = 10 + (chr - 'A');
			else
				chr = 10 + (chr - 'a');
		else
			return(-1);

		value *= base;
		value += chr;
		digit_count++;
	}

	if (digit_count < min_digits || digit_count > max_digits)
		return(-1);

	if (negative)
		value = -value;

	*result = (uint8_t)value;
	return(buf_pos);
}

struct tpms_record* tpms_convert_packet_to_record(const struct tpms_packet *packet, 
	struct tpms_record *record) {
	record->battery_state = packet->battery_state;
	record->battery_voltage = packet->battery_voltage;
	for(int i = 0; i < sizeof(record->sensor_id); i++)
		record->sensor_id[i] = packet->sensor_id[i];

	return(record);
}
