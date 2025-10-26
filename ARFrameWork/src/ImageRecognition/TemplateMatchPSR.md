# `generateImagePrefixSumArray` 函数详解

## 函数作用

生成图像的**二维前缀和数组**（Prefix Sum Array / Integral Image），这是一种用于快速计算图像任意矩形区域像素和的数据结构。

## 核心概念：二维前缀和

### 定义

前缀和数组 `dst_array[i][j]` 存储的是**从图像左上角 (0,0) 到当前位置 (i,j) 所形成的矩形区域内所有像素值的总和**。

```
原始图像 image:
┌───┬───┬───┬───┐
│ 5 │ 2 │ 3 │ 1 │
├───┼───┼───┼───┤
│ 4 │ 6 │ 1 │ 2 │
├───┼───┼───┼───┤
│ 7 │ 3 │ 8 │ 4 │
└───┴───┴───┴───┘

前缀和数组 dst_array:
┌────┬────┬────┬────┐
│  5 │  7 │ 10 │ 11 │  ← dst_array[0][3] = 5+2+3+1 = 11
├────┼────┼────┼────┤
│  9 │ 17 │ 21 │ 24 │
├────┼────┼────┼────┤
│ 16 │ 27 │ 39 │ 46 │  ← dst_array[2][3] = 所有像素和 = 46
└────┴────┴────┴────┘
```

## 代码逐行解析

```cpp
void ar::detail::generateImagePrefixSumArray(int* dst_array, unsigned char* image, const int& image_width, const int& image_height)
{
    for (int i = 0; i < image_height; i++) {
        for (int j = 0; j < image_width; j++) {
            
            // 情况 1：左上角起点 (0, 0)
            if (i == 0 && j == 0)
                dst_array[i * image_width + j] = image[i * image_width + j];
            
            // 情况 2：第一列（j=0），只能从上方累加
            // dst[i][0] = image[i][0] + dst[i-1][0]
            else if (i > 0 && j == 0) 
                dst_array[i * image_width + j] = image[i * image_width + j] + dst_array[(i - 1) * image_width + j];
            
            // 情况 3：第一行（i=0），只能从左侧累加
            // dst[0][j] = image[0][j] + dst[0][j-1]
            else if (i == 0 && j > 0) 
                dst_array[i * image_width + j] = image[i * image_width + j] + dst_array[i * image_width + j - 1];
            
            // 情况 4：一般情况，使用容斥原理
            // dst[i][j] = image[i][j] + dst[i][j-1] + dst[i-1][j] - dst[i-1][j-1]
            else 
                dst_array[i * image_width + j] = image[i * image_width + j] 
                    + dst_array[i * image_width + j - 1]      // 左侧区域和
                    + dst_array[(i - 1) * image_width + j]    // 上方区域和
                    - dst_array[(i - 1) * image_width + j - 1]; // 减去重复计算的左上角
        }
    }
}
```

### 容斥原理图解（情况 4）

```
计算 dst[i][j]（蓝色区域的总和）：

      0   1   2   j-1  j
    ┌───┬───┬───┬───┬───┐
  0 │   │   │   │   │   │
    ├───┼───┼───┼───┼───┤
  1 │   │ A │ A │ A │ B │  A = dst[i-1][j-1]（左上角重叠区）
    ├───┼───┼───┼───┼───┤  B = dst[i-1][j]（上方区域）
i-1 │   │ A │ A │ A │ B │  C = dst[i][j-1]（左侧区域）
    ├───┼───┼───┼───┼───┤  P = image[i][j]（当前像素）
  i │   │ C │ C │ C │ P │
    └───┴───┴───┴───┴───┘

公式推导：
dst[i][j] = (A + B) + (A + C) + P - A
          = A + B + C + P - A
          = B + C - A + P
          = dst[i-1][j] + dst[i][j-1] - dst[i-1][j-1] + image[i][j]
          
解释：
- dst[i-1][j] 包含了 A + B
- dst[i][j-1] 包含了 A + C
- 两者相加会重复计算 A，所以减去 dst[i-1][j-1]
- 最后加上当前像素值 image[i][j]
```

## 在 PSR 算法中的应用

### 快速计算任意矩形区域的像素和

有了前缀和数组后，计算任意矩形区域 `(x1,y1)` 到 `(x2,y2)` 的像素和只需 **O(1)** 时间：

