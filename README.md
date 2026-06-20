# Image Processing Tools

## 项目简介

`Image_Processing` 是一个基于 C++、OpenCV 和 Qt5 的基础图像处理项目。项目现在包含两层能力：

- `image_ops`：可复用的 C++ 图像处理函数库，不依赖 Qt，适合被其他 C++ 项目直接链接。
- `image_cli`：命令行图像处理工具，适合批处理、脚本调用和快速验证算法效果。
- `Image_Processing`：原 Qt5 可视化演示程序，适合交互式查看处理效果。

新增的 `image_ops` 覆盖常用图像处理操作：灰度化、尺寸变换、裁剪、旋转、翻转、直方图、均衡化、CLAHE、亮度/对比度、Gamma、白平衡、鲜明度、饱和度、色温/色调、色彩曲线、预设滤镜、自动修图、EXIF 元数据读取、阈值分割、滤波、非局部均值去噪、形态学、边缘检测、轮廓、连通域、距离变换、分水岭、Hough 检测、模板匹配、ORB 特征、图像修复、金字塔、仿射变换、锐化、噪声、颜色空间转换和伪彩色映射。

## 常用工具箱能力映射

参考 OpenCV、MATLAB Image Processing Toolbox、scikit-image、Pillow 等工具箱的常见功能，本项目将功能分成以下几类，并优先使用 OpenCV 内部优化实现以获得较好的性能和数值稳定性：

| 工具箱常见能力 | 本项目对应能力 |
| --- | --- |
| 图像读写、颜色空间、几何变换 | `colorspace`、`resize`、`crop`、`rotate`、`flip`、`affine` |
| 灰度增强、对比度增强、曝光校正 | `equalize`、`clahe`、`brightness`、`gamma`、`white_balance` |
| 阈值、区域和形态学处理 | `threshold`、`otsu`、`adaptive`、`morphology`、`components`、`contours` |
| 去噪和滤波 | `blur`、`gaussian_blur`、`median`、`bilateral`、`nlm_denoise` |
| 边缘、线、圆和距离分析 | `canny`、`auto_canny`、`sobel`、`laplacian`、`gradient`、`hough_lines`、`hough_circles`、`distance_transform` |
| 分割和对象分析 | `watershed`、`connected_components`、`contours` |
| 特征和匹配 | `orb`、`template_match` |
| 图像恢复和多尺度 | `inpaint`、`pyrdown`、`pyrup` |

## 手机相册编辑能力映射

参考 Apple Photos 和 Google Photos 等手机/相册编辑器的常见功能，本项目新增了面向照片后期的编辑层：

| 相册常见功能 | 本项目实现 |
| --- | --- |
| 自动增强、建议编辑 | `auto_enhance`：白平衡、自动色阶、局部 CLAHE、轻量锐化和自适应鲜明度 |
| 手动调整亮度、对比度、饱和度、鲜明度、色偏 | `brightness`、`saturation`、`vibrance`、`temperature` |
| 色彩曲线/色阶 | `curve`，支持全通道或单独 `r/g/b` 通道 |
| 滤镜/风格 | `filter`，内置人像、风景、美食、夜景、文档、电影感、冷暖色、黑白、褪色、复古等预设 |
| 照片详情页的拍摄参数 | `metadata`，读取 JPEG EXIF 中的 ISO、焦距、光圈、快门、机型、镜头等标签 |

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
image_cli <input> metadata
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

# CLAHE 局部对比度增强，适合光照不均图像
image_cli resources/img/lena.jpg out_clahe.jpg clahe clip=2.0 tile=8

# 形态学开运算
image_cli resources/img/lena.jpg out_open.jpg morphology type=open ksize=5 iterations=1

# 非局部均值去噪，保留纹理细节
image_cli resources/img/lena.jpg out_denoise.jpg nlm_denoise h=10 template=7 search=21

# 自动阈值 Canny 边缘检测
image_cli resources/img/lena.jpg out_auto_canny.jpg auto_canny sigma=0.33

