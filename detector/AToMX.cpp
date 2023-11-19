#include "AToMX.h"

ClassImp(AToMX);

AToMX::AToMX()
{
    fName = "AToMX";
    if (fDetectorPlaneArray==nullptr)
        fDetectorPlaneArray = new TObjArray();
}

bool AToMX::Init()
{
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
    // AddPlane(new MyPlane);
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

