#include <fstream>
using namespace std;
#include "LKWindowManager.h"
#include "ATMicromegas.h"
#include "GETChannel.h"
#include "TStyle.h"

ClassImp(ATMicromegas)

ATMicromegas::ATMicromegas()
: LKEvePlane("ATMicromegas","AToM-X Micromegas configuration")
{
    fName = "AToMXPlane";

    fAxis1 = LKVector3::kZ;
    fAxis2 = LKVector3::kX;
    fAxis3 = LKVector3::kY;
    fAxisDrift = LKVector3::kY;
    fChannelAnalyzer = nullptr;
}

void ATMicromegas::Clear(Option_t *option)
{
    LKEvePlane::Clear(option);
}

void ATMicromegas::Print(Option_t *option) const
{
    lk_info << endl;
}

bool ATMicromegas::Init()
{
    LKEvePlane::Init();

    fPar -> UpdatePar(fMappingFileName,"atomx/Mapping {lilak_common}/atomx_micromegas_mapping.txt # cobo asad aget chan x y");
    fPar -> UpdatePar(fDefinePositionByPixelIndex,"atomx/DefinePositionByPixelIndex false");
    fPar -> UpdatePar(fThreshold,"atomx/EveThreshold 300");

    if (fDefinePositionByPixelIndex)
    {
        fPosition = 0;
        fTbToLength = 1;
    }
    else {
        fPosition = 0;
        fTbToLength = 1;
        //fPar -> UpdatePar(fTbToLength,"ATMicromgegas/tb_to_length 1 # conversion from tb to length (length = tb * tb_to_length)");
    }

    //GetChannelAnalyzer();

    //gStyle -> SetPalette(kRainBow);
    //gStyle -> SetNumberContours(99);

    fMapCAACToPadID = new int***[fNumCobo];
    for(int i=0; i<fNumCobo; ++i) {
        fMapCAACToPadID[i] = new int**[fNumAsad];
        for(int j=0; j<fNumAsad; ++j) {
            fMapCAACToPadID[i][j] = new int*[fNumAget];
            for(int k=0; k<fNumAget; ++k) {
                fMapCAACToPadID[i][j][k] = new int[fNumChan];
                for(int l=0; l<fNumChan; ++l) {
                    fMapCAACToPadID[i][j][k][l] = -1;
                }
            }
        }
    }

    ifstream fileCAACMap(fMappingFileName);
    if (!fileCAACMap.is_open()) {
        lk_error << "Cannot open " << fMappingFileName << endl;
        return false;
    }
    lk_info << "mapping: " << fMappingFileName << endl;

    for (auto ix=0; ix<fNX; ++ix) {
        for (auto iz=0; iz<fNZ; ++iz) {
            auto padID = iz + ix*fNZ;
            LKPhysicalPad* pad = new LKPhysicalPad();
            pad -> SetPadID(padID);
            pad -> SetPlaneID(0);
            pad -> SetSection(0);
            pad -> SetLayer(iz);
            pad -> SetRow(ix);
            auto posz = (iz-fNZ/2+0.5)*fDZPad;
            auto posx = (ix-fNX/2+0.5)*fDXPad;
            if (fDefinePositionByPixelIndex) {
                posz = iz;
                posx = ix;
            }
            pad -> SetPosition(posz,posx);
            fChannelArray -> Add(pad);
        }
    }

    int countMap = 0;
    int coboIn, asadIn, aget, chan, iz1, ix1;
    while (fileCAACMap >> coboIn >> asadIn >> aget >> chan >> iz1 >> ix1)
    {
        for (auto addAsad=0; addAsad<4; ++addAsad)
        {
            int iz = iz1 - 1;
            int ix = ix1 - 1;
            int cobo = coboIn;
            int asad = asadIn;
            if (addAsad>0) {
                asad = asadIn + addAsad;
                iz = iz + 4*addAsad;
            }
            auto padID = iz + ix*fNZ;
            fMapCAACToPadID[cobo][asad][aget][chan] = padID;
            auto pad = (LKPhysicalPad*) fChannelArray -> At(padID);
            pad -> SetCoboID(cobo);
            pad -> SetAsadID(asad);
            pad -> SetAgetID(aget);
            pad -> SetChannelID(chan);
            ++countMap;
        }
    }

    lk_info << countMap << " pads are mapped!" << endl;

    //if (fRun!=nullptr) fRawDataArray = fRun -> GetBranchA("RawData");

    fChannelGraphArray = new TClonesArray("TGraph",20);

    return true;
}

