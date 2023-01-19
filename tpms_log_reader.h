#ifndef __TPMS_LOG_READER_H__
#define __TPMS_LOG_READER_H__

struct tpms_log_reader {
    FILE *file;
};

int tpms_log_reader_open(struct tpms_log_reader *log_reader, const char *file);
void tpms_log_reader_close(struct tpms_log_reader *log_reader);
int tpms_log_reader_getc(struct tpms_log_reader *log_reader);

#endif //__TPMS_LOG_READER_H__
