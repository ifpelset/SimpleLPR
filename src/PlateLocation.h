#ifndef PLATELOCATION_H
#define PLATELOCATION_H

#include "def.h"

class PlateLocation
{
public:
    PlateLocation();
    static IplImage *locatePlate(const QString &srcImagePath);

private:
    static int myDiffProj(IplImage *src, IplImage *dst);
    static int myRemoveBorder(IplImage *src, int *post);
    static void myScope(IplImage *scopecopy, int x, int y, int *pionter);
    static int myGetThreshold(IplImage *src);
    static void myCalHist(myHist *histponter, IplImage * src, int size, int *ranges);
};

#endif // PLATELOCATION_H
