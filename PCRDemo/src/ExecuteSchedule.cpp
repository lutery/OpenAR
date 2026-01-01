#include <ExecuteSchedule.h>
#include <Log/MiniLog.h>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <define.hpp>

/**
 * 以下方法没啥好说的，就是执行日常任务的流程
 * 就是根据游戏界面上的按钮来进行点击
 */
void pcr::execute_schedule(pcr::global_params& params){
    // 这里将要对比的模板图加载进去
    cv::Mat close_white = cv::imread("./res/pcr/close_white.png");
    cv::Mat schedule = cv::imread("./res/pcr/schedule.png");
    cv::Mat auto_schedule = cv::imread("./res/pcr/auto_schedule.png");
    cv::Mat confirm_blue = cv::imread("./res/pcr/confirm_blue.png");
    cv::Mat confirm_white = cv::imread("./res/pcr/confirm_white.png");
    cv::Mat auto_schedule_gray = cv::imread("./res/pcr/auto_schedule_gray.png");
    cv::Mat kokoro_in_schedule = cv::imread("./res/pcr/kokoro_in_schedule.png");
    cv::Mat home_off = cv::imread("./res/pcr/home_off.png");
    LOAD_IMAGE(skip_over, "./res/pcr/skip_over.png"); // 看来这张图片是必须加载的，采用宏定义，如果加载失败则退出
    
    bool stop_condition = false;
    int times = 0; // 重试的次数

    ar::info("Back to home");
    // 以下流程是尝试匹配日常工作按钮，如果超过最大重试次数则退出
    do{
        Sleep(params.operate_duration);
        times++;
        if(times > params.run_max_times){ar::error("Runtime too long !"); std::exit(0);}

        cv::Mat frame;
        params.controller->screencap(frame);

        params.controller->click(155, 885);
        ar::point p_schedule = params.image_recognition->compareImageReturnCentrePoint(frame, schedule, 0.95f); // 模板匹配到日常按钮，
        if(!p_schedule.is_empty) stop_condition = true;
    }while(!stop_condition);

    stop_condition = false;
    times = 0;
    ar::info("Enter schedule");
    // 以下流程是尝试进入日常界面，如果超过最大重试次数则退出
    do{
        Sleep(params.operate_duration);
        times++;
        if(times > params.run_max_times){ar::error("Runtime too long !"); std::exit(0);}

        cv::Mat frame;
        params.controller->screencap(frame);

        ar::point p_close_white = params.image_recognition->compareImageReturnCentrePoint(frame, close_white, 0.95f);
        if(!p_close_white.is_empty) params.controller->click(p_close_white.x, p_close_white.y);
        ar::point p_schedule = params.image_recognition->compareImageReturnCentrePoint(frame, schedule, 0.95f);
        if(!p_schedule.is_empty) params.controller->click(p_schedule.x, p_schedule.y);
        ar::point p_kokoro_in_schedule = params.image_recognition->compareImageReturnCentrePoint(frame, kokoro_in_schedule, 0.95f);
        if(!p_kokoro_in_schedule.is_empty) stop_condition = true;
        
    }while(!stop_condition);

    stop_condition = false;
    times = 0;
    //execute schedule
    ar::info("Execute schedule");
    do{
        Sleep(params.operate_duration);
        times++;
        if(times > params.run_max_times){ar::error("Runtime too long !"); std::exit(0);}

        cv::Mat frame;
        params.controller->screencap(frame);

        ar::point p_auto_schedule = params.image_recognition->compareImageReturnCentrePoint(frame, auto_schedule, 0.95f);
        if(!p_auto_schedule.is_empty) params.controller->click(p_auto_schedule.x, p_auto_schedule.y);
        ar::point p_confirm_blue = params.image_recognition->compareImageReturnCentrePoint(frame, confirm_blue, 0.95f);
        if(!p_confirm_blue.is_empty) params.controller->click(p_confirm_blue.x, p_confirm_blue.y);
        ar::point p_confirm_white = params.image_recognition->compareImageReturnCentrePoint(frame, confirm_white, 0.95f);
        if(!p_confirm_white.is_empty) params.controller->click(p_confirm_white.x, p_confirm_white.y);
        ar::point p_close_white = params.image_recognition->compareImageReturnCentrePoint(frame, close_white, 0.95f);
        if(!p_close_white.is_empty) params.controller->click(p_close_white.x, p_close_white.y);
        ar::point p_skip_over = params.image_recognition->compareImageReturnCentrePoint(frame, skip_over, 0.95f);
        if (!p_skip_over.is_empty) params.controller->click(p_skip_over.x, p_skip_over.y);
        ar::point p_auto_schedule_gray = params.image_recognition->compareImageReturnCentrePoint(frame, auto_schedule_gray, 0.95f);
        if(!p_auto_schedule_gray.is_empty) stop_condition = true;
    }while(!stop_condition);

    stop_condition = false;
    times = 0;
    ar::info("Back to home");
    params.controller->click(1578, 500);
}