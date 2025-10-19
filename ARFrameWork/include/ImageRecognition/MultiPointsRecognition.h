#pragma once
#include <opencv2/opencv.hpp>
#include <ImageRecognition/ImageParams.hpp>
#include <ImageRecognition/ImageRecognition.h>

namespace ar{

class MultiPointsRecognition : public ImageRecognition{
public:
    //return a centre point
    point compareImageReturnCentrePoint(cv::Mat& image, cv::Mat& temp, const float& threshold = 0.95f) override;

    point compareImageReturnCentrePoint(const std::string& image_path, const std::string& temp_path, const float& threshold = 0.95f) override;

    const unsigned int num_points = 32; // todo 看起来是设备的采样点数
};

}