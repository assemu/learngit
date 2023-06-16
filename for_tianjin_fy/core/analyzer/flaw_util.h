#ifndef JIANFENG_CORE_FLAW_UTIL_H_
#define JIANFENG_CORE_FLAW_UTIL_H_

#include <string>
#include <core/common.h>
#include <opencv2/opencv.hpp>

cv::Mat genFlawDetailImg(const Flaw& flaw, bool drawMark = true);

//输入为BGR格式
//图片上绘制上缺陷标记
void drawMarkers(cv::Mat& img, std::vector<BoxInfo>& boxes);

#endif //JIANFENG_CORE_FLAW_UTIL_H_
