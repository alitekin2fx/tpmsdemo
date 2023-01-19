#include "tpmsdemo.h"
#include "tpms_log_reader.h"

int tpms_log_reader_open(struct tpms_log_reader *log_reader, const char *file) {
    log_reader->file = fopen(file, "rb");
	return(log_reader->file != NULL);
}

void tpms_log_reader_close(struct tpms_log_reader *log_reader) {
    fclose(log_reader->file);
}

int tpms_log_reader_getc(struct tpms_log_reader *log_reader) {
    return(fgetc(log_reader->file));
}
