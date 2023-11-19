#include "AToMXPlane.h"

ClassImp(AToMXPlane);

AToMXPlane::AToMXPlane()
{
    fName = "AToMXPlane";
    if (fChannelArray==nullptr)
        fChannelArray = new TObjArray();
}

bool AToMXPlane::Init()
{
    // Put intialization todos here which are not iterative job though event
    lk_info << "Initializing AToMXPlane" << std::endl;

    return true;
}

void AToMXPlane::Clear(Option_t *option)
{
    LKDetectorPlane::Clear(option);
}

void AToMXPlane::Print(Option_t *option) const
{
    // You will probability need to modify here
    lk_info << "AToMXPlane" << std::endl;
}

bool AToMXPlane::IsInBoundary(Double_t x, Double_t y)
{
    // example (x,y) is inside the plane boundary
    //if (x>-10 and x<10)
    //    return true;
    //return false;
    return true;
}

Int_t AToMXPlane::FindChannelID(Double_t x, Double_t y)
{
    // example find id
    // int id = 100*x + y;
    // return id;
    return -1;
}

Int_t AToMXPlane::FindChannelID(Int_t section, Int_t row, Int_t layer)
{
    // example find id
    // int id = 10000*section + 100*row + layer;
    // return id;
    return -1;
}

/*
TCanvas* AToMXPlane::GetCanvas(Option_t *option)
{
    // example canvas
    // if (fCanvas==nullptr)
    //     fCanvas = new TCanvas("AToMXPlane",AToMXPlane);
    // return fCanvas;
    return (TCanvas *) nullptr;
}
*/

TH2* AToMXPlane::GetHist(Option_t *option)
{
    // example hist
    // if (fHist==nullptr)
    //     fHist = new TH2D("AToMXPlane",AToMXPlane,10,0,10);
    // return fHist;
    return (TH2D *) nullptr;
}

/*
bool AToMXPlane::SetDataFromBranch()
{
    return false;
}
*/

/*
void AToMXPlane::FillDataToHist()
{
    // example hist
    // if (fHist==nullptr)
    //     fHist = new TH2D("AToMXPlane",AToMXPlane,10,0,10);
    // return fHist;
    return (TH2D *) nullptr;
}
*/

void AToMXPlane::DrawFrame(Option_t *option)
{
    ;
}

/*
void AToMXPlane::Draw(Option_t *option)
{
    SetDataFromBranch();
    FillDataToHist();
    auto hist = GetHist();
    if (hist==nullptr)
        return;
    if (fPar->CheckPar(fName+"/histZMin")) hist -> SetMinimum(fPar->GetParDouble(fName+"/histZMin"));
    else hist -> SetMinimum(0.01);
    if (fPar->CheckPar(fName+"/histZMax")) hist -> SetMaximum(fPar->GetParDouble(fName+"/histZMin"));
    auto cvs = GetCanvas();
    cvs -> Clear();
    cvs -> cd();
    hist -> Reset();
    hist -> DrawClone("colz");
    hist -> Reset();
    hist -> Draw("same");
    DrawFrame();
}
*/

/*
void AToMXPlane::MouseClickEvent(int iPlane)
{
    ;
}
*/

