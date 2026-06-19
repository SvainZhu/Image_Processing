# Image Processing Tools

## 项目简介

`Image_Processing` 是一个基于 C++、OpenCV 和 Qt5 的基础图像处理项目。项目现在包含两层能力：

- `image_ops`：可复用的 C++ 图像处理函数库，不依赖 Qt，适合被其他 C++ 项目直接链接。
- `image_cli`：命令行图像处理工具，适合批处理、脚本调用和快速验证算法效果。
- `Image_Processing`：原 Qt5 可视化演示程序，适合交互式查看处理效果。

新增的 `image_ops` 覆盖常用图像处理操作：灰度化、尺寸变换、裁剪、旋转、翻转、直方图、均衡化、亮度/对比度、Gamma、阈值分割、滤波、形态学、边缘检测、锐化、噪声、颜色空间转换和伪彩色映射。

## 项目结构

```text
.
├── CMakeLists.txt
├── image_ops.h          # 新增：通用图像处理函数库头文件
├── image_ops.cpp        # 新增：通用图像处理函数库实现
├── image_cli.cpp        # 新增：命令行工具入口
├── base_proc.h          # 原 Qt 演示使用的基础函数
├── base_proc.cpp
├── MainWindow.h         # Qt GUI
├── MainWindow.cpp
├── main.cpp
├── mainwindow.ui
├── resources/
└── lib/opencv/
```

## 环境依赖

- C++ 14
- CMake 3.20+
- OpenCV 4.x
- Qt 5.15.x，仅在构建 Qt GUI 时需要

如果只需要函数库和命令行工具，可以关闭 Qt GUI：

```bash
cmake -S . -B build -DIMAGE_PROCESSING_BUILD_QT_APP=OFF
cmake --build build
```

如果需要同时构建 Qt GUI：

```bash
cmake -S . -B build -DIMAGE_PROCESSING_BUILD_QT_APP=ON
cmake --build build
```

可选构建开关：

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `IMAGE_PROCESSING_BUILD_CLI` | `ON` | 构建 `image_cli` 命令行工具 |
| `IMAGE_PROCESSING_BUILD_QT_APP` | `ON` | 构建 Qt 可视化程序 |

## 命令行工具用法

基本格式：

```bash
image_cli <input> <output> <operation> [key=value ...]
```

示例：

```bash
# 灰度化
image_cli resources/img/lena.jpg out_gray.jpg gray

# 保持比例缩放到不超过 512x512
image_cli resources/img/lena.jpg out_resize.jpg resize width=512 height=512 keep_aspect=true

# 裁剪图像
image_cli resources/img/lena.jpg out_crop.jpg crop x=80 y=60 width=200 height=160

# Canny 边缘检测
image_cli resources/img/lena.jpg out_canny.jpg canny low=60 high=150 aperture=3

# 直方图均衡化
image_cli resources/img/lena.jpg out_equalize.jpg equalize

# 形态学开运算
image_cli resources/img/lena.jpg out_open.jpg morphology type=open ksize=5 iterations=1

# 添加高斯噪声
image_cli resources/img/lena.jpg out_noise.jpg gaussian_noise mean=0 stddev=15 seed=42

# 使用伪彩色
image_cli resources/img/lena.jpg out_turbo.jpg colormap map=turbo
```

### CLI 操作列表

