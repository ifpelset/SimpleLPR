#include <QMessageBox>

#include "PlateLocation.h"

Sta sta1[IMAGE_WIDTH *IMAGE_HEIGHT];

int sp = 0;//堆栈指针

void push(int x, int y)
{
    sta1[sp].x = x;
    sta1[sp].y = y;
    sp++;
}

void  pop()
{
    sp--;
}

PlateLocation::PlateLocation()
{

}

IplImage * PlateLocation::locatePlate(const QString &srcImagePath)
{
    IplImage *pImgSrc = NULL;
    IplImage *pImg8u = NULL;            //灰度图
    IplImage *pImg8uSmooth = NULL;     //高斯滤波后的图
    IplImage *pImgCanny = NULL;        //边缘检测
    IplImage *pImgCanny1 = NULL;
    IplImage *pImgCanny2 = NULL;
    IplImage *mycopy = NULL;          //标记图像
    IplImage *pImgdst = NULL;     //形态学操作后图像
    IplImage *pImgResize = NULL; //粗定位车牌图像
    IplImage *pImgResize1 = NULL; //精确定位车牌图像
    IplImage *pImgdst1 = NULL;//去除上下边框
    IplImage *pImgThr = NULL;//二值化车牌

    pImgSrc = cvLoadImage(srcImagePath.toLatin1().data(), -1);
    if (!pImgSrc) {
        QMessageBox::warning(NULL, "不能打开图片", "不能打开图片，请检查正确的路径");
        return NULL;
    }

    pImg8u = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    pImg8uSmooth = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    pImgCanny = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    pImgCanny1 = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    pImgCanny2 = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    pImgdst = cvCreateImage(cvGetSize(pImgSrc), IPL_DEPTH_8U, 1);
    cvCvtColor(pImgSrc, pImg8u, CV_RGB2GRAY);
    cvSmooth(pImg8u, pImg8uSmooth, CV_GAUSSIAN, 3, 0, 0);//高斯滤波
    cvCanny(pImg8uSmooth, pImgCanny, 100, 200, 3);    //边缘检测

    int  i, j, k, g, sum = 0;
    pImgCanny1 = cvCloneImage(pImgCanny);

    /* 边缘密度法处理 */
    //横向扫描处理
    for (j = 0; j<pImgCanny->height; j++) {
        sum = 0;
        for (i = 0; i<pImgCanny->width; i++) {
            if (S(pImgCanny, i, j) == 255)
                sum += S(pImgCanny, i, j);
        }
        // 横向 像素为255的点个数少于20个，则将该横向的所有像素值设置为0（即黑色处理）
        if (sum<ROW_THR * 255)
            for (i = 0; i<pImgCanny->width; i++)
                S(pImgCanny1, i, j) = 0;
    }
    //纵向扫描处理
    for (i = 0; i<pImgCanny->width; i++) {
        sum = 0;
        for (j = 0; j<pImgCanny->height; j++) {
            if (S(pImgCanny, i, j) == 255)
                sum += S(pImgCanny, i, j);
        }
        if (sum<COL_THR * 255)
            for (j = 0; j<pImgCanny->height; j++)
                S(pImgCanny1, i, j) = 0;
    }

    pImgCanny2 = cvCloneImage(pImgCanny1);

    // 移动模版，计算模版内点的密度
    for (j = 0; j<pImgCanny1->height - MODLESIZE; j += 2) {
        for (i = 0; i<pImgCanny1->width - MODLESIZE; i += 2) {
            sum = 0;
            // 此模版为3，3
            for (g = j; g <= j + MODLESIZE; g++)
                for (k = i; k <= i + MODLESIZE; k++) {
                    sum += S(pImgCanny1, k, g);
                }
            //大于某阈值置为255，否则置为0
            if (sum >= COUNTER * 255) {
                for (g = j; g <= j + MODLESIZE; g++)
                    for (k = i; k <= i + MODLESIZE; k++)
                        S(pImgCanny2, k, g) = 255;
            } else {
                for (g = j; g <= j + MODLESIZE; g++)
                    for (k = i; k <= i + MODLESIZE; k++)
                        S(pImgCanny2, k, g) = 0;
            }
        }
    }

    pImgdst = cvCloneImage(pImgCanny2);
    // 对图像进行膨胀操作
    cvDilate(pImgdst, pImgdst, 0, 4);

    /* 种子法标定单连通区域 */
    int	count = 0;//记录可能的车牌区域个数
    int postion[10][4] = { {0} };

    CvRect ROI_rect[NUM];//车牌候选区矩形存储数组

    mycopy = cvCreateImage(cvGetSize(pImgdst), IPL_DEPTH_8U, 1);
    mycopy = cvCloneImage(pImgdst);

    count = 0;
    for (i = 0; i<mycopy->width; i++)
        for (j = 0; j<mycopy->height; j++) {
            if (S(mycopy, i, j)>250) {
                myScope(mycopy, i, j, postion[count]); // 找到该矩形区域x1,y1 x2,y2
                ROI_rect[count].x = postion[count][0];
                ROI_rect[count].y = postion[count][2];
                ROI_rect[count].width = postion[count][1] - postion[count][0];
                ROI_rect[count].height = postion[count][3] - postion[count][2];
                // 利用车牌特征去除干扰单连通区
                // 车牌的宽度定义在了100-260之间，并且宽高比-3.3在－0.6～0.6之间，即宽高比大约为3.9～3.3:1
                if (MAX_WIDTH>ROI_rect[count].width
                        && ROI_rect[count].width>MIN_WIDTH
                        && std::abs((float)ROI_rect[count].width / (float)ROI_rect[count].height - 3.3) < MYERROR)
                    count++;
            }
        }

    if (count == 0) {
        QMessageBox::warning(NULL, "警告", "识别失败,请选择其他图片");
        return NULL;
    }

    IplImage *pImg8uROI = NULL;         //感兴趣的图片
//    cvSetImageROI(pImg8u, ROI_rect[0]);
//    pImg8uROI = cvCreateImage(cvSize(ROI_rect[0].width, ROI_rect[0].height), IPL_DEPTH_8U, 1);
    cvSetImageROI(pImg8u, ROI_rect[count-1]);
    pImg8uROI = cvCreateImage(cvSize(ROI_rect[count-1].width, ROI_rect[count-1].height), IPL_DEPTH_8U, 1);
    cvCopy(pImg8u, pImg8uROI);
    cvResetImageROI(pImg8u);

    /* 车牌精确定位 */
    int nWidth = 409;//(409,90)分别为感兴趣图像的宽度与高度
    int nHeight = 90;

    pImgResize = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
    cvResize(pImg8uROI, pImgResize, CV_INTER_LINEAR); //线性插值

    pImgResize1 = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);//精确定位部分
    myDiffProj(pImgResize, pImgResize1);

    int post[2] = { 0 };//上下边框位置数组

    myRemoveBorder(pImgResize1, post);

    CvRect ROI_rect1;                 //获得图片感兴趣区域
    ROI_rect1.x = 0;
    ROI_rect1.y = post[0];
    ROI_rect1.width = pImgResize1->width;
    ROI_rect1.height = post[1] - post[0];

    pImgdst1 = cvCreateImage(cvSize(ROI_rect1.width, ROI_rect1.height), IPL_DEPTH_8U, 1);
    cvSetImageROI(pImgResize1, ROI_rect1);
    cvCopy(pImgResize1, pImgdst1);
    cvResetImageROI(pImgResize1);

    pImg8uSmooth = cvCreateImage(cvGetSize(pImgdst1), IPL_DEPTH_8U, 1);
    pImgThr = cvCreateImage(cvGetSize(pImgdst1), IPL_DEPTH_8U, 1);
    cvSmooth(pImgdst1, pImg8uSmooth, CV_GAUSSIAN, 3, 0, 0);//高斯滤波

    int Thres = myGetThreshold(pImg8uSmooth);
    cvThreshold(pImg8uSmooth, pImgThr, Thres, 255, CV_THRESH_BINARY);

    //销毁无用图像
    cvReleaseImage(&pImgSrc);
    cvReleaseImage(&pImg8u);
    cvReleaseImage(&pImg8uROI);
    cvReleaseImage(&pImg8uSmooth);
    cvReleaseImage(&pImgCanny);
    cvReleaseImage(&pImgCanny1);
    cvReleaseImage(&pImgCanny2);
    cvReleaseImage(&mycopy);
    cvReleaseImage(&pImgdst);
    cvReleaseImage(&pImgResize);
    cvReleaseImage(&pImgResize1);
    cvReleaseImage(&pImgdst1);

    return pImgThr;
}

