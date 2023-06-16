#ifndef JIANFENG_CORE_IMAGE_UTIL_H_
#define JIANFENG_CORE_IMAGE_UTIL_H_

#include <QPolygon>
#include <string>
#include <core/common.h>
#include <opencv2/opencv.hpp>

//读取图片，支持中文路径
cv::Mat readImgRGB(const std::string& file_path);
cv::Mat readImgGRAY(const std::string& file_path);

//写图片, format: ".png" ".jpg" ".bmp" etc
void writeImg(const cv::Mat& img, const std::string& format, const std::string& path);

//简单色阶调整
void levelAdjust(const cv::Mat& src, cv::Mat& dst, double g);
 //合并轮廓
 std::vector<std::vector<cv::Point>> mergeContours(std::vector<std::vector<cv::Point>> contours);
//提取玻璃边缘路径
//输入玻璃面拼合图像灰度图(原图)
QPolygon extractGlassEdgeOrg(const cv::Mat& img);

//提取玻璃边缘路径
//输入玻璃面拼合图像灰度图
QPolygon extractGlassEdge(const cv::Mat& img);
//Fred
QPolygon extractGlassEdgeF(const cv::Mat&img, int fred, int resize_sc, cv::Rect& rect_out);

//////////////////////////////////////////////////////////////////////////
// 计算flaw点是不是在rect矩形框内
bool flawContained(const cv::Rect rect_bounding, double flaw_x, double flaw_y);

//从边缘路径提取相机工位的扫描路径
QPolygon extractCamEdgePath(const QPolygon& path, CameraPosition position);

#endif //JIANFENG_CORE_IMAGE_UTIL_H_
