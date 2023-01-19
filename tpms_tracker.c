#include "tpmsdemo.h"
#include "tpms_tracker.h"

static int tpms_tracker_handle_line(struct tpms_tracker *tracker);
static int tpms_tracker_handle_packet(struct tpms_tracker *tracker, const struct tpms_packet *packet);
static int tpms_tracker_parse_packet(struct tpms_tracker *tracker, struct tpms_packet *packet);
static int tpms_tracker_parse_number(struct tpms_tracker *tracker, int buf_pos, int allow_negative, 
    int hexedecimal, int min_digits, int max_digits, int *result);

void tpms_tracker_init(struct tpms_tracker *tracker, struct tpms_storage *storage) {
    tracker->line_len = 0;
    tracker->last_char = '\0';
    tracker->storage = storage;
    tracker->last_char_ticks = sys_get_ticks();
}

void tpms_tracker_process(struct tpms_tracker *tracker, const uint8_t data) {
// Check packet timeout and reset the states if it happened
    if (sys_get_ticks() - tracker->last_char_ticks > TMPS_PACKET_TIMEOUT) {
        tracker->line_len = 0;
    }
 
    if (data == '\r' || data == '\n') {
        if (tracker->line_len > 0) {
            tracker->line_buffer[tracker->line_len] = '\0';
            tpms_tracker_handle_line(tracker);
        }
        tracker->line_len = 0;
    } else if (!isprint(data)) {
// Invalid character received
        tracker->line_len = 0;        
    } else {
        if (!isspace(data) || !isspace(tracker->last_char)) {
// Ignore multiple spaces            
            if (tracker->line_len < TMPS_MAX_LINE_LENGTH) {
                tracker->line_buffer[tracker->line_len++] = data;
            }
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

    record.battery_state = packet->battery_state;
    record.battery_voltage = packet->battery_voltage;
    for(int i = 0; i < sizeof(record.sensor_id); i++)
        record.sensor_id[i] = packet->sensor_id[i];

    return(tpms_storage_push_back(tracker->storage, &record));
}

int tpms_tracker_parse_packet(struct tpms_tracker *tracker, struct tpms_packet *packet) {    
    int value, buf_pos = 0;

// Parse battery state
    if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 0, 1, 2, 2, &value)) < 0 || 
        value < 0 || value > 1)
        return(0);
    packet->battery_state = value;

// Parse battery voltage
    packet->battery_voltage = 0;
    for(int i = 0; i < 2; i++) {
        if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 0, 1, 2, 2, &value)) < 0)
            return(0);
        packet->battery_voltage <<= 8;
        packet->battery_voltage |= (uint16_t)(value & 0xff);
    }

// Parse sensor id
    for(int i = 0; i < sizeof(packet->sensor_id); i++) {
        if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 0, 1, 2, 2, &value)) < 0)
            return(0);
        packet->sensor_id[i] = (int8_t)value;
    }

// Parse rssi
    if ((buf_pos = tpms_tracker_parse_number(tracker, buf_pos, 1, 0, 1, 3, &value)) < 0)
        return(0);
    packet->rssi = value;

// Check ending
    while(isspace(tracker->line_buffer[buf_pos]))
        buf_pos++;

    return(tracker->line_buffer[buf_pos] == '\0');
}

static int tpms_tracker_parse_number(struct tpms_tracker *tracker, int buf_pos, int allow_negative, 
    int hexedecimal, int min_digits, int max_digits, int *result) {        
// Skip whitespaces
    while(isspace(tracker->line_buffer[buf_pos]))
        buf_pos++;

    char chr;
    int value = 0, negative = 0, digit_count = 0;
    while((chr = tracker->line_buffer[buf_pos]) != '\0') {
        if (isspace(chr))
            break;

        if (chr == '-') {
            if (!allow_negative)
                return(-1);

            negative = 1;
        }
        else {
            if (hexedecimal) {
                value *= 16;
                if (isdigit(chr))
                    value += chr - '0';
                else if (chr >= 'a' && chr <= 'f')
                    value += 10 + (chr - 'a');
                else if (chr >= 'A' && chr <= 'F')
                    value += 10 + (chr - 'A');
                else
                    return(-1);
            } else {
                value *= 10;
                if (isdigit(chr))
                    value += chr - '0';
            }            
            digit_count++;
        }
        buf_pos++;
    }

    if (negative)
        value *= -1;

    if (digit_count < min_digits || digit_count > max_digits)
        return(-1);

    *result = (uint8_t)value;
    return(buf_pos);
}