int PlateLocation::myDiffProj(IplImage *src, IplImage *dst)
{
    int i, j, r_max, r_max_value;//r_max为最大峰值所在行,r_max_value最大峰值的值
    int l_max, l_max_value;//l_max为最大峰值所在行,l_max_value最大峰值的值
    int r_sum[1600] = { 0 }, l_sum[1600] = { 0 }, sum = 0;//r_sum[i]为i行差分值之和,l_sum[i]为i列差分值之和,sum累加单元

    IplImage *pImg8uSmooth = NULL;       //高斯滤波后的图

    if (!src) {
        QMessageBox::warning(NULL, "警告", "不能载入源图像");
        return -1;
    } else {
        pImg8uSmooth = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
        cvSmooth(src, pImg8uSmooth, CV_GAUSSIAN, 3, 0, 0);//高斯滤波

        // 水平投影
        // 水平方向一阶差分
        for (j = 0; j<pImg8uSmooth->height; j++) {
            sum = 0;
            for (i = 0; i<pImg8uSmooth->width - 2; i++) {
                sum += abs((S(pImg8uSmooth, i, j) - S(pImg8uSmooth, i + 1, j)));//差分，累加
            }
            r_sum[j] = sum;// 保存累加结果
        }
        for (j = 1; j<pImg8uSmooth->height - 2; j++) {
            r_sum[j] = (int)((r_sum[j - 1] + r_sum[j] + r_sum[j + 1]) / 3);//平滑
        }

        r_max_value = r_sum[0];//寻找最大峰值
        for (i = 0; i<pImg8uSmooth->height; i++)
        if (r_max_value<r_sum[i]) {
           r_max_value = r_sum[i];
           r_max = i;
        }

        //row_min最大峰值两侧波谷的较小值所在行,row_min1=0峰值左侧波谷,row_min2=0峰值右侧波谷
        int row_start = -1, row_end = -1, row_min = 0, row_min1 = 0, row_min2 = 0;
        int row_min_value = 0;//最大峰值两侧波谷的较小值

        for (i = r_max; i>1; i--) {
            if (r_sum[i]<r_sum[i - 1] && r_sum[i]<0.6*r_max_value) {
                row_min1 = i;
                break;
            }//搜索峰值两侧的波谷

        }
        for (i = r_max; i<pImg8uSmooth->height - 1; i++) {
            if (r_sum[i]<r_sum[i + 1] && r_sum[i]<0.6*r_max_value) {
                row_min2 = i;
                break;
            }
        }
        //波谷的较小值
        if (r_sum[row_min1]<r_sum[row_min2]) {
            row_min_value = r_sum[row_min1];
            row_min = row_min1;
        } else {
            row_min_value = r_sum[row_min2];
            row_min = row_min2;
        }

        row_start = row_min;
        row_end = row_min;

        if (row_min > r_max) {
            for (i = r_max; i>1; i--)
            if (r_sum[i]<row_min_value&&r_sum[i]<0.6*r_max_value) {
                row_start = i;
                break;
            }
        }

        // 没有找到满足条件的起始行
        if (i == 1) {
            row_start = 0;
        }

        if (row_min < r_max) {
            for (i = r_max; i<pImg8uSmooth->height - 1; i++)
            if (r_sum[i]<row_min_value&&r_sum[i]<0.6*r_max_value) {
                row_end = i;
                break;
            }
        }

        // 没有找到满足条件的结束行
        if (i == pImg8uSmooth->height - 1) {
            row_end = pImg8uSmooth->height;
        }

        int col_start = -1, col_end = -1;

        //垂直投影
        for (i = 0; i<pImg8uSmooth->width; i++) {
            sum = 0;
            for (j = 0; j<pImg8uSmooth->height - 1; j++) {
                sum = abs((S(pImg8uSmooth, i, j) - S(pImg8uSmooth, i, j + 1))) + sum;//差分
            }
            l_sum[i] = sum;
        }
        for (i = 1; i<pImg8uSmooth->width - 2; i++) {
            l_sum[i] = (int)((l_sum[i - 1] + l_sum[i] + l_sum[i + 1]) / 3);//平滑
        }

        l_max_value = l_sum[0];//寻找最大峰值
        for (j = 0; j<pImg8uSmooth->width - 1; j++)
        if (l_max_value<l_sum[j]) {
            l_max_value = l_sum[j];
            l_max = j;
        }

        int flag = 1, rig_max = 0, last_max = 0;
        for (j = 1; j<pImg8uSmooth->width - 1; j++)//搜索满足条件的起始和结束峰值
        if (l_sum[j] >= l_sum[j - 1] && l_sum[j]>l_sum[j + 1] && l_sum[j]>0.5*l_max_value) {

            if (flag) {
                rig_max = j;
                flag = 0;
            }
            else
                last_max = j;
        }

        for (j = rig_max; j>1; j--) {
            if (l_sum[j]<l_sum[j - 1]) {
                col_start = j;
                break;
            }//起始列，结束列
        }

        // 没有找到满足条件的起始列
        if (j == 1) {
            col_start = 0;
        }

        for (j = last_max; j<pImg8uSmooth->width - 1; j++) {
            if (l_sum[j]<l_sum[j + 1]) {
                col_end = j;
                break;
            }
        }

        // 没有找到满足条件的结束列
        if (j == pImg8uSmooth->width - 1) {
            col_end = pImg8uSmooth->width;
        }

        CvRect ROI_rect;                 //获得图片感兴趣区域
        ROI_rect.x = col_start;
        ROI_rect.y = row_start;
        ROI_rect.width = col_end - col_start;
        ROI_rect.height = row_end - row_start;
        IplImage *pImg8uROI = NULL;         //感兴趣的图片

        cvSetImageROI(pImg8uSmooth, ROI_rect);
        pImg8uROI = cvCreateImage(cvSize(ROI_rect.width, ROI_rect.height), IPL_DEPTH_8U, 1);
        cvCopy(pImg8uSmooth, pImg8uROI);
        cvResetImageROI(pImg8uSmooth);

        cvResize(pImg8uROI, dst, CV_INTER_LINEAR); //线性插值

        return 0;
    }

    return -1;
}

