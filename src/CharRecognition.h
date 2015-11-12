#ifndef CHARRECOGNITION_H
#define CHARRECOGNITION_H

#include "def.h"

class CharRecognition
{
public:
    CharRecognition();
    static QString recognizeChar(IplImage **srcIplImage);

private:
    static void myHorizontal(IplImage *src, int *r_sum);
    static void myVertical(IplImage *src, int *l_sum);
    static int mySub(int *r_poniter1, int *r_poniter2, int leng);
    static int myGetResult_R(IplImage *src1, IplImage *src2);
};

#endif // CHARRECOGNITION_H
