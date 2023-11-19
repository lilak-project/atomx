#ifndef ATOMX_HH
#define ATOMX_HH

#include "LKDetector.h"
#include "LKLogger.h"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK detector class
 *
 */

class AToMX : public LKDetector
{
    public:
        AToMX();
        virtual ~AToMX() { ; }

        void Print(Option_t *option="") const;
        bool Init();
        bool BuildGeometry();
        bool BuildDetectorPlane();
        bool IsInBoundary(Double_t x, Double_t y, Double_t z);

    ClassDef(AToMX,1);
};

#endif
