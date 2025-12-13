#pragma once
#include <Controller/Controller.h>
#include <string>

namespace ar {
	// adb命令控制器类
	class AdbController : public Controller {
	private:
		std::string adb_path = "";
		int adb_port = 0;
		bool is_initialize = false;
	public:
		// 构造函数只是单纯的存储对应的参数
		AdbController(std::string adb_path, int adb_port);
		~AdbController();
		bool initialize() override;
		bool connect() override;
		bool disconnect() override;
		bool click(const int x, const int y) override;
		bool longClick(const int x, const int y, const int ms_duration) override;
		bool inputKey(const int key_code) override;
		bool swipe(const int x_1, const int y_1, const int x_2, const int y_2, const int ms_duration) override;
		bool screencap(cv::Mat& image_) override;
	};

}