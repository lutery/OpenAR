# #codebase 结合整体代码，讲解一下findBestDivision方法？
Using "Codebase"

Searched text for `findBestDivision`, 6 results

Read [](file:///e%3A/Project/OpenSource/cpp/OpenAR/ARFrameWork/include/ImageRecognition/TemplateMatchMPR.h)

Searched text for `makePointsRandom|templateMatchMPR`, 45 results

# `findBestDivision` 方法详解

## 方法签名与位置

```cpp
namespace ar {
    namespace detail {
        void findBestDivision(int* res, const int& num);
    }
}
```

这是一个辅助函数，存在于两个地方：
1. **CPU 版本**：`ARFrameWork/src/ImageRecognition/TemplateMatchMPR.cpp`
2. **CUDA 版本**：`ARCuda/src/TemplateMatchMPRCuda.cu` - 实现完全相同

## 核心功能

**目的**：将一个数字 `num` 分解为两个尽可能接近的因数，用于将模板图像划分为矩形网格。

### 算法实现

```cpp
void ar::detail::findBestDivision(int* res, const int& num){
    // 从 sqrt(num) 开始向下遍历，找到第一个能整除 num 的数
    for(int i = (int)std::sqrt(num); i >= 1; i--){
        if(num % i == 0) {
            res[0] = i;           // 行数（较小的因数）
            res[1] = num / i;     // 列数（较大的因数）
            return;
        }
    }
}
```

### 为什么从 sqrt(num) 开始？

- **数学原理**：如果 `num = a × b`，且 `a ≤ b`，则 `a ≤ √num ≤ b`
- **最优分解**：从 `√num` 向下找到的第一个因数，能保证两个因数最接近（最"方正"的矩形）

## 在 MPR 算法中的应用

### 调用链

```
templateMatchMPR / templateMatchMPRCuda
    └─> makePointsRandom (生成随机采样点)
        └─> findBestDivision (计算网格划分)
```

### 完整流程示例

假设 `num_points = 36`：

```cpp
// 1. 调用 findBestDivision
int* res = new int[2];
findBestDivision(res, 36);  // res[0] = 6, res[1] = 6

// 2. 使用结果划分模板图像
int region_height = 6;  // 6 行
int region_width = 6;   // 6 列

// 3. 在每个 6×6 的区域内随机选择 1 个采样点
// 假设模板图像 60×60 像素
int height_per_region = 60 / 6 = 10;  // 每个区域高度 10 像素
int width_per_region = 60 / 6 = 10;   // 每个区域宽度 10 像素

// 4. 生成 36 个均匀分布的随机采样点
for(int i = 0; i < 6; i++){         // 遍历 6 行
    for(int j = 0; j < 6; j++){     // 遍历 6 列
        // 在第 (i, j) 个区域内随机选点
        int x = random(j*10, (j+1)*10-1);
        int y = random(i*10, (i+1)*10-1);
        points[(i*6 + j)*2] = x;
        points[(i*6 + j)*2 + 1] = y;
    }
}
```

### 不同 num_points 的分解结果

| num_points | res[0] (行) | res[1] (列) | 网格形状 |
|-----------|------------|------------|---------|
| 36 | 6 | 6 | 正方形 6×6 |
| 24 | 4 | 6 | 矩形 4×6 |
| 25 | 5 | 5 | 正方形 5×5 |
| 30 | 5 | 6 | 矩形 5×6 |
| 17 | 1 | 17 | 极端矩形 1×17 |

## 设计意图

### 为什么要均匀划分？

```cpp
// ❌ 错误做法：完全随机采样
for(int i = 0; i < num_points; i++){
    int x = random(0, temp_width);
    int y = random(0, temp_height);
    // 问题：采样点可能聚集在某个角落，无法覆盖整个模板
}

// ✅ 正确做法：分网格后在每个格子内随机
// 优势：保证采样点均匀覆盖整个模板图像
```

### 视觉示例

假设 36 个采样点在 60×60 模板上的分布：

```
┌─────┬─────┬─────┬─────┬─────┬─────┐
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │  每个格子 10×10 像素
├─────┼─────┼─────┼─────┼─────┼─────┤  · 表示随机采样点
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │  确保每个区域都有 1 个点
├─────┼─────┼─────┼─────┼─────┼─────┤
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │
├─────┼─────┼─────┼─────┼─────┼─────┤
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │
├─────┼─────┼─────┼─────┼─────┼─────┤
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │
├─────┼─────┼─────┼─────┼─────┼─────┤
│  ·  │  ·  │  ·  │  ·  │  ·  │  ·  │
└─────┴─────┴─────┴─────┴─────┴─────┘
```

## MPR 算法的核心思想

**Multi-Point Recognition (MPR)** = 多点识别算法：

1. **采样**：不比较所有像素，只比较 `num_points` 个关键点（如 36 个）
2. **均匀分布**：通过 `findBestDivision` 确保采样点覆盖整个模板
3. **快速匹配**：计算采样点的 SSD（Sum of Squared Differences）
   ```cpp
   ssd += (image[y*w+x] - temp[ty*tw+tx])²
   ```
4. **阈值判断**：当 `ssd < threshold` 时认为匹配成功

### 性能优势

- **完整模板匹配**：需要比较 `temp_width × temp_height` 个像素
- **MPR 算法**：只需比较 `num_points` 个像素（通常 36 个）
- **加速比**：对于 100×100 模板，`10000 / 36 ≈ 278` 倍加速

## 总结

`findBestDivision` 是 MPR 算法的关键辅助函数，它通过数学优化确保：
1. ✅ 采样点均匀分布在模板上
2. ✅ 网格尽可能接近正方形（`res[0] ≈ res[1]`）
3. ✅ 在保证覆盖率的前提下减少计算量

这种设计体现了 **空间均匀采样 + 统计匹配** 的图像识别思想，是 OpenAR 框架高效执行游戏自动化任务的核心技术之一。