TH2D* ATMicromegas::GetHistEventDisplay1(Option_t *option)
{
    if (fHistEventDisplay1==nullptr)
    {
        if (fDefinePositionByPixelIndex)
            fHistEventDisplay1 = new TH2D("ATMicromegas_Top","AToM-X Micromeagas;z;x",fNZ,0,fNZ,fNX,0,fNX);
        else {
            fHistEventDisplay1 = new TH2D("ATMicromegas_Top","AToM-X Micromeagas;z;x",fNZ,-0.5*fDZActive,+0.5*fDZActive,fNX,-0.5*fDXActive,+0.5*fDXActive);
        }
        fHistEventDisplay1 -> GetYaxis() -> SetNdivisions(512);
        fHistEventDisplay1 -> GetXaxis() -> SetNdivisions(512);
        fHistEventDisplay1 -> SetStats(0);
        fHistEventDisplay1 -> GetXaxis() -> SetTickSize(0);
        fHistEventDisplay1 -> GetYaxis() -> SetTickSize(0);

        const int maxPads = fNZ*fNX;
        const int maxBins = (fNZ+2)*(fNX+2);
        fMapBin1ToPadID = new int[maxBins]; for (auto i=0; i<maxBins; ++i) fMapBin1ToPadID[i] = -1;

        for (auto iz=0; iz<fNZ; ++iz) {
            for (auto ix=0; ix<fNX; ++ix) {
                auto bing = fHistEventDisplay1 -> GetBin(iz+1,ix+1);
                auto padID = iz + ix*fNZ;
                if (bing>=0)
                    fMapBin1ToPadID[bing] = padID;
            }
        }
    }

    return fHistEventDisplay1;
}

TH2D* ATMicromegas::GetHistEventDisplay2(Option_t *option)
{
    if (fHistEventDisplay2==nullptr)
    {
        fNY = 128;
        if (fDefinePositionByPixelIndex) {
            //fHistEventDisplay2 = new TH2D("ATMicromegas_Side","AToM-X Side;z;512-tb",fNZ,0,fNZ,fNY,0,fNY);
            fHistEventDisplay2 = new TH2D("ATMicromegas_Side","AToM-X Side;z;512-tb",fNZ,0,fNZ,fNY,0,512);
        }
        else {
            double yActive1 = 150;
            double yActive2 = -150;
            fHistEventDisplay2 = new TH2D("ATMicromegas_Side","AToM-X Side;z;y",fNZ,-0.5*fDZActive,+0.5*fDZActive,fNY,yActive1,yActive2);
        }
        fHistEventDisplay2 -> GetYaxis() -> SetNdivisions(512);
        fHistEventDisplay2 -> GetXaxis() -> SetNdivisions(512);
        fHistEventDisplay2 -> SetStats(0);
        fHistEventDisplay2 -> GetXaxis() -> SetTickSize(0);
        fHistEventDisplay2 -> GetYaxis() -> SetTickSize(0);
        fHistEventDisplay2 -> SetMinimum(fEnergyMin);

        /*
        const int maxBins2 = (fNZ+2)*(fNY+2);
        fMapBin2ToIZ = new int[maxBins2]; for (auto i=0; i<maxBins2; ++i) fMapBin2ToIZ[i] = -1;

        for (auto iz=0; iz<fNZ; ++iz) {
            for (auto iy=0; iy<fNY; ++iy) {
                auto bing = fHistEventDisplay2 -> GetBin(iz+1,iy+1);
                fMapBin2ToIZ[bing] = iz;
            }
        }
        */
    }


    return fHistEventDisplay2;
}

TH1D* ATMicromegas::GetHistChannelBuffer()
{
    if (fHistChannelBuffer==nullptr)
    {
        fHistChannelBuffer = new TH1D("ATMicromegas_Channel",";tb;y",512,0,512);
        fHistChannelBuffer -> SetStats(0);
        fHistChannelBuffer -> SetLineColor(kBlack);
        fHistChannelBuffer -> GetXaxis() -> SetTitleSize(0.06);
        fHistChannelBuffer -> GetYaxis() -> SetTitleSize(0.06);
        fHistChannelBuffer -> GetYaxis() -> SetTitleOffset(1.0);
        fHistChannelBuffer -> GetXaxis() -> SetLabelSize(0.06);
        fHistChannelBuffer -> GetYaxis() -> SetLabelSize(0.06);
    }
    return fHistChannelBuffer;
}

