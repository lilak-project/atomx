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

        virtual void Print(Option_t *option="") const;
        virtual bool Init();
        virtual bool BuildGeometry();
        virtual bool BuildDetectorPlane();
        virtual bool IsInBoundary(Double_t x, Double_t y, Double_t z);
        virtual bool GetEffectiveDimension(Double_t &x1, Double_t &y1, Double_t &z1, Double_t &x2, Double_t &y2, Double_t &z2);

    private:
        bool fDefinePositionByPixelIndex;

    ClassDef(AToMX,1);
};

#endif
