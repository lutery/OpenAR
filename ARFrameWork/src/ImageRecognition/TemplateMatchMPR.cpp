#include <ImageRecognition/TemplateMatchMPR.h>
#include <math.h>
#include <random>

/**
 * @brief todo 看起来是模板匹配的多点对比算法
 * 
 * @param res 返回结果，记录的是template在image中匹配到的位置的左上角坐标 (x, y)
 * @param res_msg 返回信息，记录错误信息，但是i这里貌似无用
 * @param image 原图数据像素裸数据
 * @param temp 模板图数据像素裸数据
 * @param image_width 原图宽度
 * @param image_height 原图高度
 * @param temp_width 模板图宽度
 * @param temp_height 模板图高度
 * @param threshold todo 阈值
 * @param num_points todo 采样点数量
 */
bool ar::templateMatchMPR(int* res,
    std::string& res_msg,
    unsigned char* image,
    unsigned char* temp,
    const int& image_width,
    const int& image_height,
    const int& temp_width,
    const int& temp_height,
    const float& threshold,
    const int& num_points)
{
    bool err = false;

    int* compare_points_array = new int[2 * num_points];  // 存储模板图片均匀分块后，在每个分块内部随机采样的点的坐标，因为一个坐标由两个整数表示 (x, y)，所以数组大小为 2 * num_points
    // compare_points_array采样点的坐标是相对于模板图的左上角(0,0)的坐标
    ar::detail::makePointsRandom(compare_points_array, temp, temp_width, temp_height, num_points);

    size_t cur_min_ssd = (size_t)255 * 255 * num_points;
    // image_height - temp_height：表示在原图比模板图片大多少
    // image_width - temp_width：表示在原图比模板图片大多少
    // h和w按照模板图在原图中可以移动的位置进行遍历
    for (int h = 0; h <= image_height - temp_height; h++) {
        for (int w = 0; w <= image_width - temp_width; w++) {
            size_t ssd = 0;
            // 遍历所有采样点，todo 计算SSD（Sum of Squared Differences，平方差和）
            for (int t = 0; t < num_points; t++) {
                // 获取采样点在模板图中的坐标
                int temp_x = compare_points_array[t * 2];
                int temp_y = compare_points_array[t * 2 + 1];
                // 计算采样点在原图中的对应坐标，看来这里是将模板图放置在原图的 (w, h) 位置时，采样点在原图中的位置
                int image_x = w + temp_x;
                int image_y = h + temp_y;

                // 计算平方差并累加到ssd中，这是计算采样点在原图中的像素值与模板图中对应采样点的像素值之间的差异的平方
                ssd += (size_t)(image[image_y * image_width + image_x] - temp[temp_y * temp_width + temp_x]) *
                    (image[image_y * image_width + image_x] - temp[temp_y * temp_width + temp_x]);
            }

            // 如果当前的模板和图片的采样点的SSD小于阈值对应的最大允许SSD，并且小于当前最小SSD，则更新结果
            //  (size_t)num_points * 255 * 255 * (1 - threshold) * (1 - threshold)的公式由来看md文档
            if (ssd < (size_t)num_points * 255 * 255 * (1 - threshold) * (1 - threshold) && ssd < cur_min_ssd) {
                res[0] = w; res[1] = h;
                cur_min_ssd = ssd;
            }
        }
    }
    delete[] compare_points_array;
    return true;
}

/**
 * @brief 生成随机采样点，先将temp均分为指定的方块数量，然后在每个方块内部随机采样一个点存储搭配points中，总共采样num_points个点
 * @param points 采样点数组, 每个点由两个整数表示 (x, y),采用1维数组存储
 * @param temp 模板图像素裸数据
 * @param temp_width 模板图宽度
 * @param temp_height 模板图高度
 * @param num_points 采样点数量
 */
void ar::detail::makePointsRandom(int* points, unsigned char* temp, const int& temp_width, const int& temp_height, const int& num_points){
    int* res = new int[2]; // todo 这是什么？看解释因为是存储将图片均分的行数和列数
    findBestDivision(res, num_points);

    // 提取均分的行数和列数，这里使用的比较传统过的方式
    // 如果结合pair或者tuple，可是使用auto[]自动解包或者std::tie的方式实现自动解包
    int region_height = res[0];
    int region_width = res[1];

    // 计算分割后每个区域的高度和宽度
    int height_per_region = temp_height / region_height;
    int width_per_region = temp_width / region_width;

    std::random_device rd;
    std::mt19937 generator(rd());

    // 遍历分割的每个区域
    // 这段代码不是选择每个区域的中间点，而是在每个区域内随机选择一个点，确保采样点均匀分布在整个模板图像上。
    for(int i = 0; i < region_height; i++){
        for(int j = 0; j < region_width; j++){
            std::uniform_int_distribution<int> distribution_x(j * width_per_region, (j + 1) * width_per_region - 1);
            int x_ = distribution_x(generator);
            std::uniform_int_distribution<int> distribution_y(i * height_per_region, (i + 1) * height_per_region - 1);
            int y_ = distribution_y(generator);
            points[(i * region_width + j) * 2] = x_;
            points[(i * region_width + j) * 2 + 1] = y_;
        }
    } 
    // 安全的释放内存
    delete[] res;
}

/**
 * @brief 目的：将一个数字 num 分解为两个尽可能接近的因数，用于将模板图像划分为矩形网格。
 * 
 * @param res 返回结果，res[0] 为行数，res[1] 为列数 todo 意义？
 * @param num 需要分解的数字（采样点数量）
 */
void ar::detail::findBestDivision(int* res, const int& num){
    // 从 sqrt(num) 开始向下遍历，找到第一个能整除 num 的数
    // 相当于找到能够将num分解为两个尽可能接近的因数 axb=num
    // todo 为啥？
    for(int i = (int)std::sqrt(num); i >= 1; i--){
        if(num % i == 0) {
            res[0] = i;           // 行数（较小的因数）
            res[1] = num / i;     // 列数（较大的因数）
            return ;
        }
    }
}

