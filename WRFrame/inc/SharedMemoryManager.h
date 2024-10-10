// SharedMemoryManager.h
#ifndef SHAREDMEMORYMANAGER_H
#define SHAREDMEMORYMANAGER_H

#include <sys/ipc.h>
#include <cstddef>

class SharedMemoryManager {
public:
    SharedMemoryManager(key_t shm_key, key_t sem_key, size_t shm_size);
    ~SharedMemoryManager();

    void* attach();
    void detach(void* shm_ptr);

    void P();
    void V();

    void initSemaphore(int value);

    size_t getSize() const;

private:
    key_t shm_key_;
    key_t sem_key_;
    int shmid_;
    int semid_;
    size_t shm_size_;
};

#endif // SHAREDMEMORYMANAGER_H

