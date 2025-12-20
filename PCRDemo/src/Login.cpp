#include <Login.h>
#include <define.hpp>

void pcr::login(pcr::global_params& params)
{
    // 使用宏定义，通过opencv mat imread加载图片
    // 这些图片的主要作用是提前获取一些游戏特定界面区域按钮的图片，然后通过
    // 图像对比算法，得到当前的模拟器是否处于特定的界面上
    // 感觉限制很大，因为游戏界面可能会经常更新做活动
    LOAD_IMAGE(skip, "res/pcr/skip.png"); // 跳过按钮
    LOAD_IMAGE(confirm_blue, "res/pcr/confirm_blue.png"); // 确认界面
    LOAD_IMAGE(close_white, "res/pcr/close_white.png"); // 关闭按钮
    LOAD_IMAGE(main_line, "res/pcr/main_line.png"); // 主线按钮 
    LOAD_IMAGE(download, "res/pcr/download.png"); // 下载按钮

    bool stop_condition = false;
    int times = 0; // 记录尝试启动的次数，用于超过指定次数时报错

    ar::info("Start launch pcr !");
    stop_condition = false; times = 0; 
    do {
        Sleep(params.operate_duration); times++; if (times > params.run_max_times) {
            ar::error("Runtime too long !"); std::exit(0);
        } 
        cv::Mat frame; 
        params.controller->screencap(frame); 

        // 通过尝试截图判断图片是否加载成功
        if (frame.empty()) {
            ar::error("Frame empty !");
            params.controller->disconnect();
            std::exit(0);
        }

        // 尝试点击指定区域
        params.controller->click(804, 890); 
        ar::point p_skip = params.image_recognition->compareImageReturnCentrePoint(frame, skip, 0.95f);  // 尝试判断是否是需要点击跳过的界面，并识别指定按钮的位置
        if (!p_skip.is_empty) params.controller->click(p_skip.x, p_skip.y);  // 点击指定的区域
        ar::point p_confirm_blue = params.image_recognition->compareImageReturnCentrePoint(frame, confirm_blue, 0.95f);  // 尝试从界面上找到确认的按钮
        if (!p_confirm_blue.is_empty) params.controller->click(p_confirm_blue.x, p_confirm_blue.y); 
        ar::point p_close_white = params.image_recognition->compareImageReturnCentrePoint(frame, close_white, 0.95f); // 尝试i从界面上找到关闭按钮
        if (!p_close_white.is_empty) params.controller->click(p_close_white.x, p_close_white.y); 
        ar::point p_download = params.image_recognition->compareImageReturnCentrePoint(frame, download, 0.95f); // 尝试从界面上找到下载按钮
        if (!p_download.is_empty) params.controller->click(p_download.x, p_download.y);  
        ar::point p_main_line = params.image_recognition->compareImageReturnCentrePoint(frame, main_line, 0.95f); // 尝试从界面上找到特定的区域
        if (!p_main_line.is_empty) stop_condition = true; //如果找到了，则说明进入了游戏主线的界面，完成登录
    } while (!stop_condition);
}