void PlateLocation::myScope(IplImage *scopecopy, int x, int y, int *pionter)
{
    S(scopecopy, x, y) = FLAG;//标记
    pionter[0] = x;
    pionter[1] = x;
    pionter[2] = y;
    pionter[3] = y;
    push(x, y);
    while (sp) {
        pop();
        x = sta1[sp].x;
        y = sta1[sp].y;
        if ((x - 1)>0) // 负X方向搜索
            if (S(scopecopy, x - 1, y)>250) {
                push(x - 1, y);
                S(scopecopy, x - 1, y) = FLAG;//标记
                if (pionter[0]>x - 1)
                    pionter[0] = x - 1;
            }

        if ((x + 1)<scopecopy->width) // 正X方向搜索
            if (S(scopecopy, x + 1, y)>250) {
                push(x + 1, y);
                S(scopecopy, x + 1, y) = FLAG;//标记
                if (pionter[1]<x + 1)
                    pionter[1] = x + 1;
            }

        if ((y - 1)>0) // 正Y方向搜索
            if (S(scopecopy, x, y - 1)>250) {
                push(x, y - 1);
                S(scopecopy, x, y - 1) = FLAG;//标记
                if (pionter[2]>y - 1)
                    pionter[2] = y - 1;
            }

        if ((y + 1)<scopecopy->height) //负Y方向搜索
            if (S(scopecopy, x, y + 1)>250) {
                push(x, y + 1);
                S(scopecopy, x, y + 1) = FLAG;//标记
                if (pionter[3]<y + 1)
                    pionter[3] = y + 1;
            }
    }
}

