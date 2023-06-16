#include "flaw_util.h"
#include <core/util/str_util.h>
#include <core/util/log.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <locale.h>
#include <wchar.h>
#include <windows.h>

static void GetStringSize(HDC hDC, const char* str, int* w, int* h)
{
  SIZE size;
  GetTextExtentPoint32A(hDC, str, strlen(str), &size);
  if(w != 0) *w = size.cx;
  if(h != 0) *h = size.cy;
}

//解决中文问题
static void putTextZH(cv::Mat& dst,
                      const char* str,
                      cv::Point org,
                      cv::Scalar color,
                      int fontSize,
                      const char* fn,
                      bool italic,
                      bool underline)
{
  CV_Assert(dst.data != 0 && (dst.channels() == 1 || dst.channels() == 3));

  int x, y, r, b;
  if(org.x > dst.cols || org.y > dst.rows) return;
  x = org.x < 0 ? -org.x : 0;
  y = org.y < 0 ? -org.y : 0;

  LOGFONTA lf;
  lf.lfHeight = -fontSize;
  lf.lfWidth = 0;
  lf.lfEscapement = 0;
  lf.lfOrientation = 0;
  lf.lfWeight = 5;
  lf.lfItalic = italic;   //斜体
  lf.lfUnderline = underline; //下划线
  lf.lfStrikeOut = 0;
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfOutPrecision = 0;
  lf.lfClipPrecision = 0;
  lf.lfQuality = PROOF_QUALITY;
  lf.lfPitchAndFamily = 0;
  strcpy_s(lf.lfFaceName, fn);

  HFONT hf = CreateFontIndirectA(&lf);
  HDC hDC = CreateCompatibleDC(0);
  HFONT hOldFont = (HFONT)SelectObject(hDC, hf);

  int strBaseW = 0, strBaseH = 0;
  int singleRow = 0;
  char buf[1 << 12];
  strcpy_s(buf, str);
  char* bufT[1 << 12];  // 这个用于分隔字符串后剩余的字符，可能会超出。
  //处理多行
  {
    int nnh = 0;
    int cw, ch;

    const char* ln = strtok_s(buf, "\n", bufT);
    while(ln != 0) {
      GetStringSize(hDC, ln, &cw, &ch);
      strBaseW = std::max(strBaseW, cw);
      strBaseH = std::max(strBaseH, ch);

      ln = strtok_s(0, "\n", bufT);
      nnh++;
    }
    singleRow = strBaseH;
    strBaseH *= nnh;
  }

  if(org.x + strBaseW < 0 || org.y + strBaseH < 0) {
    SelectObject(hDC, hOldFont);
    DeleteObject(hf);
    DeleteObject(hDC);
    return;
  }

  r = org.x + strBaseW > dst.cols ? dst.cols - org.x - 1 : strBaseW - 1;
  b = org.y + strBaseH > dst.rows ? dst.rows - org.y - 1 : strBaseH - 1;
  org.x = org.x < 0 ? 0 : org.x;
  org.y = org.y < 0 ? 0 : org.y;

  BITMAPINFO bmp = { 0 };
  BITMAPINFOHEADER& bih = bmp.bmiHeader;
  int strDrawLineStep = strBaseW * 3 % 4 == 0 ? strBaseW * 3 : (strBaseW * 3 + 4 - ((strBaseW * 3) % 4));

  bih.biSize = sizeof(BITMAPINFOHEADER);
  bih.biWidth = strBaseW;
  bih.biHeight = strBaseH;
  bih.biPlanes = 1;
  bih.biBitCount = 24;
  bih.biCompression = BI_RGB;
  bih.biSizeImage = strBaseH * strDrawLineStep;
  bih.biClrUsed = 0;
  bih.biClrImportant = 0;

  void* pDibData = 0;
  HBITMAP hBmp = CreateDIBSection(hDC, &bmp, DIB_RGB_COLORS, &pDibData, 0, 0);

  CV_Assert(pDibData != 0);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hDC, hBmp);

  //color.val[2], color.val[1], color.val[0]
  SetTextColor(hDC, RGB(255, 255, 255));
  SetBkColor(hDC, 0);
  //SetStretchBltMode(hDC, COLORONCOLOR);

  strcpy_s(buf, str);
  const char* ln = strtok_s(buf, "\n", bufT);
  int outTextY = 0;
  while(ln != 0) {
    TextOutA(hDC, 0, outTextY, ln, strlen(ln));
    outTextY += singleRow;
    ln = strtok_s(0, "\n", bufT);
  }
  uchar* dstData = (uchar*)dst.data;
  int dstStep = dst.step / sizeof(dstData[0]);
  unsigned char* pImg = (unsigned char*)dst.data + org.x * dst.channels() + org.y * dstStep;
  unsigned char* pStr = (unsigned char*)pDibData + x * 3;
  for(int tty = y; tty <= b; ++tty) {
    unsigned char* subImg = pImg + (tty - y) * dstStep;
    unsigned char* subStr = pStr + (strBaseH - tty - 1) * strDrawLineStep;
    for(int ttx = x; ttx <= r; ++ttx) {
      for(int n = 0; n < dst.channels(); ++n) {
        double vtxt = subStr[n] / 255.0;
        int cvv = vtxt * color.val[n] + (1 - vtxt) * subImg[n];
        subImg[n] = cvv > 255 ? 255 : (cvv < 0 ? 0 : cvv);
      }

      subStr += 3;
      subImg += dst.channels();
    }
  }

  SelectObject(hDC, hOldBmp);
  SelectObject(hDC, hOldFont);
  DeleteObject(hf);
  DeleteObject(hBmp);
  DeleteDC(hDC);
}

