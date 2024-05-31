#ifndef ATOMX_HH
#define ATOMX_HH

#include "LKLogger.h"
#include "LKATTPC.h"

class AToMX : public LKATTPC
{
    public:
        AToMX();
        virtual ~AToMX() { ; }
        virtual bool BuildDetectorPlane();

    ClassDef(AToMX,1);
};

#endif
