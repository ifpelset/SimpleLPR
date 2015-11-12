#ifndef CHARPARTITION_H
#define CHARPARTITION_H

#include "def.h"

class CharPartition
{
public:
    CharPartition();
    static IplImage **partChar(IplImage *srcImage);

private:
    static bool findSeparator(int modle[], int sum[], int *begin, int *end);
};

#endif // CHARPARTITION_H
