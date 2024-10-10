#include "ReadSHM.h"
#include <iostream>
#include <cstring>

ReadSHM::ReadSHM(SharedMemoryManager& shm_mgr)
    : shm_mgr_(shm_mgr) {}

void ReadSHM::rframe(char* output_buffer, size_t data_size) {
    shm_mgr_.P();  // 等待生产者提供帧数据

    char* shm_ptr = static_cast<char*>(shm_mgr_.attach());
    std::memcpy(output_buffer, shm_ptr, data_size);
    shm_mgr_.detach(shm_ptr);

    shm_mgr_.V();  // 通知生产者继续写入
}

