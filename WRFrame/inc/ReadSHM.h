#ifndef RSHM_H
#define RSHM_H

#include "SharedMemoryManager.h"

class ReadSHM {
public:
    ReadSHM(SharedMemoryManager& shm_mgr);
    void rframe(char* output_buffer, size_t data_size);

private:
    SharedMemoryManager& shm_mgr_;
};

#endif // RSHM_H

