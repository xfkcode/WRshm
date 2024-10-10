#include "SharedMemoryManager.h"
#include "WriteSHM.h"
#include "ReadSHM.h"
#include <opencv2/opencv.hpp>
#include <cstring>
#include <thread>

#define WIDTH 1280
#define HEIGHT 720
#define SHM_SIZE (1280 * 720 * 3)
#define SHM_KEY_1 0x5005
#define SEM_KEY_1 0x5005
#define SHM_KEY_2 0x5006
#define SEM_KEY_2 0x5006

void addWatermark(cv::Mat& image, const std::string& watermarkText) {
    // 设置字体、大小、颜色等
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.5;
    int thickness = 2;
    cv::Scalar color = cv::Scalar(255, 255, 255);  // 白色
    int baseline = 0;

    // 获取文本的尺寸
    cv::Size textSize = cv::getTextSize(watermarkText, fontFace, fontScale, thickness, &baseline);

    // 定义水印的位置（右下角）
    cv::Point textOrg(image.cols - textSize.width - 10, image.rows - textSize.height - 10);

    // 在图像上添加文本水印
    cv::putText(image, watermarkText, textOrg, fontFace, fontScale, color, thickness);
}

void OP_frames(SharedMemoryManager& shm_manager_r, SharedMemoryManager& shm_manager_w) {
    ReadSHM rshm(shm_manager_r);
    WriteSHM wshm(shm_manager_w);
    cv::Mat frame(HEIGHT, WIDTH, CV_8UC3);
    while (true) {
        rshm.rframe(reinterpret_cast<char*>(frame.data), shm_manager_r.getSize());
        //模拟AI推理（加水印）
        addWatermark(frame, "JRLC");
        wshm.wframe(reinterpret_cast<char*>(frame.data), shm_manager_w.getSize());
    }
}


int main(int argc, char* argv[]) {
    SharedMemoryManager shm_mgr_receive(SHM_KEY_1, SEM_KEY_1, SHM_SIZE);
    shm_mgr_receive.initSemaphore(0);

    SharedMemoryManager shm_mgr_send(SHM_KEY_2, SEM_KEY_2, SHM_SIZE);
    shm_mgr_send.initSemaphore(1);

    OP_frames(shm_mgr_receive, shm_mgr_send);
    
    return 0;
}

