#ifndef __TPMS_STORAGE_H__
#define __TPMS_STORAGE_H__

struct tpms_record
{
	int8_t battery_state;       // battery state (1/0)
	uint16_t battery_voltage;   // battery voltage (mV)
	uint8_t sensor_id[4];       // sensor ID
};

struct tpms_storage
{
	struct tpms_record *records;
	volatile int write_seq;
	volatile int read_seq;
	int capacity;
};

int tpms_storage_init(struct tpms_storage *storage, int capacity);
void tpms_storage_cleanup(struct tpms_storage *storage);

int tpms_storage_push_back(struct tpms_storage *storage, const struct tpms_record *record);
int tpms_storage_pop(struct tpms_storage *storage, struct tpms_record *record);

static inline int tpms_storage_is_empty(const struct tpms_storage *storage) {
	return (storage->write_seq < storage->read_seq);
}

static inline int tpms_storage_is_full(const struct tpms_storage *storage) {
	int size = (storage->write_seq - storage->read_seq) + 1;
	return (size >= storage->capacity);
}

#endif //__TPMS_STORAGE_H__