int ATMicromegas::FindPadID(int cobo, int asad, int aget, int chan)
{
    return fMapCAACToPadID[cobo][asad][aget][chan];
}

LKPhysicalPad* ATMicromegas::FindPad(int cobo, int asad, int aget, int chan)
{
    LKPhysicalPad *pad = nullptr;
    auto padID = fMapCAACToPadID[cobo][asad][aget][chan];
    if (padID>=0) {
        pad = (LKPhysicalPad*) fChannelArray -> At(padID);
        return pad;
    }
    return (LKPhysicalPad*) nullptr;
}

int ATMicromegas::FindPadIDFromHistEventDisplay1Bin(int hbin)
{
    return fMapBin1ToPadID[hbin];
}

TPad* ATMicromegas::Get3DEventPad()
{
    if (fCurrentView==0)
        return GetPadEventDisplay2();
    else {
        return (TPad *) nullptr;
    }
}

void ATMicromegas::UpdateEventDisplay2()
{
    if (fCurrentView==0) // 3d
        return;

    if (fHistEventDisplay2==nullptr) 
        return;

    fPadEventDisplay2 -> cd();
    fPadEventDisplay2 -> SetGrid();
    if      (fEnergyMax==0) { fHistEventDisplay2 -> SetMinimum(fEnergyMin); fHistEventDisplay2 -> SetMaximum(-1111); }
    else if (fEnergyMax==1) { fHistEventDisplay2 -> SetMinimum(fEnergyMin); fHistEventDisplay2 -> SetMaximum(4200); }
    else
    {
        fHistEventDisplay2 -> SetMinimum(fEnergyMin);
        fHistEventDisplay2 -> SetMaximum(fEnergyMax);
        if (fEnergyMax>100)
            gStyle -> SetNumberContours(100);
        else
            gStyle -> SetNumberContours(fEnergyMax);
    }
    fHistEventDisplay2 -> Draw(fEventDisplayDrawOption);
    fEventDisplayDrawOption = "colz";
}

void ATMicromegas::FillDataToHistEventDisplay2(Option_t *option)
{
    if (fCurrentView==0) // 3d
        return;

    TString optionString(option);
    optionString.ToLower();

    TString title;

    if (fAccumulateEvents==0)
        fHistEventDisplay2 -> Reset();

    if (optionString.Index("hit")>=0&&fHitArray!=nullptr)
    {
        if (fAccumulateEvents==0) lk_info << "Filling hit to plane" << endl;
        title = "Hit";
        TIter nextHit(fHitArray);
        LKHit* hit = nullptr;
        while (hit = (LKHit*) nextHit())
        {
            auto pos = hit -> GetPosition(fAxisDrift);
            auto i = pos.I();
            auto j = pos.J();
            auto k = pos.K();
            auto energy = hit -> GetCharge();
            fHistEventDisplay2 -> Fill(i,k,energy);
        }
    }
    //else if (optionString.Index("raw")>=0&&fRawDataArray!=nullptr)
    else if (fRawDataArray!=nullptr)
    {
        if (fAccumulateEvents==0) lk_info << "Filling raw data to plane" << endl;
        title = "Raw Data";
        TIter nextRawData(fChannelArray);
        LKPhysicalPad *pad = nullptr;
        while (pad = (LKPhysicalPad*) nextRawData())
        {
            auto iz = pad -> GetI();
            auto ix = pad -> GetJ();
            auto idx = pad -> GetDataIndex();
            if (idx<0)
                continue;
            auto channel = (GETChannel*) fRawDataArray -> At(idx);
            auto buffer = channel -> GetWaveformY();
            auto pedestal = channel -> GetPedestal();
            auto energy = channel -> GetEnergy();
            auto time = channel -> GetTime();
            //if (energy>0||time>0)
            if (0)
            {
                if (energy>100) lk_debug << time << " " << energy << endl;
                fHistEventDisplay2 -> Fill(iz,time,energy);
            }
            else {
                fChannelAnalyzer -> Analyze(buffer);
                pedestal = fChannelAnalyzer -> GetPedestal();
                for (auto tb=0; tb<512; ++tb) {
                    auto value = buffer[tb] - pedestal;
                    if (value>fThreshold) {
                        //if (iz==0) lk_debug << "ix,iz,p,e,t,v: " << ix << " " << iz << " " << pedestal << " " << energy << " " << time << " " << value << endl;
                        fHistEventDisplay2 -> Fill(iz,512-tb,value);
                    }
                }
            }
        }
    }
}

