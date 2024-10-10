#include "SharedMemoryManager.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

SharedMemoryManager::SharedMemoryManager(key_t shm_key, key_t sem_key, size_t shm_size)
    : shm_key_(shm_key), sem_key_(sem_key), shm_size_(shm_size) {
    // 创建共享内存
    shmid_ = shmget(shm_key_, shm_size_, IPC_CREAT | 0666);
    if (shmid_ == -1) {
        perror("shmget failed");
        exit(1);
    }

    // 创建信号量
    semid_ = semget(sem_key_, 1, IPC_CREAT | 0666);
    if (semid_ == -1) {
        perror("semget failed");
        exit(1);
    }
}

SharedMemoryManager::~SharedMemoryManager() {
    // 分离并销毁共享内存和信号量
    shmctl(shmid_, IPC_RMID, nullptr);
    semctl(semid_, 0, IPC_RMID);
}

void* SharedMemoryManager::attach() {
    void* shm_ptr = shmat(shmid_, nullptr, 0);
    if (shm_ptr == reinterpret_cast<void*>(-1)) {
        perror("shmat failed");
        exit(1);
    }
    return shm_ptr;
}

void SharedMemoryManager::detach(void* shm_ptr) {
    shmdt(shm_ptr);
}

void SharedMemoryManager::P() {
    struct sembuf sb = {0, -1, 0};
    semop(semid_, &sb, 1);
}

void SharedMemoryManager::V() {
    struct sembuf sb = {0, 1, 0};
    semop(semid_, &sb, 1);
}

void SharedMemoryManager::initSemaphore(int value) {
    semctl(semid_, 0, SETVAL, value);
}

size_t SharedMemoryManager::getSize() const {
    return shm_size_;
}