```cpp
// PSR 算法中的四种情况对应四种矩形区域计算

// 情况 1：左上角矩形（右下角在 (i, j)）
int region_sum = prefix_sum_array[i * image_width + j];

// 情况 2：去掉上方 temp_height 行
int region_sum = prefix_sum_array[i * image_width + j] 
               - prefix_sum_array[(i - temp_height) * image_width + j];

// 情况 3：去掉左侧 temp_width 列
int region_sum = prefix_sum_array[i * image_width + j] 
               - prefix_sum_array[i * image_width + j - temp_width];

// 情况 4：一般矩形（容斥原理）
int region_sum = prefix_sum_array[i * image_width + j]
               - prefix_sum_array[i * image_width + j - temp_width]
               - prefix_sum_array[(i - temp_height) * image_width + j]
               + prefix_sum_array[(i - temp_height) * image_width + j - temp_width];
```

### 可视化示例

```
计算模板区域（红框）的像素和：

前缀和数组：
      0    1    2    3    4
    ┌────┬────┬────┬────┬────┐
  0 │  5 │  7 │ 10 │ 11 │ 13 │
    ├────┼────┼────┼────┼────┤
  1 │  9 │ 17 │ 21 │ 24 │ 28 │  假设模板 3×2
    ├────┼────┼────┼────┼────┤  右下角在 (2, 3)
  2 │ 16 │ 27 │ 39 │ 46 │ 52 │  左上角在 (0, 1)
    └────┴────┴────┴────┴────┘
         ↑              ↑
       左上角          右下角

region_sum = prefix[2][3]              // 大矩形 D
           - prefix[2][1]              // 减去左侧 A
           - prefix[-1][3]             // 减去上方 B（越界，为0）
           + prefix[-1][1]             // 加回重复减的 C（越界，为0）
           = 46 - 27 = 19

实际验证：
原始图像中 (0,1) 到 (2,3) 区域的像素和：
2+3+1 + 6+1+2 + 3+8+4 = 6 + 9 + 15 = 30  ❌

// 修正：这里的计算有误，实际应该是：
region_sum = prefix[2][3]              
           - prefix[2][0]              // (i, j-temp_width) = (2, 0)
           - prefix[-1][3]             // (i-temp_height, j) 越界
           + prefix[-1][0]             // 越界
```

## PSR 算法的完整流程

```cpp
// 1. 生成原图的前缀和数组 - O(W×H)
generateImagePrefixSumArray(prefix_sum_array, image, image_width, image_height);

// 2. 计算模板图的像素总和 - O(tw×th)
int temp_sum = sum_all_pixels(temp);

// 3. 滑动窗口遍历所有可能的匹配位置 - O((W-tw)×(H-th))
for (int i = temp_height - 1; i < image_height; i++) {
    for (int j = temp_width - 1; j < image_width; j++) {
        
        // 使用前缀和数组快速计算当前窗口的像素和 - O(1)
        int region_sum = calculate_using_prefix_sum(i, j);
        
        // 计算与模板的差异
        int region_diff = abs(region_sum - temp_sum);
        
        // 判断是否满足阈值
        if (region_diff < threshold && region_diff < cur_min_diff) {
            res[0] = j - temp_width + 1;  // 左上角 x
            res[1] = i - temp_height + 1; // 左上角 y
            cur_min_diff = region_diff;
        }
    }
}
```

## PSR vs MPR 的区别

| 特性 | PSR（Prefix Sum Recognition） | MPR（Multi-Point Recognition） |
|------|------------------------------|-------------------------------|
| **比较对象** | 区域像素总和 | 采样点像素值 |
| **预处理** | 生成前缀和数组 O(W×H) | 生成随机采样点 O(n) |
| **匹配时间** | O((W-tw)×(H-th)) | O((W-tw)×(H-th)×n) |
| **内存消耗** | O(W×H) int 数组 | O(n) int 数组 |
| **精度** | 较低（只比较总和） | 较高（比较多个点的像素值） |
| **适用场景** | 快速粗略匹配 | 精确匹配 |

## 总结

1. **`generateImagePrefixSumArray`** 使用**动态规划**思想构建二维前缀和数组
2. **容斥原理**是核心：`dst[i][j] = 左 + 上 - 左上 + 当前`
3. **PSR 算法**利用前缀和实现 **O(1)** 时间计算任意矩形区域像素和
4. **权衡**：PSR 速度快但精度低，MPR 精度高但速度慢
5. **实际应用**：OpenAR 框架默认使用 MPR 算法（`num_points=36`），在需要极致性能时可切换到 PSR

这种数据结构在计算机视觉中非常常见，OpenCV 的 `cv::integral()` 函数也是同样的原理。