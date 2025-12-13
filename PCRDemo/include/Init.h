#pragma once
#include <string>
#include <Controller/Controller.h>
#include <DeviceController/DeviceController.h>
#include <ImageRecognition/ImageRecognition.h>

namespace pcr{

    // 全局参数
struct global_params{
public:
	const std::string device_path = ""; //模拟器路径
	const int index; //模拟器序号 todo
	std::string adb_path; // adb路径
	int adb_port; // adb端口
	ar::controllerType controller_type; // 控制器类型，这个应该是控制器安卓设备的操作类型
	ar::imageRecognitionType image_recognition_type; // 图像识别类型 todo
    ar::deviceControllerType device_controller_type; // 安卓设备的类型，目前只有MuMu模拟器
    
    bool start_with_silence = false; // todo
    int operate_duration = 500; //ms 每次操作的延迟等待时间，估计是没什么好的判断标识，只能选择等待
    int run_max_times = 200; // 最大的重试次数，如果一个操作重试的次数超过了这个，则判断为有问题，自动退出

    // 以下几个控制器对象的作用是啥？
    std::unique_ptr<ar::Controller> controller;
    std::unique_ptr<ar::DeviceController> device_controller;
    // 这个应该就是图像识别的对象
    std::unique_ptr<ar::ImageRecognition> image_recognition;

    global_params(const std::string device_path,
                  const int index,
                  std::string adp_path,
                  int adb_port,
                  ar::controllerType controller_type,
                  ar::imageRecognitionType image_recognition_type,
                  ar::deviceControllerType device_controller_type);
};

void checkImageCompareRate(ar::imageRecognitionType image_recognition_type);

struct main_line_normal_params {
public:
    bool is_auto_execute_in_multi_drop = true;
    int scheme_num = -1;
};

struct main_line_veryhard_params {
public:
    int scheme_num = -1;
};

struct store_params {
public:
    bool is_buy_lotion = true;
    bool is_buy_refining_stone = true;
};

}