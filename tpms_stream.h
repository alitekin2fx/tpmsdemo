#ifndef __TPMS_STREAM_H__
#define __TPMS_STREAM_H__

struct tpms_stream {
	FILE *file;
};

int tpms_stream_open(struct tpms_stream *stream, const char *file);
void tpms_stream_close(struct tpms_stream *stream);
int tpms_stream_read_byte(struct tpms_stream *stream);

#endif //__TPMS_STREAM_H__
