#include "SharedMemoryManager.h"
#include "WriteSHM.h"
#include "ReadSHM.h"
#include <opencv2/opencv.hpp>
#include <cstring>

#define WIDTH 1280
#define HEIGHT 720
#define SHM_SIZE (1280 * 720 * 3)
#define SHM_KEY 0x5005
#define SEM_KEY 0x5005

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "./main <W|R> [VideoPath]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    SharedMemoryManager shm_mgr(SHM_KEY, SEM_KEY, SHM_SIZE);

    if (mode == "W") {
        if (argc < 3) {
            std::cerr << "./main W VideoPath" << std::endl;
            return 1;
        }
        std::string video_source = argv[2];
        shm_mgr.initSemaphore(1);
        WriteSHM wshm(shm_mgr);

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
                continue;
            }

            if (frame.cols != WIDTH || frame.rows != HEIGHT) {
                cv::resize(frame, frame, cv::Size(WIDTH, HEIGHT));
            }

            wshm.wframe(reinterpret_cast<char*>(frame.data), SHM_SIZE);
        }

        cap.release();
    } else if (mode == "R") {
        shm_mgr.initSemaphore(0);
        ReadSHM rshm(shm_mgr);

        cv::Mat frame(HEIGHT, WIDTH, CV_8UC3);
        while (true) {
            rshm.rframe(reinterpret_cast<char*>(frame.data), SHM_SIZE);

            cv::imshow("Frame", frame);
            if (cv::waitKey(30) == 'q') {
                break;
            }
        }
    } else {
        std::cerr << "Error: invalid mode\n" << mode << "W or R" << std::endl;
        return 1;
    }

    return 0;
}

