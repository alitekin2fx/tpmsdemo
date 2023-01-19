#include "tpmsdemo.h"
#include "tpms_stream.h"
#include "tpms_tracker.h"
#include "tpms_storage.h"
#include "tpms_reporter.h"

#define TPMS_LOG_FILE   "TPMS.txt"

int main(int argc, char **argv) {
	uint32_t uart_ticks;
	struct tpms_stream stream;
	struct tpms_tracker tracker;
	struct tpms_storage storage;

	if (!tpms_stream_open(&stream, TPMS_LOG_FILE))
		sys_handle_error();

	if (!tpms_storage_init(&storage, 32))
		sys_handle_error();    

	uart_ticks = sys_get_ticks();
	tpms_tracker_init(&tracker, &storage);
	while(1) {
// Simulate the data is coming from a serial port in 10ms
		if (sys_get_ticks() - uart_ticks > 10) {
			int data = tpms_stream_read_byte(&stream);
			if (data > 0) {
				uart_ticks = sys_get_ticks();
				tpms_tracker_process(&tracker, data);
			}
		}

// Report queued tpms records
		tpms_reporter_report(&storage);

// Let CPU sleep, we can wait for UART or reporter interrupt here
		usleep(100);
	}

	tpms_stream_close(&stream);
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
