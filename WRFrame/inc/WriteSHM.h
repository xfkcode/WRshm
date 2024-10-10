#ifndef WSHM_H
#define WSHM_H

#include "SharedMemoryManager.h"

class WriteSHM {
public:
    WriteSHM(SharedMemoryManager& shm_mgr);
    void wframe(const char* frame_data, size_t data_size);

private:
    SharedMemoryManager& shm_mgr_;
};

#endif // WSHM_H