void ATMicromegas::ClickedEventDisplay2(double xOnClick, double yOnClick)
{
    if (fHistEventDisplay2==nullptr)
        return;

    int selectedBin = fHistEventDisplay2 -> FindBin(xOnClick, yOnClick);
    fSelIZ = FindZFromHistEventDisplay2Bin(selectedBin);

    fCountChannelGraph = 0;
    fAccumulateChannel = 3;
    fHistControlEvent2 -> SetBinContent(fBinCtrlAcmltCh, 3);
    fHistControlEvent2 -> SetBinContent(fBinCtrlDrawACh, 3);

    UpdateChannelBuffer();
}

int ATMicromegas::FindZFromHistEventDisplay2Bin(int hbin)
{
    Int_t binx, biny, binz;
    fHistEventDisplay2 -> GetBinXYZ(hbin, binx, biny, binz);
    return (binx - 1);
    //return fMapBin2ToIZ[hbin];
}

void ATMicromegas::UpdateChannelBuffer()
{
    if (fHistChannelBuffer==nullptr) 
        return;

    if (fSelIZ>=0)
    {
        if (fDefinePositionByPixelIndex)
        {
            double z1 = fSelIZ;
            double z2 = fSelIZ+1;
            double y1 = 0;
            double y2 = 512;
            fGSelEventDisplay2 -> SetPoint(0,z1,y1);
            fGSelEventDisplay2 -> SetPoint(1,z1,y2);
            fGSelEventDisplay2 -> SetPoint(2,z2,y2);
            fGSelEventDisplay2 -> SetPoint(3,z2,y1);
            fGSelEventDisplay2 -> SetPoint(4,z1,y1);
            fGSelEventDisplay2 -> SetLineColor(kRed);
            if (fPaletteNumber==0)
                fGSelEventDisplay2 -> SetLineColor(kGreen);
        }
        fPadEventDisplay2 -> cd();
        fGSelEventDisplay2 -> Draw("samel");

        fPadChannelBuffer -> cd();
        fHistChannelBuffer -> Reset();
        fHistChannelBuffer -> SetTitle(Form("iz = %d",fSelIZ));
        fHistChannelBuffer -> Draw();
        double yMin=DBL_MAX, yMax=-DBL_MAX;

        auto iz0 = fSelIZ;
        for (auto iPad=0; iPad<fNX; iPad++)
        {
            auto padID = iz0 + fNZ*iPad;
            //auto padID = iz + ix*fNZ;
            auto pad = (LKPhysicalPad*) fChannelArray -> At(padID);
            auto iz = pad -> GetI();
            auto ix = pad -> GetJ();
            auto idx = pad -> GetDataIndex();
            if (idx<0)
                continue;
            auto channel = (GETChannel*) fRawDataArray -> At(idx);
            auto graph = (TGraph*) fChannelGraphArray -> ConstructedAt(fCountChannelGraph);
            fCountChannelGraph++;
            channel -> FillGraph(graph);
            graph -> Draw("plc samel");
            double x0, y0;
            auto n = graph -> GetN();
            for (auto i=0; i<n; ++i) {
                graph -> GetPoint(i,x0,y0);
                if (yMin>y0) yMin = y0;
                if (yMax<y0) yMax = y0;
            }
        }

        if (fCountChannelGraph>0)
        {
            lk_info << fCountChannelGraph << " channels in iz = " << fSelIZ << endl;
            double dy = 0.1*(yMax - yMin);
            yMin = yMin - dy;
            yMax = yMax + dy;
            if (yMin<0) yMin = 0;
            if (yMax>4200) yMax = 4200;
            fHistChannelBuffer -> SetMinimum(yMin);
            fHistChannelBuffer -> SetMaximum(yMax);
            fHistChannelBuffer -> SetTitle(Form("iz = %d (%d)",fSelIZ,fCountChannelGraph));
        }

        fSelIZ = -1;
    }
    else
        LKEvePlane::UpdateChannelBuffer();
}