# 连通域可视化
image_cli resources/img/lena.jpg out_components.jpg components min_area=50

# ORB 特征点可视化
image_cli resources/img/lena.jpg out_orb.jpg orb features=800

# 模板匹配并框出最佳匹配区域
image_cli resources/img/lena.jpg out_match.jpg template_match template=patch.jpg method=ccoeff_normed

# 添加高斯噪声
image_cli resources/img/lena.jpg out_noise.jpg gaussian_noise mean=0 stddev=15 seed=42

# 使用伪彩色
image_cli resources/img/lena.jpg out_turbo.jpg colormap map=turbo

# 手机相册式鲜明度和饱和度调整
image_cli resources/img/lena.jpg out_vibrance.jpg vibrance amount=0.35
image_cli resources/img/lena.jpg out_saturation.jpg saturation factor=1.2

# 曲线调整，支持 all/r/g/b 通道
image_cli resources/img/lena.jpg out_curve.jpg curve points=0:0,64:58,128:140,255:255 channel=all

# 一键滤镜和自动修图
image_cli resources/img/lena.jpg out_portrait.jpg filter preset=portrait intensity=0.85
image_cli resources/img/lena.jpg out_auto.jpg auto_enhance strength=1

# 读取照片元数据，不需要输出文件
image_cli resources/img/lena.jpg metadata
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
| `clahe` | `clip`, `tile` | `image_cli in.jpg out.jpg clahe clip=2.0 tile=8` |
| `brightness` / `contrast` | `alpha`, `beta` | `image_cli in.jpg out.jpg brightness alpha=1.2 beta=15` |
| `gamma` | `gamma` | `image_cli in.jpg out.jpg gamma gamma=1.8` |
| `white_balance` | 无 | `image_cli in.jpg out.jpg white_balance` |
| `saturation` | `factor` | `image_cli in.jpg out.jpg saturation factor=1.2` |
| `vibrance` | `amount`，范围 `[-1,1]` | `image_cli in.jpg out.jpg vibrance amount=0.35` |
| `temperature` | `temperature`, `tint`，范围 `[-1,1]` | `image_cli in.jpg out.jpg temperature temperature=0.12 tint=0.02` |
| `curve` | `points`, `channel` | `image_cli in.jpg out.jpg curve points=0:0,128:140,255:255` |
| `filter` | `preset`, `intensity` | `image_cli in.jpg out.jpg filter preset=landscape intensity=0.9` |
| `auto_enhance` | `strength` | `image_cli in.jpg out.jpg auto_enhance strength=1` |
| `metadata` | 无 | `image_cli in.jpg metadata` |
| `threshold` | `value`, `max` | `image_cli in.jpg out.jpg threshold value=128` |
| `otsu` | 无 | `image_cli in.jpg out.jpg otsu` |
| `adaptive` | `block`, `c`, `gaussian` | `image_cli in.jpg out.jpg adaptive block=15 c=3` |
| `blur` | `ksize` | `image_cli in.jpg out.jpg blur ksize=5` |
| `gaussian_blur` | `ksize`, `sigma` | `image_cli in.jpg out.jpg gaussian_blur ksize=7 sigma=1.5` |
| `median` | `ksize` | `image_cli in.jpg out.jpg median ksize=5` |
| `bilateral` | `diameter`, `sigma_color`, `sigma_space` | `image_cli in.jpg out.jpg bilateral diameter=9 sigma_color=75 sigma_space=75` |
| `nlm_denoise` | `h`, `template`, `search` | `image_cli in.jpg out.jpg nlm_denoise h=10 template=7 search=21` |
| `morphology` | `type`, `ksize`, `iterations` | `image_cli in.jpg out.jpg morphology type=close ksize=5` |
| `canny` | `low`, `high`, `aperture` | `image_cli in.jpg out.jpg canny low=80 high=160` |
| `auto_canny` | `sigma`, `aperture` | `image_cli in.jpg out.jpg auto_canny sigma=0.33` |
| `sobel` | `dx`, `dy`, `ksize` | `image_cli in.jpg out.jpg sobel dx=1 dy=0` |
| `laplacian` | `ksize` | `image_cli in.jpg out.jpg laplacian ksize=3` |
| `gradient` | `ksize`, `scharr` | `image_cli in.jpg out.jpg gradient scharr=true` |
| `contours` | `value`, `otsu`, `thickness` | `image_cli in.jpg out.jpg contours otsu=true thickness=2` |
| `components` | `min_area` | `image_cli in.jpg out.jpg components min_area=50` |
| `distance_transform` | 无 | `image_cli in.jpg out.jpg distance_transform` |
| `watershed` | 无 | `image_cli in.jpg out.jpg watershed` |
| `hough_lines` | `rho`, `theta`, `threshold`, `min_length`, `max_gap` | `image_cli in.jpg out.jpg hough_lines threshold=80` |
| `hough_circles` | `dp`, `min_dist`, `param1`, `param2`, `min_radius`, `max_radius` | `image_cli in.jpg out.jpg hough_circles min_dist=40` |
| `template_match` | `template`, `method` | `image_cli in.jpg out.jpg template_match template=patch.jpg` |
| `orb` | `features` | `image_cli in.jpg out.jpg orb features=800` |
| `inpaint` | `mask`, `radius` | `image_cli in.jpg out.jpg inpaint mask=mask.png radius=3` |
| `pyrdown` / `pyrup` | `levels` | `image_cli in.jpg out.jpg pyrdown levels=2` |
| `affine` | `a00`, `a01`, `a02`, `a10`, `a11`, `a12`, `width`, `height` | `image_cli in.jpg out.jpg affine a00=1 a01=0 a02=20 a10=0 a11=1 a12=10` |
| `sharpen` | 无 | `image_cli in.jpg out.jpg sharpen` |
| `unsharp` | `amount`, `ksize`, `sigma` | `image_cli in.jpg out.jpg unsharp amount=1.4` |
| `salt_pepper` | `amount`, `seed` | `image_cli in.jpg out.jpg salt_pepper amount=0.03 seed=42` |
| `gaussian_noise` | `mean`, `stddev`, `seed` | `image_cli in.jpg out.jpg gaussian_noise stddev=12` |
| `colormap` | `map` | `image_cli in.jpg out.jpg colormap map=viridis` |
| `colorspace` | `code` | `image_cli in.jpg out.jpg colorspace code=bgr2hsv` |

