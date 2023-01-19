#include "tpmsdemo.h"
#include "tpms_tracker.h"
#include "tpms_storage.h"
#include "tpms_reporter.h"
#include "tpms_log_reader.h"

int main(int argc, char **argv) {
    uint32_t uart_ticks;
    struct tpms_tracker tracker;
    struct tpms_storage storage;
    struct tpms_log_reader log_reader;

    if (!tpms_log_reader_open(&log_reader, "./TPMS.txt"))
        sys_handle_error();

    if (!tpms_storage_init(&storage, 32))
        sys_handle_error();    

    uart_ticks = sys_get_ticks();
    tpms_tracker_init(&tracker, &storage);
    while(1) {
// Simulate the data is coming from a serial port in 10ms
        if (sys_get_ticks() - uart_ticks > 10) {
            int data = tpms_log_reader_getc(&log_reader);
            if (data > 0) {
                uart_ticks = sys_get_ticks();
                tpms_tracker_process(&tracker, data);
            }
        }


        tpms_reporter_report(&storage);

// Let CPU sleep, we can wait for UART or reporter interrupt here
        usleep(100);
    }

    tpms_log_reader_close(&log_reader);
    tpms_storage_cleanup(&storage);
    return(0);
}

void sys_handle_error() {
    exit(-1);
}

uint32_t sys_get_ticks() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (1000 * ts.tv_sec + ts.tv_nsec / 1000000);
}