//------------------------------------------------------
//------------------------------------------------------
//------------------------------------------------------

//GRB格式
static cv::Scalar getLabelDrawColor(const std::string& label)
{
  auto color = cv::Scalar(0, 255, 0);

  if(label == "white" || label == "black")
    color = cv::Scalar(255, 0, 0);
  else if(label == "bright" || label == "dark")
    color = cv::Scalar(255, 0, 0);
  else if(label == "scratch" || label == "dirty")
    color = cv::Scalar(255, 222, 0);
  else
    color = cv::Scalar(0, 255, 0); //circle wet water

  return color;
}

static cv::Point rectCenter(const cv::Rect& rect)
{
  cv::Point cpt;
  cpt.x = rect.x + cvRound(rect.width / 2.0);
  cpt.y = rect.y + cvRound(rect.height / 2.0);
  return cpt;
}

static std::string getLabelStr(const std::string& label)
{
  if(label == "white")
    return str_utf8_to_system("亮斑");
  else if(label == "black")
    return str_utf8_to_system("亮斑");
  else if(label == "bright")
    return str_utf8_to_system("崩边");
  else if(label == "dark")
    return str_utf8_to_system("亮斑");
  else if(label == "dirty")
    return str_utf8_to_system("脏污");
  else if(label == "scratch")
    return str_utf8_to_system("裂纹");
  else
    return label;
}

cv::Mat genFlawDetailImg(const Flaw& flaw, bool drawMark)
{
  auto img = flaw.part_img.img; //原图
  auto brect = flaw.rect;

  if(brect.empty())
    return cv::Mat();

  int H = (int)(1.5 * std::max<int>(brect.width, brect.height));
  H = std::max<int>(250, H);
  int W = (int)(1 * H);

  auto p = rectCenter(brect);
  auto r1 = cv::Rect(p.x - W / 2, p.y - H / 2, W, H);
  auto r2 = cv::Rect(0, 0, img.cols, img.rows);
  auto rect = r1 & r2;

  auto out_img = img(rect).clone();
  if(out_img.empty())
    return cv::Mat();

  if(drawMark) {
    auto color = getLabelDrawColor(flaw.label); //RGB
    auto c = color;
    c[0] = color[2];
    c[2] = color[0];

    cv::cvtColor(out_img, out_img, cv::COLOR_GRAY2BGR);
    rectangle(out_img,
              cv::Rect(brect.x - rect.x, brect.y - rect.y, brect.width*1.05, brect.height*1.05),
              c,
              2);
	// rotate flaw image
	//cv::rotate(out_img, out_img, cv::ROTATE_90_CLOCKWISE);

    char buffer[100];
    sprintf(buffer, "%s\n%s:%.2f\n[%.2f * %.2f]",
            cameraNameOf(flaw.part_img.position).c_str(),
            getLabelStr(flaw.label).c_str(),
            flaw.score,
            flaw.realWidth,
            flaw.realHeight);
    std::string label(buffer);
    putTextZH(out_img, label.c_str(), cv::Point(10, 10), c, 50.0 * 72 / 96 * H / 250, "宋体", false, false);
  }

  return out_img;
}

void drawMarkers(cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  for(auto& b : boxes) {
    auto c = getLabelDrawColor(b.label);
    rectangle(img,
              b.rect,
              c,
              2);
    char buffer[100];
    sprintf(buffer, "%s:%.2f",
            getLabelStr(b.label).c_str(),
            b.score);
    std::string label(buffer);
    //Fredrick *b.rect.width / 350 放大倍数
    putTextZH(img, label.c_str(), cv::Point(b.rect.x, b.rect.y - 40), c, 50.0 * 72 / 96 * b.rect.height / 250, "宋体", false, false);
  }
}
