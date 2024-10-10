#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>

class FrameQueue {
private:
    std::queue<cv::Mat> queue;
    std::mutex mtx;
    std::condition_variable cv;
    size_t max_size;

public:
    FrameQueue(size_t max_size);

    void push(const cv::Mat& frame);
    cv::Mat pop();
    bool is_empty();
};

#endif