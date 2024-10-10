#include "FrameQueue.h"

FrameQueue::FrameQueue(size_t max_size) : max_size(max_size) {}

void FrameQueue::push(const cv::Mat& frame) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this](){ return queue.size() < max_size; });
    if (queue.size() == max_size - 1) {
        std::cout << "队列已满，等待消费..." << std::endl;
    }
    queue.push(frame);
    // std::cout << "推入一帧，当前队列大小: " << queue.size() << std::endl;
    lock.unlock();
    cv.notify_all();
}

cv::Mat FrameQueue::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this](){ return !queue.empty(); });
    if (queue.empty()) {
        std::cout << "队列为空，等待生产..." << std::endl;
    }
    cv::Mat frame = queue.front();
    queue.pop();
    // std::cout << "弹出一帧，当前队列大小: " << queue.size() << std::endl;
    lock.unlock();
    cv.notify_all();
    return frame;
}

bool FrameQueue::is_empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
}