| 操作 | 参数 | 示例 |
| --- | --- | --- |
| `gray` / `grayscale` | 无 | `image_cli in.jpg out.jpg gray` |
| `resize` | `width`, `height`, `keep_aspect` | `image_cli in.jpg out.jpg resize width=640 height=480` |
| `crop` | `x`, `y`, `width`, `height` | `image_cli in.jpg out.jpg crop x=10 y=10 width=200 height=200` |
| `rotate` | `angle`, `scale` | `image_cli in.jpg out.jpg rotate angle=30 scale=1` |
| `flip` | `code=-1|0|1` | `image_cli in.jpg out.jpg flip code=1` |
| `histogram` | `width`, `height` | `image_cli in.jpg hist.jpg histogram` |
| `equalize` | 无 | `image_cli in.jpg out.jpg equalize` |
| `brightness` / `contrast` | `alpha`, `beta` | `image_cli in.jpg out.jpg brightness alpha=1.2 beta=15` |
| `gamma` | `gamma` | `image_cli in.jpg out.jpg gamma gamma=1.8` |
| `threshold` | `value`, `max` | `image_cli in.jpg out.jpg threshold value=128` |
| `otsu` | 无 | `image_cli in.jpg out.jpg otsu` |
| `adaptive` | `block`, `c`, `gaussian` | `image_cli in.jpg out.jpg adaptive block=15 c=3` |
| `blur` | `ksize` | `image_cli in.jpg out.jpg blur ksize=5` |
| `gaussian_blur` | `ksize`, `sigma` | `image_cli in.jpg out.jpg gaussian_blur ksize=7 sigma=1.5` |
| `median` | `ksize` | `image_cli in.jpg out.jpg median ksize=5` |
| `bilateral` | `diameter`, `sigma_color`, `sigma_space` | `image_cli in.jpg out.jpg bilateral diameter=9 sigma_color=75 sigma_space=75` |
| `morphology` | `type`, `ksize`, `iterations` | `image_cli in.jpg out.jpg morphology type=close ksize=5` |
| `canny` | `low`, `high`, `aperture` | `image_cli in.jpg out.jpg canny low=80 high=160` |
| `sobel` | `dx`, `dy`, `ksize` | `image_cli in.jpg out.jpg sobel dx=1 dy=0` |
| `laplacian` | `ksize` | `image_cli in.jpg out.jpg laplacian ksize=3` |
| `sharpen` | 无 | `image_cli in.jpg out.jpg sharpen` |
| `unsharp` | `amount`, `ksize`, `sigma` | `image_cli in.jpg out.jpg unsharp amount=1.4` |
| `salt_pepper` | `amount`, `seed` | `image_cli in.jpg out.jpg salt_pepper amount=0.03 seed=42` |
| `gaussian_noise` | `mean`, `stddev`, `seed` | `image_cli in.jpg out.jpg gaussian_noise stddev=12` |
| `colormap` | `map` | `image_cli in.jpg out.jpg colormap map=viridis` |
| `colorspace` | `code` | `image_cli in.jpg out.jpg colorspace code=bgr2hsv` |

`morphology type` 支持：`erode`、`dilate`、`open`、`close`、`gradient`、`tophat`、`blackhat`。

`colormap map` 支持常用 OpenCV colormap：`jet`、`viridis`、`turbo`、`hot`、`magma`、`inferno`、`plasma`、`cividis` 等。

`colorspace code` 支持：`bgr2rgb`、`rgb2bgr`、`bgr2gray`、`gray2bgr`、`bgr2hsv`、`hsv2bgr`、`bgr2lab`、`lab2bgr`、`bgr2ycrcb`、`ycrcb2bgr`。

## C++ 函数库用法

在自己的 C++ 项目中包含头文件并链接 `image_processing_ops`：

```cpp
#include "image_ops.h"

#include <opencv2/imgcodecs.hpp>

int main() {
    cv::Mat src = cv::imread("resources/img/lena.jpg");
    cv::Mat gray = imgproc::toGray(src);
    cv::Mat edges = imgproc::cannyEdge(gray, 60, 150);
    cv::Mat result = imgproc::unsharpMask(src, 1.2, 5, 1.0);

    cv::imwrite("edges.jpg", edges);
    cv::imwrite("sharp.jpg", result);
    return 0;
}
```

CMake 链接示例：

```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE image_processing_ops)
```

## `image_ops` 函数参考

