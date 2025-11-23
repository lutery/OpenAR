#pragma once

namespace ar{

struct point{
    int x = 0; // temp图片最匹配时中心点的x坐标在image图片中的位置
    int y = 0; // temp图片最匹配时中心点的y坐标在image图片中的位置
    unsigned char data = 0; // todo
    bool is_empty = true; // 这里是标记是否找到了最匹配的位置
};

/**
 * [图像增强 相似度比对 采用库中质量好的图像作为输入 图像相似度匹配算法_liutao988的技术博客_51CTO博客](https://blog.51cto.com/u_13317/10778091)
 * @brief Image recognition type 图像识别方法的类型
 * enum struct 和 enum class等价
 * 
 * todo 继续增加MSD NCC SSDA SATD四个图像对比的方法
 */
enum struct imageRecognitionType{
    MPR, // SSD
    PSR, // SAD
    #if defined(ENABLE_CUDA)
    MPR_CUDA,
    #endif
};

}