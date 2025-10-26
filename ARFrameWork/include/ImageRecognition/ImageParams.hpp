#pragma once

namespace ar{

struct point{
    int x = 0; // temp图片最匹配时中心点的x坐标在image图片中的位置
    int y = 0; // temp图片最匹配时中心点的y坐标在image图片中的位置
    unsigned char data = 0; // todo
    bool is_empty = true; // 这里是标记是否找到了最匹配的位置
};

/**
 * @brief Image recognition type 图像识别方法的类型
 * enum struct 和 enum class等价
 */
enum struct imageRecognitionType{
    MPR,
    PSR,
    #if defined(ENABLE_CUDA)
    MPR_CUDA,
    #endif
};

}