#include <QLabel>
#include <QMessageBox>

#include "PlateRecognition.h"

#include "PlateLocation.h"
#include "CharPartition.h"
#include "CharRecognition.h"

PlateRecognition::PlateRecognition()
{

}

IplImage * PlateRecognition::locatePlate(const QString &srcImagePath)
{
    return PlateLocation::locatePlate(srcImagePath);
}

IplImage **PlateRecognition::partChar(IplImage *srcImage)
{
    return CharPartition::partChar(srcImage);
}

QString PlateRecognition::recognizeChar(IplImage **srcIplImage)
{
    return CharRecognition::recognizeChar(srcIplImage);
}
