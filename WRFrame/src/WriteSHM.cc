#include "WriteSHM.h"
#include <iostream>
#include <cstring>

WriteSHM::WriteSHM(SharedMemoryManager& shm_mgr)
    : shm_mgr_(shm_mgr) {}

void WriteSHM::wframe(const char* frame_data, size_t data_size) {
    if (data_size > shm_mgr_.getSize()) {
        std::cerr << "数据大小超过共享内存限制" << std::endl;
        return;
    }

    char* shm_ptr = static_cast<char*>(shm_mgr_.attach());
    std::memcpy(shm_ptr, frame_data, data_size);
    shm_mgr_.V();  // 通知消费者

    shm_mgr_.P();  // 等待消费者处理完
    shm_mgr_.detach(shm_ptr);
}

