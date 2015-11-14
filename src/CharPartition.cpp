#include <QMessageBox>

#include "CharPartition.h"


CharPartition::CharPartition()
{

}

IplImage **CharPartition::partChar(IplImage *srcImage)
{
    IplImage *pImgSrc = NULL;
    IplImage *pImgThreshold = NULL;
    IplImage *pImgResize = NULL;        //归一化为高90，宽409的图像

    pImgSrc = srcImage;
    if (!pImgSrc) {
        QMessageBox::warning(NULL, "警告", "你的车牌定位的源图片有问题");
        return NULL;
    }

    int nWidth = 409, nHeight = 90;

    pImgThreshold = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
    pImgResize = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);//归一化

    // 其实就是将图像缩放成409*90
    cvResize(pImgSrc, pImgResize, CV_INTER_LINEAR); //线性插值
    cvThreshold(pImgResize, pImgThreshold, 100, 255, CV_THRESH_BINARY);

    // WIDTH 34 分隔符区域宽度
    // 409 整个车牌的宽度
    int l_sum[409] = { 0 }, modle[WIDTH] = { 0 };
    int sum = 0, i = 0, j = 0;
    int begin = 0, end = 0;//分隔符开始与结束位置

    // 3 模板的 (3, 3, 15, 15 ... 15, 15, 3, 3)
    modle[0] = MOD_MIN * 255;
    modle[1] = MOD_MIN * 255;
    modle[WIDTH - 1] = MOD_MIN * 255;
    modle[WIDTH - 2] = MOD_MIN * 255;

    for (i = 2; i<WIDTH - 2; i++)
        modle[i] = MOD_MAX * 255; // 模板除了首尾都是15

    for (i = 0; i<pImgThreshold->width; i++) {
        sum = 0;
        for (j = 0; j<pImgThreshold->height; j++) {
            sum = S(pImgThreshold, i, j) + sum;//灰度之和
        }
        l_sum[i] = sum;
    }

    // 查找分隔符区域
    bool ret = findSeparator(modle, l_sum, &begin, &end);
    if (!ret) {
        QMessageBox::warning(NULL, "警告", "识别失败，请选择其他图片");
        return NULL;
    }

    int min_value = 0, min_col = 0;//最小值及其所在列
    int oneright = 0, oneleft = 0, twoleft = 0, threeright = 0, fourleft = 0, fourright = 0, fiveleft = 0, fiveright = 0, sixleft = 0, sixright = 0, sevenleft = 0, sevenright = 0;

    i = begin - (int)((45 / 90.0)*pImgThreshold->height) - 1; // 第二个字符的左边界
    min_value = l_sum[i];
    min_col = i;

    // 在第一个字符与第二个字符之间的空隙去搜索
    for (; i >= begin - (int)((57 / 90.0)*pImgThreshold->height) - 1; i--) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }

    // 从最小值往右搜索第二个字符的左边界
    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i] > 3 * 255) {
            twoleft = i;
            break;
        }
    }

    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        twoleft = begin - (int)((51 / 90.0)*pImgThreshold->height);
    }

    // 从最小值往左搜索第一个字符的右边界
    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            oneright = i; break;
        }
    }

    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        oneright = begin - (int)((51 / 90.0)*pImgThreshold->height);
    }

    // 预留宽度5 用来搜索第一个字符左边界
    i = oneright - (int)((40 / 90.0)*pImgThreshold->height) - 1;
    min_value = l_sum[i];
    min_col = i;

    for (; i >= oneright - (int)((52 / 90.0)*pImgThreshold->height) - 1; i--) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }

    // 向右搜索第一个字符左边界
    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i]>3 * 255) {
            oneleft = i;
            break;
        }
    }

    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        oneleft = oneright - (int)((51 / 90.0)*pImgThreshold->height);
    }

    if (oneleft<0) {
        oneleft = 0;
    }

    i = end + (int)((45 / 90.0)*pImgThreshold->height) + 1;
    min_value = l_sum[i];
    min_col = i;

    for (; i <= end + (int)((57 / 90.0)*pImgThreshold->height); i++) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }

    // 搜索第三个字符右边界
    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            threeright = i;
            break;
        }
    }
    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        threeright = end + (51 / 90)*pImgThreshold->height;
    }

    // 第四个字符左边界
    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i]>3 * 255) {
            fourleft = i;
            break;
        }
    }
    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        fourleft = end + (51 / 90)*pImgThreshold->height;
    }

    i = fourleft + (int)((45 / 90.0)*pImgThreshold->height) + 1;
    min_value = l_sum[i];
    min_col = i;
    for (; i <= fourleft + (int)((57 / 90.0)*pImgThreshold->height); i++) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }
    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            fourright = i; break;
        }
    }
    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        fourright = fourleft + (int)((51 / 90.0)*pImgThreshold->height);
    }

    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i]>3 * 255) {
            fiveleft = i; break;
        }
    }
    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        fiveleft = fourleft + (int)((51 / 90.0)*pImgThreshold->height);
    }

    i = fiveleft + (int)((45 / 90.0)*pImgThreshold->height) + 1;
    min_value = l_sum[i];
    min_col = i;
    for (; i <= fiveleft + (int)((57 / 90.0)*pImgThreshold->height); i++) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }

    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            fiveright = i; break;
        }
    }
    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        fiveright = fiveleft + (int)((51 / 90.0)*pImgThreshold->height);
    }


    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i]>3 * 255) {
            sixleft = i;
            break;
        }
    }
    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        sixleft = fiveleft + (int)((51 / 90.0)*pImgThreshold->height);
    }

    i = sixleft + (int)((45 / 90.0)*pImgThreshold->height) + 1;
    min_value = l_sum[i];
    min_col = i;
    for (; i <= sixleft + (int)((57 / 90.0)*pImgThreshold->height); i++) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }
    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            sixright = i;
            break;
        }
    }
    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        sixright = sixleft + (int)((51 / 90.0)*pImgThreshold->height);
    }


    for (i = min_col; i<min_col + (int)((12 / 90.0)*pImgThreshold->height); i++) {
        if (l_sum[i]>3 * 255) {
            sevenleft = i; break;
        }
    }
    if (i >= min_col + (int)((12 / 90.0)*pImgThreshold->height)) {
        sevenleft = sixleft + (int)((51 / 90.0)*pImgThreshold->height);
    }
    i = sevenleft + (int)((45 / 90.0)*pImgThreshold->height) + 1;
    min_value = l_sum[i];
    min_col = i;
    for (; (i <= sevenleft + (int)((57 / 90.0)*pImgThreshold->height)) && i<pImgThreshold->width; i++) {
        if (min_value>l_sum[i]) {
            min_value = l_sum[i];
            min_col = i;
        }
    }
    for (i = min_col; i>min_col - (int)((12 / 90.0)*pImgThreshold->height); i--) {
        if (l_sum[i]>3 * 255) {
            sevenright = i;
            break;
        }
    }
    if (i <= min_col - (int)((12 / 90.0)*pImgThreshold->height)) {
        sevenright = sevenleft + (int)((51 / 90.0)*pImgThreshold->height);
    }
    if (sevenright>pImgThreshold->width - 1) {
        sevenright = pImgThreshold->width - 1;
    }

    cvLine(pImgThreshold, cvPoint(oneleft, 0), cvPoint(oneleft, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(oneright, 0), cvPoint(oneright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(twoleft, 0), cvPoint(twoleft, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(begin, 0), cvPoint(begin, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(end, 0), cvPoint(end, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(threeright, 0), cvPoint(threeright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(fourleft, 0), cvPoint(fourleft, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(fourright, 0), cvPoint(fourright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(fiveleft, 0), cvPoint(fiveleft, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(fiveright, 0), cvPoint(fiveright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(sixleft, 0), cvPoint(sixleft, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(sixright, 0), cvPoint(sixright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(sevenleft, 0), cvPoint(sevenleft, pImgThreshold->height), cvScalar(255, 0, 0), 1, 8, 0);
    cvLine(pImgThreshold, cvPoint(sevenright, 0), cvPoint(sevenright, pImgThreshold->height), cvScalar(188, 0, 0), 1, 8, 0);

    static IplImage *pImgChar[7] = {NULL};

    pImgChar[0] = cvCreateImage(cvSize(oneright - oneleft + 1, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[1] = cvCreateImage(cvSize(begin - twoleft + 2, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[2] = cvCreateImage(cvSize(threeright - end + 2, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[3] = cvCreateImage(cvSize(fourright - fourleft + 2, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[4] = cvCreateImage(cvSize(fiveright - fiveleft + 2, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[5] = cvCreateImage(cvSize(sixright - sixleft + 2, nHeight), IPL_DEPTH_8U, 1);
    pImgChar[6] = cvCreateImage(cvSize(sevenright - sevenleft + 1, nHeight), IPL_DEPTH_8U, 1);

    CvRect ROI_rect1;
    ROI_rect1.x = oneleft;
    ROI_rect1.y = 0;
    ROI_rect1.width = oneright - oneleft + 1;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[0], NULL); //获取第1个字符
    cvResetImageROI(pImgResize);

    ROI_rect1.x = twoleft - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = begin - twoleft + 2;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[1], NULL); //获取第2个字符
    cvResetImageROI(pImgResize);

    ROI_rect1.x = end - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = threeright - end + 2;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[2], NULL); //获取第3个字符
    cvResetImageROI(pImgResize);


    ROI_rect1.x = fourleft - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = fourright - fourleft + 2;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[3], NULL); //获取第4个字符
    cvResetImageROI(pImgResize);

    ROI_rect1.x = fiveleft - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = fiveright - fiveleft + 2;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[4], NULL); //获取第5个字符
    cvResetImageROI(pImgResize);

    ROI_rect1.x = sixleft - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = sixright - sixleft + 2;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[5], NULL); //获取第6个字符
    cvResetImageROI(pImgResize);

    ROI_rect1.x = sevenleft - 1;
    ROI_rect1.y = 0;
    ROI_rect1.width = sevenright - sevenleft + 1;
    ROI_rect1.height = pImgResize->height;
    cvSetImageROI(pImgResize, ROI_rect1);
    cvCopy(pImgResize, pImgChar[6], NULL); //获取第7个字符
    cvResetImageROI(pImgResize);

    // 释放资源
    cvReleaseImage(&pImgSrc);
    cvReleaseImage(&pImgThreshold);
    cvReleaseImage(&pImgResize);

    return pImgChar;
}

bool CharPartition::findSeparator(int templ[], int sum[], int *begin, int *end)
{
    int m = WIDTH; // 34  分隔符总的宽度是2×D+R=34 mm
    int i, j, g;
    int subre[WIDTH] = { 0 };

    // 分隔符区域搜索算法
    while (m > 7) { // m为分隔符宽度，为34
        // 57->181  扩大范围，设定搜索范围在第2个字符的起始位置到第3个字符的结束位置
        // templ模板为 (3, 3, 15, 15 ... 15, 15, 3, 3)
        // sum垂直投影，像素值的和
        for (i = START; i + m < END; i++) {
            for (j = 0, g = i; j < m; j++, g++) {
                subre[j] = templ[j] - sum[g];
            }

            j = 0;
            while (subre[j]>0 && j<m)
                j++;

            // 如果序列subre中的每个元素都大于0，则认为找到了分隔符区域
            if (j == m) {
                *begin = i;
                *end = *begin + m; // 分隔符区域为[L+i, L+i+m]
                return true;
            }
        }

        // 没有找到分隔符区域  缩小模板
        templ[m - 3] = templ[m - 1];
        templ[m - 4] = templ[m - 2];
        m--;
    }

    return false;
}

