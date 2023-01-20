#include "tpmsdemo.h"
#include "tpms_storage.h"

// This code is not thread safe, we need to use sychronization mothods to make it thread safe.
int tpms_storage_init(struct tpms_storage *storage, int capacity) {
	storage->records = (struct tpms_record*)malloc(sizeof(struct tpms_storage) * capacity);
	if (storage->records == NULL)
		return(0);

	storage->capacity = capacity;
	storage->head = 0;
	storage->tail = 0;
	storage->full = 0;
	return(1);
}

void tpms_storage_cleanup(struct tpms_storage *storage) {
	free(storage->records);
}

int tpms_storage_push_back(struct tpms_storage *storage, const struct tpms_record *record) {
	if (tpms_storage_is_full(storage))
			return(0);

	storage->records[storage->head] = *record;
	storage->head = (storage->head + 1) % storage->capacity;
	if (storage->tail == storage->head)
		storage->full = 1;

	return(1);
}

int tpms_storage_pop(struct tpms_storage *storage, struct tpms_record *record) {
	if (tpms_storage_is_empty(storage))
		return(0);

	if (record != NULL)
		*record = storage->records[storage->tail];

	storage->tail = (storage->tail + 1) % storage->capacity;
	storage->full = 0;
	return (1);
}