int PlateLocation::myRemoveBorder(IplImage *src, int *post)
{
    int i, j, sum = 0;
    int r_sum[1000] = { 0 };//, l_sum[1000] = { 0 };

    IplImage *pImg8uSmooth;
    pImg8uSmooth = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
    cvSmooth(src, pImg8uSmooth, CV_GAUSSIAN, 3, 0, 0);//高斯滤波

    //水平投影
    for (j = 0; j<pImg8uSmooth->height; j++) {
        sum = 0;
        for (i = 0; i<pImg8uSmooth->width - 2; i++) {
            sum = abs((S(pImg8uSmooth, i, j) - S(pImg8uSmooth, i + 1, j))) + sum;//差分
        }
        r_sum[j] = sum;
    }
    for (j = 1; j<pImg8uSmooth->height - 2; j++) {
        r_sum[j] = (int)((r_sum[j - 1] + r_sum[j] + r_sum[j + 1]) / 3);//平滑
    }

    int r_max_value = r_sum[0], r_max = 0;//寻找最大峰值r_max_value,寻找最大峰值所在行r_max
    for (j = 0; j<pImg8uSmooth->height - 1; j++)
    if (r_max_value<r_sum[j]){ r_max_value = r_sum[j]; r_max = j; }

    int flag = 1, rig_max = 0, last_max = 0, row_start = 0, row_end = 0;
    for (j = 1; j<pImg8uSmooth->height - 1; j++)//搜索满足条件的起始和结束峰值
    if (r_sum[j] >= r_sum[j - 1] && r_sum[j]>r_sum[j + 1]) {

        if (flag) {
            rig_max = j; flag = 0;
        }
        else
            last_max = j;
    }

    for (j = rig_max; j<pImg8uSmooth->height - 3; j++) {
        if (r_sum[j]<r_sum[j + 1] && r_sum[j + 1]<r_sum[j + 2] && r_sum[j])
        {
            row_start = j;
            break;
        }//起始行
    }
    if (r_sum[row_start]>0.5*r_max_value || j == pImg8uSmooth->height - 3) {
        row_start = 0;
    }//满足条件起始峰下侧没有到波谷
    for (j = last_max; j>3; j--) {
        if (r_sum[j]<r_sum[j - 1] && r_sum[j - 1]<r_sum[j - 2] && r_sum[j]){ row_end = j; break; }//结束行

    }
    if (r_sum[row_end]>0.5*r_max_value || j == 3) {
        row_end = pImg8uSmooth->height - 1;
    }//满足条件结束峰上侧没有到波谷

    post[0] = row_start;
    post[1] = row_end;

    return 0;
}

