#include "tpmsdemo.h"
#include "tpms_stream.h"
#include "tpms_tracker.h"
#include "tpms_storage.h"
#include "tpms_reporter.h"

#define TPMS_LOG_FILE			"TPMS.txt"
#define TPMS_UART_SPEED			10
#define TPMS_REPORT_SPEED		5
#define TPMS_STORAGE_CAPACITY	2

int main(int argc, char **argv) {
	struct tpms_stream stream;
	struct tpms_tracker tracker;
	struct tpms_storage storage;
	uint32_t uart_ticks, reporter_ticks;

	if (!tpms_stream_open(&stream, TPMS_LOG_FILE))
		sys_handle_error();

	if (!tpms_storage_init(&storage, TPMS_STORAGE_CAPACITY))
		sys_handle_error();    

	tpms_tracker_init(&tracker, &storage);
	uart_ticks = reporter_ticks = sys_get_ticks();
	while(1) {
// Simulate the data is coming from a serial port
		if (sys_get_ticks() - uart_ticks > TPMS_UART_SPEED) {
			int data = tpms_stream_read_byte(&stream);
			if (data > 0)
				tpms_tracker_process(&tracker, data);

			uart_ticks = sys_get_ticks();
		}

// Report queued tpms records
		if (sys_get_ticks() - reporter_ticks > TPMS_REPORT_SPEED) {
			tpms_reporter_report(&storage);
			reporter_ticks = sys_get_ticks();
		}

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
