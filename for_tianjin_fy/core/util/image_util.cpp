#include "image_util.h"
#include <core/util/str_util.h>
#include <core/util/log.h>
#include <QFile>
#include <QByteArray>
#include <vector>

cv::Mat readImgRGB(const std::string& file_path)
{
  cv::Mat re;
  QFile file(QString::fromStdString(file_path));
  if(file.open(QIODevice::ReadOnly)) {
    QByteArray byteArray = file.readAll();
    std::vector<char> data(byteArray.data(), byteArray.data() + byteArray.size());
    re = cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
    file.close();
  }
  return re;
}

cv::Mat readImgGRAY(const std::string& file_path)
{
  cv::Mat re;
  QFile file(QString::fromStdString(file_path));
  if(file.open(QIODevice::ReadOnly)) {
    QByteArray byteArray = file.readAll();
    std::vector<char> data(byteArray.data(), byteArray.data() + byteArray.size());
    re = cv::imdecode(cv::Mat(data), cv::IMREAD_GRAYSCALE);
    file.close();
  }
  return re;
}

void writeImg(const cv::Mat& img, const std::string& format, const std::string& path)
{
  std::vector<unsigned char> buf;
  cv::imencode(format.c_str(), img, buf);  //把Image1的内容写入缓存buf

  QFile file(QString::fromStdString(path));
  if(file.open(QIODevice::WriteOnly)) {
    file.write((char*)&buf[0], buf.size());  //保存buf到文件
    file.close();
  }
}

void levelAdjust(const cv::Mat& src, cv::Mat& dst, double g)
{
  if(!src.empty()) {
    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    for(int i = 0; i < 256; ++i)
      p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, g) * 255.0);
    cv::LUT(src, lookUpTable, dst);
  }
}
std::vector<std::vector<cv::Point>> mergeContours(std::vector<std::vector<cv::Point>> contours) {

	std::vector<std::vector<cv::Point>> allcontours;
	std::vector<cv::Point> contourslist;

	for (int i = 0; i < contours.size(); i++) {
		std::vector<cv::Point> vec_i;
		vec_i = contours[i];
		contourslist.insert(contourslist.end(), vec_i.begin(), vec_i.end());
	}
	allcontours.emplace_back(contourslist);
	return allcontours;
}

