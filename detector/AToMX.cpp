#include "AToMX.h"
#include "ATMicromegas.h"

ClassImp(AToMX);

AToMX::AToMX()
    :LKATTPC("atomx","Active target TPC for Multiple nuclear eXperiment")
{
}

bool AToMX::BuildDetectorPlane()
{
    AddPlane(new ATMicromegas);
    return true;
}
