#include <ImageRecognition/TemplateMatchPSR.h>

/**
 * @brief 使用PSR算法进行模板匹配
 * 
 * @param int* res: 存储找到的坐标位置
 * @param res_msg: 错误的信息
 * @param image: 原图的像素裸数据
 * @param temp: 模板图的像素裸数据
 * @param image_width: 原图的图片宽度
 * @param image_hegiht: 原图的图片高度
 * @param temp_width: 模板图片的宽度
 * @param temp_height: 模板图片的高度
 * @param threshold: 匹配阈值
 */
bool ar::templateMatchPSR(int* res,
	std::string& res_msg,
	unsigned char* image,
	unsigned char* temp,
	const int& image_width,
	const int& image_height,
	const int& temp_width,
	const int& temp_height,
	const float& threshold)
{
	// 创建的是一个和原图一样大的int数组
	// 用于创建保存一个快速计算任意矩形区域的像素和，该矩阵的每一个位置
	// 包含从当前位置到左上角所构成矩阵的所有像素和，用于快速计算任意区域矩阵位置的像素和
	int* prefix_sum_array = new int[image_width * image_height];
	ar::detail::generateImagePrefixSumArray(prefix_sum_array, image, image_width, image_height);

	// 这边就是单纯计算模板图片的像素总和了
	int temp_sum = 0;
	for (int i = 0; i < temp_height; i++) {
		for (int j = 0; j < temp_width; j++) {
			temp_sum += temp[i * temp_width + j];
		}
	}

	int cur_min_diff = 255 * temp_width * temp_height;
	for (int i = temp_height - 1; i < image_height; i++) {
		for (int j = temp_width - 1; j < image_width; j++) {
			if (i == temp_height - 1 && j == temp_width - 1) {
				int region_sum = prefix_sum_array[i * image_width + j];
				int region_diff = abs(region_sum - temp_sum);
				if (region_diff < 255 * temp_height * temp_width * (1 - threshold) && region_diff < cur_min_diff) {
					res[0] = j; res[1] = i;
					cur_min_diff = region_diff;
				}
			}
			else if (i > temp_height - 1 && j == temp_width - 1) {
				int region_sum = prefix_sum_array[i * image_width + j] - prefix_sum_array[(i - temp_height) * image_width + j];
				int region_diff = abs(region_sum - temp_sum);
				if (region_diff < 255 * temp_height * temp_width * (1 - threshold) && region_diff < cur_min_diff) {
					res[0] = j; res[1] = i;
					cur_min_diff = region_diff;
				}
			}
			else if (i == temp_height - 1 && j > temp_width - 1) {
				int region_sum = prefix_sum_array[i * image_width + j] - prefix_sum_array[i * image_width + j - temp_width];
				int region_diff = abs(region_sum - temp_sum);
				if (region_diff < 255 * temp_height * temp_width * (1 - threshold) && region_diff < cur_min_diff) {
					res[0] = j; res[1] = i;
					cur_min_diff = region_diff;
				}
			}
			else {
				int region_sum = prefix_sum_array[i * image_width + j] - prefix_sum_array[i * image_width + j - temp_width] - prefix_sum_array[(i - temp_height) * image_width + j] + prefix_sum_array[(i - temp_height) * image_width + j - temp_width];
				int region_diff = abs(region_sum - temp_sum);
				if (region_diff < 255 * temp_height * temp_width * (1 - threshold) && region_diff < cur_min_diff) {
					res[0] = j; res[1] = i;
					cur_min_diff = region_diff;
				}
			}
		}
	}
	delete[] prefix_sum_array;
	return true;
}

/**
 * @brief 生成图像的二维前缀和数组（Prefix Sum Array / Integral Image），这是一种用于快速计算图像任意矩形区域像素和的数据结构
 * 具体看md文件
 * @param dst_array todo
 * @param image 原图裸数据
 * @param image_width 原图的宽度
 * @param image_height 原图的高度
 */
void ar::detail::generateImagePrefixSumArray(int* dst_array, unsigned char* image, const int& image_width, const int& image_height)
{
	for (int i = 0; i < image_height; i++) {
		for (int j = 0; j < image_width; j++) {
			//res[0][0] = res[0][0] 情况 1：左上角起点 (0, 0)
			if (i == 0 && j == 0)dst_array[i * image_width + j] = image[i * image_width + j];
			//res[i][0] = image[i][0] + res[i-1][0] // 情况 2：第一列（j=0），只能从上方累加
			else if (i > 0 && j == 0) dst_array[i * image_width + j] = image[i * image_width + j] + dst_array[(i - 1) * image_width + j];
			//res[0][j] = image[0][j] + res[0][j-1]; 情况 3：第一行（i=0），只能从左侧累加
			else if (i == 0 && j > 0) dst_array[i * image_width + j] = image[i * image_width + j] + dst_array[i * image_width + j - 1];
			//res[i][j] = image[i][j] + res[i][j-1] + res[i-1][j] - res[i-1][j-1] 情况 4：一般情况，使用容斥原理
			//                          左侧区域和     上方区域和     减去重复计算的左上角
			else dst_array[i * image_width + j] = image[i * image_width + j] + dst_array[i * image_width + j - 1] + dst_array[(i - 1) * image_width + j] - dst_array[(i - 1) * image_width + j - 1];
		}
	}
}