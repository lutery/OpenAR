#pragma once

namespace ar{

struct point{
    int x = 0; // todo
    int y = 0; // todo
    unsigned char data = 0; // todo
    bool is_empty = true; // todo
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