### 几何与颜色

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `imgproc::toGray(src)` | 将 BGR/BGRA 图像转灰度；灰度图会直接复制返回 | `cv::Mat gray = imgproc::toGray(src);` |
| `imgproc::resizeImage(src, width, height, keepAspect)` | 缩放图像；`keepAspect=true` 时保持比例 | `auto small = imgproc::resizeImage(src, 512, 512, true);` |
| `imgproc::cropImage(src, rect)` | 裁剪 ROI，越界部分会安全裁切 | `auto roi = imgproc::cropImage(src, cv::Rect(10,10,128,128));` |
| `imgproc::rotateImage(src, angle, scale)` | 旋转图像并自动扩大画布避免裁剪 | `auto rotated = imgproc::rotateImage(src, 30);` |
| `imgproc::flipImage(src, flipCode)` | 翻转图像，`1` 水平、`0` 垂直、`-1` 双向 | `auto flipped = imgproc::flipImage(src, 1);` |
| `imgproc::convertColor(src, code)` | 按 OpenCV 色彩转换码转换 | `auto hsv = imgproc::convertColor(src, cv::COLOR_BGR2HSV);` |
| `imgproc::parseColorConversion(name)` | 将字符串转换为 OpenCV 色彩转换码 | `auto code = imgproc::parseColorConversion("bgr2lab");` |

### 直方图、增强与阈值

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `imgproc::histogramImage(src, width, height)` | 生成灰度直方图可视化图片 | `auto hist = imgproc::histogramImage(src);` |
| `imgproc::equalizeHistogram(src)` | 灰度图直接均衡化；彩色图对 Y 通道均衡化 | `auto eq = imgproc::equalizeHistogram(src);` |
| `imgproc::adjustBrightnessContrast(src, alpha, beta)` | 线性亮度/对比度调整：`dst = alpha * src + beta` | `auto bright = imgproc::adjustBrightnessContrast(src, 1.2, 10);` |
| `imgproc::gammaCorrection(src, gamma)` | Gamma 校正 | `auto corrected = imgproc::gammaCorrection(src, 1.8);` |
| `imgproc::thresholdBinary(src, threshold, max)` | 全局二值阈值分割 | `auto bin = imgproc::thresholdBinary(src, 128);` |
| `imgproc::thresholdOtsu(src)` | Otsu 自动阈值分割 | `auto bin = imgproc::thresholdOtsu(src);` |
| `imgproc::adaptiveThresholdImage(src, block, c, gaussian)` | 自适应阈值分割 | `auto bin = imgproc::adaptiveThresholdImage(src, 15, 3);` |
| `imgproc::normalizeToByte(src)` | 任意深度图像归一化到 8 位 | `auto view = imgproc::normalizeToByte(floatMap);` |

### 滤波与形态学

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `imgproc::meanBlurImage(src, ksize)` | 均值滤波 | `auto dst = imgproc::meanBlurImage(src, 5);` |
| `imgproc::gaussianBlurImage(src, ksize, sigma)` | 高斯滤波 | `auto dst = imgproc::gaussianBlurImage(src, 7, 1.5);` |
| `imgproc::medianBlurImage(src, ksize)` | 中值滤波，适合去除椒盐噪声 | `auto dst = imgproc::medianBlurImage(src, 5);` |
| `imgproc::bilateralFilterImage(src, diameter, sigmaColor, sigmaSpace)` | 双边滤波，尽量保边去噪 | `auto dst = imgproc::bilateralFilterImage(src, 9, 75, 75);` |
| `imgproc::morphologyImage(src, type, ksize, iterations)` | 形态学操作 | `auto dst = imgproc::morphologyImage(src, imgproc::MorphType::Close, 5);` |
| `imgproc::parseMorphType(name)` | 将字符串转换为 `MorphType` | `auto type = imgproc::parseMorphType("open");` |

