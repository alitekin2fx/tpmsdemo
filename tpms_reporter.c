#include "tpmsdemo.h"
#include "tpms_reporter.h"

static void tpms_reporter_print(const struct tpms_record *record);

void tpms_reporter_report(struct tpms_storage *storage) {
    while(!tpms_storage_is_empty(storage)) {
        struct tpms_record record;
        if (tpms_storage_pop(storage, &record))
            tpms_reporter_print(&record);
    }
}

void tpms_reporter_print(const struct tpms_record *record) {
    printf("sensor_id = %02x%02x%02x%02x, battery_state = %d, battery_voltage = %d mV\r\n", 
        record->sensor_id[0], record->sensor_id[1], record->sensor_id[2], record->sensor_id[3], 
        record->battery_state, record->battery_voltage);
}
