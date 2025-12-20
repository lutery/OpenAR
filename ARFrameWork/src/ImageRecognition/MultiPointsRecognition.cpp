#include <ImageRecognition/MultiPointsRecognition.h>
#include <ImageRecognition/TemplateMatchMPR.h>
#include <Log/MiniLog.h>

/**
 * @brief
 * @param image 原图
 * @param temp 模板图片
 * @param threshold 图片对比是通过灰度图的方式，需要设置一个灰度图
 */
ar::point ar::MultiPointsRecognition::compareImageReturnCentrePoint(cv::Mat& image, cv::Mat& temp, const float& threshold) {
	ar::point point;
	if (num_points < 8) ar::warn("num_points < 8 are not recommand !");
	if (temp.empty() || image.empty()) {
		ar::error("Image empty !");
		return point;
	}
	// 看来这里要求模板图不能大于原图
	if (temp.rows * temp.cols > image.rows * image.cols) {
		ar::error("temp size : {} > image size : {} !", temp.rows * temp.cols, image.rows * image.cols);
		return point;
	}

	// 将所有图像转为灰度图
	// 看来这是是以灰度图来进行对比的 todo 是否可以直接彩色图
	if (temp.channels() != 1) cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
	if (image.channels() != 1) cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
	int res[2] = { -1, -1 }; // 存储temp图片在image图片中最匹配时的左上角坐标
	std::string res_msg = "";
	bool err = false;
	// 调用模板匹配的多点对比算法，找到temp图片在image图片中的最匹配的左上角坐标
	err = ar::templateMatchMPR(res,
		res_msg,
		image.data,
		temp.data,
		image.cols,
		image.rows,
		temp.cols,
		temp.rows,
		threshold,
		num_points);
	if (!err) {
		ar::error("{}", res_msg);
		return point;
	}

	if (res[0] != -1 && res[1] != -1)
	// 存储temp图片最匹配时中心点的坐标在image图片中的位置
		point = {(res[0] + temp.cols / 2), (res[1] + temp.rows / 2), 0, false};
	return point;
}

/**
 * 与另一个方法不同的地方在于传入的是图片的路径
 */
ar::point ar::MultiPointsRecognition::compareImageReturnCentrePoint(const std::string& image_path, const std::string& temp_path, const float& threshold) {
	ar::point point;
	cv::Mat image = cv::imread(image_path);
	cv::Mat temp = cv::imread(temp_path);
	if (num_points < 8) ar::warn("num_points < 8 are not recommand !");
	if (num_points < 8) ar::warn("num_points < 8 are not recommand !");
	if (temp.empty() || image.empty()) {
		ar::error("Image empty !");
		return point;
	}
	if (temp.rows * temp.cols > image.rows * image.cols) {
		ar::error("temp size : {} > image size : {} !", temp.rows * temp.cols, image.rows * image.cols);
		return point;
	}
	if (temp.channels() != 1) cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
	if (image.channels() != 1) cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
	int res[2] = { -1, -1 };
	std::string res_msg = "";
	bool err = false;
	err = ar::templateMatchMPR(res,
		res_msg,
		image.data,
		temp.data,
		image.cols,
		image.rows,
		temp.cols,
		temp.rows,
		threshold,
		num_points);
	if (!err) {
		ar::error("{}", res_msg);
		return point;
	}

	if (res[0] != -1 && res[1] != -1)
		point = { (res[0] + temp.cols / 2), (res[1] + temp.rows / 2), 0, false };
	return point;
}