int PlateLocation::myGetThreshold(IplImage *src)
{
    IplImage *pImg8uSmooth = NULL;       //高斯滤波后的图

    pImg8uSmooth = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
    cvSmooth(src, pImg8uSmooth, CV_GAUSSIAN, 3, 0, 0);//高斯滤波
    int size = 125, i = 0;
    int ranges[2] = { 0, 255 };
    int THRESHOLD = 0;
    myHist hist;
    myCalHist(&hist, pImg8uSmooth, size, ranges);

    for (i = hist.bin_max; i<hist.size - 4; i++) {
        if (hist.bin[i]<hist.bin[i + 1] && hist.bin[i + 1]<hist.bin[i + 2] && hist.bin[i + 2]<hist.bin[i + 3])
            break;
    }

    if (i<hist.size - 4)
        THRESHOLD = (hist.ranges[1] - hist.ranges[0]) / hist.size*i;
    else
        THRESHOLD = (hist.ranges[1] - hist.ranges[0]) / hist.size*hist.bin_max;

    return THRESHOLD;

}

void PlateLocation::myCalHist(myHist *histponter, IplImage * src, int size, int *ranges)//
{
    int i = 0, j = 0, k = 0, counter = 0;//计数器，记录某一范围中像素的个数

    int bin_wid = (ranges[1] - ranges[0]) / size;//每个所表示的直方块像素范围

    while (k<size) {
        counter = 0;
        histponter->bin[k] = 0;
        for (i = 0; i<src->width; i++)
        for (j = 0; j<src->height; j++) {
            if (S(src, i, j)>k*bin_wid&&S(src, i, j)<(k + 1)*bin_wid)
                counter++;
        }
        histponter->bin[k++] = counter;
    }

    histponter->size = size;
    histponter->ranges[0] = ranges[0];
    histponter->ranges[1] = ranges[1];

    histponter->max_value = histponter->bin[0];
    histponter->bin_max = 0;
    for (i = 0; i<k; i++) {
        if (histponter->bin[i]>histponter->max_value){ histponter->max_value = histponter->bin[i]; histponter->bin_max = i; }
    }

    histponter->min_value = histponter->bin[0];
    histponter->bin_min = 0;
    for (i = 0; i<k; i++) {
        if (histponter->bin[i]<histponter->min_value) {
            histponter->min_value = histponter->bin[i];
            histponter->bin_min = i;
        }
    }
}


