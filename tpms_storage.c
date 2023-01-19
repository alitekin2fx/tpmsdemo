#include "tpmsdemo.h"
#include "tpms_storage.h"

int tpms_storage_init(struct tpms_storage *storage, int capacity) {
    storage->records = (struct tpms_record*)malloc(sizeof(struct tpms_storage) * capacity);
    if (storage->records == NULL)
        return(0);

    storage->capacity = capacity;
    storage->write_seq = -1;
    storage->read_seq = 0;
    return(1);
}

void tpms_storage_cleanup(struct tpms_storage *storage) {
    free(storage->records);
}

int tpms_storage_push_back(struct tpms_storage *storage, const struct tpms_record *record) {
    if (tpms_storage_is_full(storage))
        return(0);

    int next_write_seq = storage->write_seq + 1;
    storage->records[next_write_seq % storage->capacity] = *record;
    storage->write_seq++;
    return(1);
}

int tpms_storage_pop(struct tpms_storage *storage, struct tpms_record *record) {
    if (tpms_storage_is_empty(storage))
        return(0);

    *record = storage->records[storage->read_seq % storage->capacity];
    storage->read_seq++;
    return (1);
}
