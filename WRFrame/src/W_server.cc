#include "SharedMemoryManager.h"
#include "WriteSHM.h"
#include "ReadSHM.h"
#include <opencv2/opencv.hpp>
#include <cstring>
#include <thread>
#include <sys/stat.h>

#define WIDTH 1280
#define HEIGHT 720
#define SHM_SIZE (1280 * 720 * 3)
#define SHM_KEY_1 0x5005
#define SEM_KEY_1 0x5005
#define SHM_KEY_2 0x5006
#define SEM_KEY_2 0x5006

// 接收显示
void receive_frames(SharedMemoryManager& shm_manager) {
    ReadSHM rshm(shm_manager);
    cv::Mat frame(HEIGHT, WIDTH, CV_8UC3);
    
    while (true) {
        rshm.rframe(reinterpret_cast<char*>(frame.data), shm_manager.getSize());
        cv::imshow("Frame", frame);

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "./W_server VideoPath" << std::endl;
        return 1;
    }

    SharedMemoryManager shm_mgr_send(SHM_KEY_1, SEM_KEY_1, SHM_SIZE);
    shm_mgr_send.initSemaphore(1);

    SharedMemoryManager shm_mgr_receive(SHM_KEY_2, SEM_KEY_2, SHM_SIZE);
    shm_mgr_receive.initSemaphore(0);


    std::string video_source = argv[1];
    shm_mgr_send.initSemaphore(1);
    WriteSHM wshm(shm_mgr_send);

    std::thread receiver_thread(receive_frames, std::ref(shm_mgr_receive));

    cv::VideoCapture cap(video_source);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open the video" << std::endl;
        return 1;
    }
    
    cv::Mat frame;
    while (true) {
        if (!cap.read(frame)) {
            std::cout << "End of video" << std::endl;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;;
        }

        if (frame.cols != WIDTH || frame.rows != HEIGHT) {
            cv::resize(frame, frame, cv::Size(WIDTH, HEIGHT));
        }

        wshm.wframe(reinterpret_cast<char*>(frame.data), shm_mgr_send.getSize());
    }

    cap.release();
    receiver_thread.join();

    return 0;
}

