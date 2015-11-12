#ifndef DEF_H
#define DEF_H

// The old OpenCV header files.
//#include <cv.h>
//#include <cxcore.h>
//#include <highgui.h>

// The new OpenCV header files.
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <stdio.h>

// This application use the common Qt header files
#include <QMessageBox>
#include <QString>

using namespace cv;

// -- 车牌定位相关宏定义
//访问图像image像素（x,y）
#define S(image,x,y) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)]
#define COUNTER 5
#define ROW_THR 20
#define COL_THR 2
#define MODLESIZE 3

#define   FLAG 0
#define   IMAGE_WIDTH 600
#define   IMAGE_HEIGHT 600
#define   MAX_WIDTH 260
#define   MIN_WIDTH 100
#define   MYERROR 0.6

#define   NUM 100    //车牌候选区域最大个数

// --字符分割相关宏定义
#define  START  57
#define  END  181
#define  MOD_MAX 15
#define  MOD_MIN  3
#define  WIDTH  34 //模板宽度

// --字符识别相关定义
const int CHARS_LENGHT = 7;

typedef  struct Sta
{
    int x; /* X坐标, 通常以为基点*/
    int y; /* y坐标, 通常以为基点*/
}Sta;

typedef struct myHist//直方图机构体
{
    int size; // 直方块个数
    int ranges[2];//像素变化范围
    int bin_max;//最大高度直方块坐标
    int bin_min;//最小直高度直方块坐标
    int max_value;//最大高度直方块的值
    int min_value;//最小高度直方块的值
    int bin[256];//直方图各方块的像素个数
}myHist;


#endif // DEF_H

