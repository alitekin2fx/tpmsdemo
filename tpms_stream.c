#include "tpmsdemo.h"
#include "tpms_stream.h"

int tpms_stream_open(struct tpms_stream *stream, const char *file) {
	stream->file = fopen(file, "rb");
	return(stream->file != NULL);
}

void tpms_stream_close(struct tpms_stream *stream) {
	fclose(stream->file);
}

int tpms_stream_read_byte(struct tpms_stream *stream) {
	return(fgetc(stream->file));
}