`morphology type` 支持：`erode`、`dilate`、`open`、`close`、`gradient`、`tophat`、`blackhat`。

`colormap map` 支持常用 OpenCV colormap：`jet`、`viridis`、`turbo`、`hot`、`magma`、`inferno`、`plasma`、`cividis` 等。

`colorspace code` 支持：`bgr2rgb`、`rgb2bgr`、`bgr2gray`、`gray2bgr`、`bgr2hsv`、`hsv2bgr`、`bgr2lab`、`lab2bgr`、`bgr2ycrcb`、`ycrcb2bgr`。

`template_match method` 支持：`sqdiff`、`sqdiff_normed`、`ccorr`、`ccorr_normed`、`ccoeff`、`ccoeff_normed`。

`filter preset` 支持：`vivid`、`landscape`、`portrait`、`food`、`night`、`document`、`cinematic`、`warm`、`cool`、`mono`、`fade`、`vintage`。

`metadata` 会输出 `key=value`。JPEG EXIF 常见输出包括：`iso`、`focal_length_mm`、`focal_length_35mm`、`aperture`、`exposure_time_s`、`make`、`model`、`lens_model`、`datetime_original`、`pixel_width`、`pixel_height`。

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
| `imgproc::claheEqualize(src, clip, tile)` | CLAHE 局部直方图均衡，改善局部对比度并限制噪声放大 | `auto enhanced = imgproc::claheEqualize(src, 2.0, 8);` |
| `imgproc::adjustBrightnessContrast(src, alpha, beta)` | 线性亮度/对比度调整：`dst = alpha * src + beta` | `auto bright = imgproc::adjustBrightnessContrast(src, 1.2, 10);` |
| `imgproc::gammaCorrection(src, gamma)` | Gamma 校正 | `auto corrected = imgproc::gammaCorrection(src, 1.8);` |
| `imgproc::grayWorldWhiteBalance(src)` | 灰度世界白平衡，纠正简单色偏 | `auto wb = imgproc::grayWorldWhiteBalance(src);` |
| `imgproc::adjustSaturation(src, factor)` | 调整整体色彩强度 | `auto sat = imgproc::adjustSaturation(src, 1.2);` |
| `imgproc::adjustVibrance(src, amount)` | 鲜明度调整，优先提升低饱和区域并保护高饱和区域 | `auto vib = imgproc::adjustVibrance(src, 0.35);` |
| `imgproc::adjustTemperatureTint(src, temperature, tint)` | 基于 Lab 色彩空间调整冷暖和偏色 | `auto warm = imgproc::adjustTemperatureTint(src, 0.15, 0.02);` |
| `imgproc::applyToneCurve(src, points, channel)` | 按曲线 LUT 调整全通道或单独 R/G/B 通道 | `auto curved = imgproc::applyToneCurve(src, {{0,0},{128,140},{255,255}});` |
| `imgproc::applyPresetFilter(src, preset, intensity)` | 套用成熟场景滤镜预设 | `auto look = imgproc::applyPresetFilter(src, "portrait", 0.85);` |
| `imgproc::autoEnhance(src, strength)` | 根据亮度、局部直方图和饱和度统计自动修图 | `auto autoFixed = imgproc::autoEnhance(src);` |
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
| `imgproc::denoiseNlMeans(src, h, template, search)` | 非局部均值去噪，适合需要保留细节的照片 | `auto clean = imgproc::denoiseNlMeans(src, 10);` |
| `imgproc::morphologyImage(src, type, ksize, iterations)` | 形态学操作 | `auto dst = imgproc::morphologyImage(src, imgproc::MorphType::Close, 5);` |
| `imgproc::parseMorphType(name)` | 将字符串转换为 `MorphType` | `auto type = imgproc::parseMorphType("open");` |