### 边缘、锐化与噪声

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `imgproc::cannyEdge(src, low, high, aperture)` | Canny 边缘检测 | `auto edge = imgproc::cannyEdge(src, 60, 150);` |
| `imgproc::sobelEdge(src, dx, dy, ksize)` | Sobel 梯度边缘 | `auto gx = imgproc::sobelEdge(src, 1, 0);` |
| `imgproc::laplacianEdge(src, ksize)` | Laplacian 边缘 | `auto edge = imgproc::laplacianEdge(src);` |
| `imgproc::sharpen(src)` | 3x3 卷积锐化 | `auto sharp = imgproc::sharpen(src);` |
| `imgproc::unsharpMask(src, amount, ksize, sigma)` | 反遮罩锐化 | `auto sharp = imgproc::unsharpMask(src, 1.4);` |
| `imgproc::addSaltPepperNoise(src, amount, seed)` | 添加椒盐噪声，`amount` 范围为 `[0,1]` | `auto noisy = imgproc::addSaltPepperNoise(src, 0.02, 42);` |
| `imgproc::addGaussianNoise(src, mean, stddev, seed)` | 添加高斯噪声 | `auto noisy = imgproc::addGaussianNoise(src, 0, 12, 42);` |
| `imgproc::applyColorMapImage(src, colorMap)` | 伪彩色映射 | `auto color = imgproc::applyColorMapImage(src, cv::COLORMAP_TURBO);` |
| `imgproc::parseColorMap(name)` | 将字符串转换为 OpenCV colormap | `auto map = imgproc::parseColorMap("viridis");` |

## 原有 `base_proc` 函数

`base_proc` 仍服务于 Qt GUI 演示，函数名保持兼容。当前已修复直方图绘制、Otsu 阈值、双阈值连接、LBP 初始化、灰度/彩色噪声等基础问题。

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `gray_to_hist(gray)` | 生成 256 级灰度直方图图像 | `cv::Mat hist = gray_to_hist(gray);` |
| `gray_to_vector(gray)` | 统计 256 级灰度频次 | `QVector<int> hist = gray_to_vector(gray);` |
| `add_salt_noise(src, intensity)` | 添加指定数量的椒盐噪声点 | `cv::Mat noisy = add_salt_noise(src, 500);` |
| `add_gaussian_noise(src, intensity)` | 添加高斯噪声 | `cv::Mat noisy = add_gaussian_noise(src, 20);` |
| `double_threshold(src, low, high)` | 双阈值初筛，强边缘设为 255，弱边缘保留 | `double_threshold(edges, 50, 120);` |
| `double_threshold_link(src, low, high)` | 双阈值边缘连接 | `double_threshold_link(edges, 50, 120);` |
| `lbp_operator(src, dst, radius, neighbors)` | 圆形 LBP 特征 | `lbp_operator(gray, lbp, 1, 8);` |
| `sum_of_rect(src, rect)` | 按积分图公式计算矩形区域和 | `double s = sum_of_rect(integral, rect);` |
| `get_OSTU(hist)` | 根据直方图计算 Otsu 阈值 | `int t = get_OSTU(hist);` |

## Qt 可视化程序

Qt 程序保留原有按钮演示流程，包含：

1. 图像预处理：RGB 转灰度、直方图、直方图均衡、梯度锐化、Laplacian 锐化。
2. 噪声添加：椒盐噪声、高斯噪声。
3. 边缘检测：Roberts、Sobel、Prewitt、Laplacian、Kirsch、Canny。
4. 滤波：均值、中值、高斯、形态学滤波等。
5. 特征与变换：LBP、SIFT、ORB、Haar、Gabor、仿射、透视等演示。

## 常见问题

### 只想用命令行，为什么还报 Qt 找不到？

关闭 GUI：

```bash
cmake -S . -B build -DIMAGE_PROCESSING_BUILD_QT_APP=OFF
```

### 找不到 OpenCV 怎么办？

确认本机安装 OpenCV，并在 CMake 中指定：

```bash
cmake -S . -B build -DOpenCV_DIR=/path/to/opencv/lib/cmake/opencv4
```

Windows 下如果使用仓库中的 `lib/opencv`，CMake 会自动尝试读取 `lib/opencv/OpenCVConfig.cmake`。Linux/macOS 不会自动使用该目录，因为其中的配置和库文件是 Windows/MSVC 路径。

### CLI 输出失败怎么办？

请确认输出目录已经存在。`image_cli` 不会自动创建父目录。
