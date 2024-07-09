#ifndef ATOMX_HH
#define ATOMX_HH

#include "LKLogger.h"
#include "LKDetector.h"

class AToMX : public LKDetector
{
    public:
        AToMX();
        virtual ~AToMX() { ; }
        virtual bool BuildDetectorPlane();

    ClassDef(AToMX,1);
};

#endif
