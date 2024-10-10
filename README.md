# 接口说明

**工程目录**

WRFrame/
|-- CMakeLists.txt
|-- dance.mp4
|-- inc
|   |-- FrameQueue.h
|   |-- ReadSHM.h
|   |-- SharedMemoryManager.h
|   \`-- WriteSHM.h
\`-- src
    |-- FrameQueue.cc
    |-- R_client.cc
    |-- ReadSHM.cc
    |-- SharedMemoryManager.cc
    |-- W_server.cc
    |-- WriteSHM.cc
    `-- main.cc

## 1. `SharedMemoryManager`

### 概述

`SharedMemoryManager` 是一个管理共享内存和信号量操作的类，用于进程间通信（IPC）。它提供了创建、附加、分离和管理共享内存段及信号量的方法。

### 构造函数

```c++
SharedMemoryManager(key_t shm_key, key_t sem_key, size_t shm_size);
```

- 参数：
  - `shm_key`：共享内存段的唯一键。
  - `sem_key`：信号量的唯一键。
  - `shm_size`：共享内存段的大小（以字节为单位）。

### 析构函数

```
~SharedMemoryManager();
```

- **描述**：当 `SharedMemoryManager` 实例被销毁时，清理并移除共享内存和信号量。

### 方法

- **`void* attach()`**
  - **返回**：指向附加的共享内存段的指针。
  - **描述**：将共享内存段附加到调用进程的地址空间。
- **`void detach(void\* shm_ptr)`**
  - **参数**：
    - `shm_ptr`：要分离的共享内存段的指针。
  - **描述**：将共享内存段从调用进程的地址空间中分离。
- **`void P()`**
  - **描述**：对信号量执行等待操作（P 操作），直到信号量可用为止。
- **`void V()`**
  - **描述**：对信号量执行信号操作（V 操作），释放给其他进程使用。
- **`void initSemaphore(int value)`**
  - **参数**：
    - `value`：信号量的初始值。
  - **描述**：使用指定的值初始化信号量。
- **`size_t getSize() const`**
  - **返回**：共享内存段的大小。
  - **描述**：提供由实例管理的共享内存段的大小。

------

## 2. `WriteSHM`

### 概述

`WriteSHM` 是一个提供将帧数据写入共享内存段接口的类。它利用 `SharedMemoryManager` 进行管理和同步。

### 构造函数

```C++
WriteSHM(SharedMemoryManager& shm_mgr);
```

- **参数**：
  - `shm_mgr`：一个对 `SharedMemoryManager` 实例的引用，用于内存和信号量管理。

### 方法

- `void wframe(const char* frame_data, size_t data_size)`
  - **参数**：
    - `frame_data`：指向要写入的数据帧的指针。
    - `data_size`：数据帧的大小（以字节为单位）。
  - **描述**：将提供的帧数据写入共享内存。它检查数据大小是否超过分配的共享内存大小，并处理信号量操作以在生产者和消费者之间进行同步。

------

## 3. `ReadSHM`

### 概述

`ReadSHM` 是一个提供从共享内存段读取帧数据接口的类。它同样利用 `SharedMemoryManager` 进行管理和同步。

### 构造函数

```C++
ReadSHM(SharedMemoryManager& shm_mgr);
```

- **参数**：
  - `shm_mgr`：一个对 `SharedMemoryManager` 实例的引用，用于内存和信号量管理。

### 方法

- `void rframe(char* output_buffer, size_t data_size)`
  - **参数**：
    - `output_buffer`：指向将存储读取帧数据的缓冲区的指针。
    - `data_size`：要从共享内存读取的数据大小（以字节为单位）。
  - **描述**：从共享内存中读取帧数据到提供的输出缓冲区。它处理信号量操作，以确保与生产者的同步。

------

## 使用示例

该示例演示了如何使用共享内存和信号量在两个进程之间传输视频帧数据。
它使用 OpenCV 库从视频文件中读取帧，并将其写入共享内存，另一个进程读取共享内存并显示视频帧。

```C++
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
```

**编译**：

```bash
$ mkdir build && cd  build
$ cmake ..
$ make
```

**运行**：

- 读取模式：

  ```bash
  $ ./main R
  ```

- 写入模式：

  ```bash
  $./main W <VideoPath>
  ```

## 模拟AI处理示例

程序 A 从视频源逐帧读取 RGB 数据并将其发送到程序 B，程序 B 在接收到每帧数据后添加水印并将处理后的帧返回给程序 A 进行显示。

```c++
//A
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
```

```c++
//B
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
```

- 读取模式：

  ```bash
  $ ./build/R_client
  ```

- 写入模式：

  ```bash
  $ ./build/W_server ./dance.mp4
  ```