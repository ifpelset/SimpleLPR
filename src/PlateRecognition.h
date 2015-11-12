#ifndef PLATERECOGNITION_H
#define PLATERECOGNITION_H

#include "def.h"

class PlateRecognition
{
public:
    PlateRecognition();
    static IplImage *locatePlate(const QString &srcImagePath);
    static IplImage **partChar(IplImage *srcImage);
    static QString recognizeChar(IplImage **srcIplImage);
};

#endif // PLATERECOGNITION_H