### 边缘、锐化与噪声

| 函数 | 作用 | 示例 |
| --- | --- | --- |
| `imgproc::cannyEdge(src, low, high, aperture)` | Canny 边缘检测 | `auto edge = imgproc::cannyEdge(src, 60, 150);` |
| `imgproc::autoCannyEdge(src, sigma, aperture)` | 基于灰度中位数自动估计 Canny 阈值 | `auto edge = imgproc::autoCannyEdge(src);` |
| `imgproc::sobelEdge(src, dx, dy, ksize)` | Sobel 梯度边缘 | `auto gx = imgproc::sobelEdge(src, 1, 0);` |
| `imgproc::laplacianEdge(src, ksize)` | Laplacian 边缘 | `auto edge = imgproc::laplacianEdge(src);` |
| `imgproc::gradientMagnitude(src, ksize, useScharr)` | 梯度幅值图，可选 Scharr 高精度算子 | `auto grad = imgproc::gradientMagnitude(src, 3, true);` |
| `imgproc::contoursImage(src, threshold, useOtsu, thickness)` | 提取并绘制外轮廓 | `auto view = imgproc::contoursImage(src);` |
| `imgproc::connectedComponentsImage(src, minArea)` | 连通域标记并用颜色可视化 | `auto cc = imgproc::connectedComponentsImage(src, 50);` |
| `imgproc::distanceTransformImage(src)` | 距离变换图 | `auto dist = imgproc::distanceTransformImage(src);` |
| `imgproc::watershedSegmentation(src)` | 基于距离变换的分水岭分割 | `auto seg = imgproc::watershedSegmentation(src);` |
| `imgproc::houghLinesImage(src, rho, theta, threshold, minLen, maxGap)` | 概率 Hough 直线检测并绘制 | `auto lines = imgproc::houghLinesImage(src);` |
| `imgproc::houghCirclesImage(src, dp, minDist, p1, p2, minR, maxR)` | Hough 圆检测并绘制 | `auto circles = imgproc::houghCirclesImage(src);` |
| `imgproc::templateMatchImage(src, templ, method)` | 模板匹配并标出最佳位置 | `auto matched = imgproc::templateMatchImage(src, patch);` |
| `imgproc::drawOrbKeypoints(src, maxFeatures)` | ORB 特征点检测与可视化 | `auto kps = imgproc::drawOrbKeypoints(src, 800);` |
| `imgproc::inpaintImage(src, mask, radius)` | Telea 图像修复 | `auto restored = imgproc::inpaintImage(src, mask, 3);` |
| `imgproc::pyramidDown(src, levels)` | 高斯金字塔下采样 | `auto small = imgproc::pyramidDown(src, 2);` |
| `imgproc::pyramidUp(src, levels)` | 高斯金字塔上采样 | `auto large = imgproc::pyramidUp(src, 1);` |
| `imgproc::affineTransform(src, a00, a01, a02, a10, a11, a12)` | 自定义仿射矩阵变换 | `auto moved = imgproc::affineTransform(src, 1,0,20, 0,1,10);` |
| `imgproc::sharpen(src)` | 3x3 卷积锐化 | `auto sharp = imgproc::sharpen(src);` |
| `imgproc::unsharpMask(src, amount, ksize, sigma)` | 反遮罩锐化 | `auto sharp = imgproc::unsharpMask(src, 1.4);` |
| `imgproc::addSaltPepperNoise(src, amount, seed)` | 添加椒盐噪声，`amount` 范围为 `[0,1]` | `auto noisy = imgproc::addSaltPepperNoise(src, 0.02, 42);` |
| `imgproc::addGaussianNoise(src, mean, stddev, seed)` | 添加高斯噪声 | `auto noisy = imgproc::addGaussianNoise(src, 0, 12, 42);` |
| `imgproc::applyColorMapImage(src, colorMap)` | 伪彩色映射 | `auto color = imgproc::applyColorMapImage(src, cv::COLORMAP_TURBO);` |
| `imgproc::parseColorMap(name)` | 将字符串转换为 OpenCV colormap | `auto map = imgproc::parseColorMap("viridis");` |
| `imgproc::parseTemplateMatchMethod(name)` | 将字符串转换为 OpenCV 模板匹配方法 | `auto m = imgproc::parseTemplateMatchMethod("ccoeff_normed");` |
| `imgproc::readImageMetadata(path)` | 读取 JPEG EXIF/PNG 基础元数据 | `auto meta = imgproc::readImageMetadata("photo.jpg");` |
| `imgproc::availablePresetFilters()` | 返回内置滤镜名称列表 | `auto filters = imgproc::availablePresetFilters();` |

## 高性能与准确性建议

- 优先使用 `clahe` 而不是普通均衡化处理光照不均的医学图像、文档图像和低照度图像，`clip` 用于限制噪声放大。
- 去噪优先级：椒盐噪声用 `median`；一般照片噪声用 `nlm_denoise`；需要保留边缘时用 `bilateral`。
- 边缘检测优先使用 `auto_canny` 快速估计阈值，再根据结果微调 `low/high`。
- 小目标分割可先 `clahe -> gaussian_blur -> otsu/adaptive -> morphology -> components/contours`。
- 模板匹配适合刚性目标或 UI/文档定位；旋转、尺度变化明显时建议使用 `orb` 特征。
- 手机照片修图优先尝试 `auto_enhance`，再叠加小强度 `filter`；避免多次大幅度曲线调整导致高光或阴影细节被压扁。
- 读取拍摄参数时使用 `metadata`，它只扫描文件头和 EXIF 结构，不需要解码整张图片，适合相册列表或详情页快速展示。
- 所有新增高级函数尽量调用 OpenCV 优化算子，避免手写逐像素慢循环；输入参数会做边界检查，减少无效结果和崩溃。

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
