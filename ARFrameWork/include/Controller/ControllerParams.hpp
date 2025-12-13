#pragma once
#include <string>

namespace ar {

	enum class controllerType {
		ADB,
		MUMU,
	};

	// 控制器参数
	struct controllerParams {
		//adb
		std::string adb_path = "";
		int adb_port = -1;
		//mumu
		const wchar_t* mumu_path = L"";
		int mumu_index = -1; // 这个应该是mumu模拟器的设备索引
	};

}