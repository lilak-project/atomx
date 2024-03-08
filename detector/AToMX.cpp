#include "AToMX.h"
#include "ATMicromegas.h"

ClassImp(AToMX);

AToMX::AToMX()
{
    fName = "AToMX";
    if (fDetectorPlaneArray==nullptr)
        fDetectorPlaneArray = new TObjArray();
}

bool AToMX::Init()
{
    LKDetector::Init();

    // Put intialization todos here which are not iterative job though event
    lk_info << "Initializing AToMX" << std::endl;

    return true;
}

void AToMX::Print(Option_t *option) const
{
    // You will probability need to modify here
    lk_info << "AToMX" << std::endl;
}

bool AToMX::BuildGeometry()
{
    return true;
}

bool AToMX::BuildDetectorPlane()
{
    // example plane
    AddPlane(new ATMicromegas);
    return true;
}

bool AToMX::IsInBoundary(Double_t x, Double_t y, Double_t z)
{
    // example (x,y,z) is inside the plane boundary
    //if (x>-10 and x<10)
    //    return true;
    //return false;
    return true;
}

bool AToMX::GetEffectiveDimension(Double_t &x1, Double_t &y1, Double_t &z1, Double_t &x2, Double_t &y2, Double_t &z2)
{
    x1 = -80*4;
    x2 = +80*4;
    y1 = -200;
    y2 = +200;
    z1 = -80*4;
    z2 = +80*4;
    return true;
}