QPolygon extractGlassEdgeOrg(const cv::Mat& img)
{
  QPolygon re;

  //1.高斯滤波
  cv::Mat f_img;
  //Fredrick
  //cv::rotate(img, f_img, cv::ROTATE_180);
  cv::GaussianBlur(img, f_img, cv::Size(17, 17), 2.324, 1);
  auto clahe = cv::createCLAHE(5.25, cv::Size(15, 15));
  clahe->apply(f_img, f_img);

  //2.边缘检测
  cv::Mat c_img;
  cv::Canny(f_img, c_img, 100, 50);

  //3.膨胀+闭操作
  cv::Mat img_pz;
  cv::Mat img_bcz;
  cv::Mat kernel = cv::Mat::ones(15, 15, CV_8U);
  cv::dilate(c_img, img_pz, kernel, cv::Point(-1, -1), 1);
  cv::morphologyEx(img_pz, img_bcz, cv::MORPH_CLOSE, kernel);

  //4.提取外轮廓
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(img_bcz, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

  if(contours.size() > 0) {
    //取点数最多的那个
    int that = 0;
    for(int i = 1; i < contours.size(); i++) {
      if(contours[i].size() > contours[that].size())
        that = i;
    }

    re.resize(contours[that].size());
    for(int i = 0; i < contours[that].size(); i++) {
      auto p = contours[that][i];
      re[i] = QPoint(p.x, p.y);
    }
  }

  return re;
}

QPolygon extractGlassEdgeF(const cv::Mat& img,int shiftVal,int resize_sc, cv::Rect& rect_out) // fred is shift value
{
	QPolygon re;
	double tm = cv::getTickCount();
	//std::cout << "1-----" << std::endl;

	if (img.empty()) return re;
	cv::Mat image;
	// rotate !!!!!!
	cv::rotate(img, image, cv::ROTATE_90_COUNTERCLOCKWISE);
	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

	cv::Mat res_img, blur_img, dest;
	//shift value then rebuild the image
	cv::Mat r = image(cv::Rect(0, shiftVal, image.cols / 2, image.rows - shiftVal));
	cv::Mat p = r.clone();
	r = image(cv::Rect(0, 0, image.cols / 2, image.rows - shiftVal));
	p.copyTo(r);
	//resize to reduce timecost
	//Gaussian enhance and adaptiveThreshold
	cv::resize(image, res_img, image.size() / resize_sc, 0, 0, cv::INTER_CUBIC);
	cv::GaussianBlur(res_img, blur_img, cv::Size(5, 5), 4);
	levelAdjust(res_img, blur_img, 11);

	cv::adaptiveThreshold(blur_img, dest, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 15, 35);
	//find contours then fill the contours and finally findcontours again
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(dest, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::Mat out = cv::Mat(dest.size(), dest.type(), cv::Scalar(0));

	for (size_t i = 0; i < contours.size(); i++)
	{
		cv::Scalar color = cv::Scalar(255, 0, 0);
		if (contours[i].size() > 1000)
		{
			cv::drawContours(out, contours, (int)i, color, cv::FILLED, 8, hierarchy, 0);
		}
	}

	cv::Mat out2;
	cv::resize(out, out2, out.size() * resize_sc);
	contours.clear(); hierarchy.clear();
	//rotate!!!!!
	cv::rotate(out2, out2, cv::ROTATE_90_CLOCKWISE);
	cv::findContours(out2, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	// 搜索中心线上的 x最大最小坐标
	//std::cout << "2-----" << std::endl;

	std::vector<int> mid_point;
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++)
		{
			if (contours[i][j].y == out2.rows / 2 || contours[i][j].y == out2.rows / 2 - 1)
			{
				mid_point.push_back(contours[i][j].x);
			}
		}
	}
	if (mid_point.size() > 0)
	{
		int max_x = mid_point.at(max_element(mid_point.begin(), mid_point.end()) - mid_point.begin());
		int min_x = mid_point.at(min_element(mid_point.begin(), mid_point.end()) - mid_point.begin());
		//std::cout << "3-----" << std::endl;
		cv::line(out2, cv::Point(min_x, out2.rows / 2), cv::Point(max_x, out2.rows / 2), cv::Scalar(255), 2);

	}


	contours.clear(); hierarchy.clear();
	cv::findContours(out2, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	// merge contours
	std::vector<std::vector<cv::Point>> merged_contours;
	{
		merged_contours = mergeContours(contours);
	}	
	//std::cout << "4-----" << std::endl;

	if(merged_contours.size() > 0) rect_out = cv::boundingRect(merged_contours[0]);
	cv::rectangle(out2, rect_out, cv::Scalar(255), 1);
	//cv::imwrite("E:\\Desktop\\1.jpg", out2);
	if (merged_contours.size() > 0)
	{
		re.resize(merged_contours[0].size());
		for (int i = 0; i < merged_contours[0].size(); i++)
		{
			auto p = merged_contours[0][i];
			re[i] = QPoint(p.x, p.y);
		}
	}
	//std::cout << "Time" << ((double)cv::getTickCount() - tm) / cv::getTickFrequency() * 1000 << std::endl;
	p.release();
	out.release();
	return re;
}

QPolygon extractGlassEdge(const cv::Mat& img)
{
  QPolygon re;

  //1.高斯滤波
  cv::Mat f_img;
  cv::Mat f_kernel = (cv::Mat_<char>(3, 3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
  //cv::GaussianBlur(img, f_img, cv::Size(3, 3), 1);
  cv::filter2D(img, f_img, CV_16S, f_kernel);

  //1.5Sobel滤波器
  cv::Mat s_img_x;
  cv::Mat s_img_y;
  cv::Mat s_img;
  cv::Sobel(f_img, s_img_x, CV_16S, 1, 0, 3);
  cv::Sobel(f_img, s_img_y, CV_16S, 0, 1, 3);
  cv::convertScaleAbs(s_img_x, s_img_x);
  cv::convertScaleAbs(s_img_y, s_img_y);
  cv::addWeighted(s_img_x, 0.4, s_img_y, 0.6, 0, s_img);

  //2.边缘检测
  cv::Mat c_img;
  cv::Canny(s_img, c_img, 400, 420);

  //3.膨胀+闭操作
  cv::Mat img_pz;
  cv::Mat img_bcz;
  cv::Mat kernel = cv::Mat::ones(21, 21, CV_8U);
  cv::Mat kernel2 = cv::Mat::ones(3, 3, CV_8U);
  //cv::dilate(c_img, img_pz, kernel, cv::Point(-1, -1), 1);
  cv::morphologyEx(c_img, img_pz, cv::MORPH_CLOSE, kernel);
  cv::erode(img_pz, img_bcz, kernel2);

  //4.提取外轮廓
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(img_bcz, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

  if(contours.size() > 0) {
    //取点数最多的那个
    int that = 0;
    for(int i = 1; i < contours.size(); i++) {
      if(contours[i].size() > contours[that].size())
        that = i;
    }

    re.resize(contours[that].size());
    for(int i = 0; i < contours[that].size(); i++) {
      auto p = contours[that][i];
      re[i] = QPoint(p.x, p.y);
    }
  }

  return re;
}

bool flawContained(const cv::Rect rect_bounding, double flaw_x, double flaw_y)
{
	//convert Rect to RotatedRect
	cv::Point2f corners[4];
	cv::Point2f center, flaw_point;
	cv::Size2f s;
	center.x = (float)(rect_bounding.tl().x + rect_bounding.br().x) / 2;
	center.y = (float)(rect_bounding.tl().y + rect_bounding.br().y) / 2;
	flaw_point.x = flaw_x;
	flaw_point.y = flaw_y;
	s.width = (float)rect_bounding.width;
	s.height = (float)rect_bounding.height;
	cv::RotatedRect r(center, s, 0);

	r.points(corners);
	cv::Point2f* lastItPointer = corners + sizeof(corners) / sizeof(corners[0]);
	std::vector<cv::Point2f> contours(corners, lastItPointer);
	if (pointPolygonTest(contours, flaw_point, false) >= 0)
		return true;
	else
		return false;
}

//顺时针
static QPolygon _getPartPathClockWise(const QPolygon& path, int start, int end)
{
  const int N = path.size();
  QPolygon re;

  if(N <= 0)
    return re;

  if(start >= 0 && start < N && end >= 0 && end < N) {
    for(int i = start; i != end; i = ((N + i--) % N)) {
      if(i < 0) i += N;
      if(i >= 0 && i < N)
        re.append(path.at(i));
    }
  }

  return re;
}

//逆时针
static QPolygon _getPartPathCounterClockWise(const QPolygon& path, int start, int end)
{
  const int N = path.size();
  QPolygon re;

  if(N <= 0)
    return re;

  if(start >= 0 && start < N && end >= 0 && end < N) {
    for(int i = start; i != end; i = ((i++) % N)) {
      if(i >= 0 && i < N)
        re.append(path.at(i));
    }
  }

  return re;
}

QPolygon extractCamEdgePath(const QPolygon& edge_path, CameraPosition position)
{
  if(edge_path.size() <= 20)
    return QPolygon();

  int TL_index = 0, TR_index = 0;   //上边左右
  int BL_index = 0, BR_index = 0;   //下边左右
  QRect bb = edge_path.boundingRect();
  double TLD = DBL_MAX, TRD = DBL_MAX;
  double BLD = DBL_MAX, BRD = DBL_MAX;

  //找到与包围矩形四个顶点最近的距离的四个点
  for(int i = 0; i < edge_path.size(); i++) {
    QPoint p = edge_path.at(i);
    {
      auto p2 = bb.topLeft();
      double d = sqrt((p.x() - p2.x()) * (p.x() - p2.x()) + (p.y() - p2.y()) * (p.y() - p2.y()));
      if(d < TLD) {
        TL_index = i;
        TLD = d;
      }
    }

    {
      auto p2 = bb.topRight();
      double d = sqrt((p.x() - p2.x()) * (p.x() - p2.x()) + (p.y() - p2.y()) * (p.y() - p2.y()));
      if(d < TRD) {
        TR_index = i;
        TRD = d;
      }
    }

    {
      auto p2 = bb.bottomLeft();
      double d = sqrt((p.x() - p2.x()) * (p.x() - p2.x()) + (p.y() - p2.y()) * (p.y() - p2.y()));
      if(d < BLD) {
        BL_index = i;
        BLD = d;
      }
    }

    {
      auto p2 = bb.bottomRight();
      double d = sqrt((p.x() - p2.x()) * (p.x() - p2.x()) + (p.y() - p2.y()) * (p.y() - p2.y()));
      if(d < BRD) {
        BR_index = i;
        BRD = d;
      }
    }
  }

  switch(position) {
    case EdgeLeft:
      return _getPartPathClockWise(edge_path, TL_index, TR_index); //顺时针取路径
    case EdgeRight:
      return _getPartPathCounterClockWise(edge_path, BL_index, BR_index); //逆时针取路径
    default:
      ;
  }

  return QPolygon();
}
