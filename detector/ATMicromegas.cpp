#include <fstream>
using namespace std;
#include "LKWindowManager.h"
#include "ATMicromegas.h"
#include "GETChannel.h"
#include "TStyle.h"

ClassImp(ATMicromegas)

ATMicromegas::ATMicromegas()
: LKDetectorPlane("ATMicromegas","AToM-X Micromegas configuration")
{
    fName = "AToMXPlane";

    fAxis1 = LKVector3::kZ;
    fAxis2 = LKVector3::kX;
    fAxis3 = LKVector3::kY;
    fAxisDrift = LKVector3::kY;
    fChannelAnalyzer = nullptr;
}

LKChannelAnalyzer* ATMicromegas::GetChannelAnalyzer(int id)
{
    if (fChannelAnalyzer==nullptr)
    {
        fChannelAnalyzer = new LKChannelAnalyzer();
        //fChannelAnalyzer -> SetPulse(pulseFileName);
        double threshold = 300;
        fPar -> UpdatePar(threshold,"atomx/Threshold  300  # threshold for default peak finding method");
        fChannelAnalyzer -> SetThreshold(threshold);
        fChannelAnalyzer -> Print();
    }
    return fChannelAnalyzer;
}

void ATMicromegas::Clear(Option_t *option)
{
    LKDetectorPlane::Clear(option);
}

void ATMicromegas::Print(Option_t *option) const
{
    lk_info << endl;
}

bool ATMicromegas::Init()
{
    fPar -> UpdatePar(fMappingFileName,"atomx/Mapping {lilak_common}/atomx_micromegas_mapping.txt # cobo asad aget chan x y");
    fPar -> UpdatePar(fDefinePositionByPixelIndex,"atomx/DefinePositionByPixelIndex false");

    if (fDefinePositionByPixelIndex)
    {
        fPosition = 0;
        fTbToLength = 1;
    }
    else {
        fPosition = 0;
        fTbToLength = 1;
        fPar -> UpdatePar(fTbToLength,"ATMicromgegas/tb_to_length 1 # conversion from tb to length (length = tb * tb_to_length)");
    }

    GetChannelAnalyzer();

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
    int cobo, asad, aget, chan, iz1, ix1;
    while (fileCAACMap >> cobo >> asad >> aget >> chan >> iz1 >> ix1)
    {
        auto padID = (iz1-1) + (ix1-1)*fNZ;
        fMapCAACToPadID[cobo][asad][aget][chan] = padID;
        auto pad = (LKPhysicalPad*) fChannelArray -> At(padID);
        pad -> SetCoboID(cobo);
        pad -> SetAsadID(asad);
        pad -> SetAgetID(aget);
        pad -> SetChannelID(chan);
        ++countMap;
    }

    lk_info << countMap << " pads are mapped!" << endl;

    if (fRun!=nullptr)
        fRawDataArray = fRun -> GetBranchA("RawData");

    fChannelGraphArray = new TClonesArray("TGraph",20);

    return true;
}

int ATMicromegas::FindPadID(int cobo, int asad, int aget, int chan)
{
    return fMapCAACToPadID[cobo][asad][aget][chan];
}

void ATMicromegas::Draw(Option_t *option)
{
    TString fillOption;
    TString optionString(option);
    int ic = optionString.Index(":");
    if (ic>=0) {
        fillOption = optionString(0,ic);
        f2DEventDrawOption = optionString(ic+1,optionString.Sizeof()-ic-2);
    }

    GetCanvas();
    GetHist();
    SetDataFromBranch();

    FillDataToHist(fillOption);
    UpdateAll();
}

void ATMicromegas::UpdateAll()
{
    Update2DEvent();
    UpdateChannel();
    UpdateCtrlEv1();
    UpdateCtrlEv2();
}

void ATMicromegas::Update2DEvent()
{
    if (fHist2DEvent==nullptr) 
        return;
    fPad2DEvent -> cd();
    fPad2DEvent -> SetGrid();
    if      (fEnergyMaxMode==0) { fHist2DEvent -> SetMinimum(-1111); fHist2DEvent -> SetMaximum(-1111); }
    else if (fEnergyMaxMode==1) { fHist2DEvent -> SetMinimum(0); fHist2DEvent -> SetMaximum(2500); }
    else if (fEnergyMaxMode==2) { fHist2DEvent -> SetMinimum(0); fHist2DEvent -> SetMaximum(4200); }
    else
    {
        fHist2DEvent -> SetMinimum(0);
        fHist2DEvent -> SetMaximum(fEnergyMaxMode);
        if (fEnergyMaxMode>100)
            gStyle -> SetNumberContours(100);
        else
            gStyle -> SetNumberContours(fEnergyMaxMode);
    }
    fHist2DEvent -> Draw(f2DEventDrawOption);
    f2DEventDrawOption = "colz";
}

void ATMicromegas::UpdateChannel()
{
    if (fHistChannel==nullptr) 
        return;

    auto pad = (LKPhysicalPad*) fChannelArray -> At(fSelPadIdx);
    if (pad==nullptr) {
        lk_error << "pad at " << fSelPadIdx << " is nullptr" << endl;
        return;
    }

    double z0 = pad -> GetI();
    double x0 = pad -> GetJ();
    double z1 = z0 - 0.5*fDZPad;
    double z2 = z0 + 0.5*fDZPad;
    double x1 = x0 - 0.5*fDXPad;
    double x2 = x0 + 0.5*fDXPad;
    if (fDefinePositionByPixelIndex) {
        z1 = z0;
        z2 = z0 + 1;
        x1 = x0;
        x2 = x0 + 1;
    }

    fPad2DEvent -> cd();
    fGSel2DEvent -> Set(0);
    fGSel2DEvent -> SetPoint(0,z1,x1);
    fGSel2DEvent -> SetPoint(1,z1,x2);
    fGSel2DEvent -> SetPoint(2,z2,x2);
    fGSel2DEvent -> SetPoint(3,z2,x1);
    fGSel2DEvent -> SetPoint(4,z1,x1);
    fGSel2DEvent -> SetLineColor(kGray+1);
    fGSel2DEvent -> SetLineColor(kRed);
    fGSel2DEvent -> Draw("samel");

    fSelRawDataIdx = pad -> GetDataIndex();

    if (fRawDataArray!=nullptr&&fSelRawDataIdx>=0)
    {
        auto channel = (GETChannel*) fRawDataArray -> At(fSelRawDataIdx);
        if (fAccumulateChannel)
            fHistChannel -> Reset();
        else
            channel -> FillHist(fHistChannel);

        fPadChannel -> cd();
        if (fAccumulateChannel)
            fHistChannel -> SetMaximum(4200);
        else {
            if      (fEnergyMaxMode==0) fHistChannel -> SetMaximum(-1111);
            else if (fEnergyMaxMode==1) fHistChannel -> SetMaximum(2500);
            else if (fEnergyMaxMode==2) fHistChannel -> SetMaximum(4200);
        }
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto engy = channel -> GetEnergy();
        auto time = channel -> GetTime();
        auto pdst = channel -> GetPedestal();
        TString title = Form("(CAAC) = (%d, %d, %d, %d)   |   (TEP)=(%.1f, %.1f, %.1f)", cobo, asad, aget, chan, time, engy, pdst);
        fHistChannel -> SetTitle(title);
        fHistChannel -> Draw();

        if (fAccumulateChannel)
        {
            auto graph = (TGraph*) fChannelGraphArray -> ConstructedAt(fCountChannelGraph);
            channel -> FillGraph(graph);
            fCountChannelGraph++;
            double yMin=DBL_MAX, yMax=-DBL_MAX;
            for (auto iGraph=0; iGraph<fCountChannelGraph; ++iGraph)
            {
                auto graph = (TGraph*) fChannelGraphArray -> At(iGraph);
                graph -> Draw("plc samel");
                double x0, y0;
                auto n = graph -> GetN();
                for (auto i=0; i<n; ++i) {
                    graph -> GetPoint(i,x0,y0);
                    if (yMin>y0) yMin = y0;
                    if (yMax<y0) yMax = y0;
                }
            }
            double dy = 0.1*(yMax - yMin);
            yMin = yMin - dy;
            yMax = yMax + dy;
            if (yMin<0) yMin = 0;
            if (yMax>4200) yMax = 4200;
            fHistChannel -> SetMinimum(yMin);
            fHistChannel -> SetMaximum(yMax);
        }

        if (fFitChannel)
        {
            fChannelAnalyzer -> Analyze(channel->GetWaveformY());
            auto numHits = fChannelAnalyzer -> GetNumHits();
            fPadChannel -> cd();
            auto graphPedestal = fChannelAnalyzer -> GetPedestalGraph();
            graphPedestal -> SetLineColor(kOrange-3);
            graphPedestal -> Draw("samel");
            for (auto iHit=0; iHit<numHits; ++iHit)
            {
                auto tbHit = fChannelAnalyzer -> GetTbHit(iHit);
                auto amplitude = fChannelAnalyzer -> GetAmplitude(iHit);
                auto pedestal = fChannelAnalyzer -> GetPedestal();
                lk_info << iHit << ") (T,E,P) = (" << tbHit << ", " << amplitude << ", " << pedestal << ")" << endl;
                auto graph = fChannelAnalyzer -> GetPulseGraph(tbHit,amplitude,pedestal);
                graph -> SetLineColor(kBlue-4);
                graph -> SetLineStyle(2);
                graph -> Draw("samel");
            }
        }
        fFitChannel = false;
    }
    else {
        fPadChannel -> cd();
        fHistChannel -> Draw();
    }
}

void ATMicromegas::UpdateCtrlEv1()
{
    if (fHistCtrlEv1==nullptr)
        return;
    fPadCtrlEv1 -> cd();
    fPadCtrlEv1 -> SetGrid();
    if (fRun!=nullptr) {
        auto currentEventID = fRun -> GetCurrentEventID();
        auto lastEventID = fRun -> GetNumEvents() - 1;
        fHistCtrlEv1 -> SetBinContent(fBinCtrlPr50, (currentEventID-50<0?0:currentEventID-50));
        fHistCtrlEv1 -> SetBinContent(fBinCtrlPrev, (currentEventID==0?0:currentEventID-1));
        fHistCtrlEv1 -> SetBinContent(fBinCtrlNext, (currentEventID==lastEventID?lastEventID:currentEventID+1));
        fHistCtrlEv1 -> SetBinContent(fBinCtrlNe50, (currentEventID+50>lastEventID?lastEventID:currentEventID+50));
        fHistCtrlEv1 -> Draw("col text");
    }
    else
        fHistCtrlEv1 -> Draw("text");
}

void ATMicromegas::UpdateCtrlEv2()
{
    if (fHistCtrlEv2==nullptr)
        return;
    fPadCtrlEv2 -> cd();
    fPadCtrlEv2 -> SetGrid();
    if (fRun!=nullptr)
        fHistCtrlEv2 -> Draw("col text");
    else
        fHistCtrlEv2 -> Draw("text");
}

TCanvas *ATMicromegas::GetCanvas(Option_t *option)
{
    if (fCanvas==nullptr)
    {
        fCanvas = LKWindowManager::GetWindowManager() -> CanvasResize("TTMicromegas",1100,700,0.9);
        fPad2DEvent = new TPad("pad_2DEvent","",0,230./700,0.5,1);
        fPad2DEvent -> SetMargin(0.12,0.15,0.1,0.1);
        fPad2DEvent -> SetNumber(1);
        fPad2DEvent -> Draw();
        fPadChannel = new TPad("pad_channel","",0,0,0.5,230./700);
        fPadChannel -> SetMargin(0.12,0.05,0.20,0.12);
        fPadChannel -> SetNumber(2);
        fPadChannel -> Draw();
        fPad3DEvent = new TPad("pad_3DEvent","",0.5,230./700,1,1);
        fPad3DEvent -> SetMargin(0.12,0.15,0.1,0.1);
        fPad3DEvent -> SetNumber(3);
        fPad3DEvent -> Draw();

        double yCtrl1 = 0;
        double yCtrl2 = 230./700;
        double y1 = 0;
        double y2 = y1 + 0.5*(yCtrl2-yCtrl1);
        fPadCtrlEv1 = new TPad("pad_control","",0.5,y1,1,y2);
        //fPadCtrlEv1 -> SetMargin(0.12,0.05,0.20,0.12);
        fPadCtrlEv1 -> SetMargin(0.02,0.02,0.30,0.02);
        fPadCtrlEv1 -> SetNumber(4);
        fPadCtrlEv1 -> Draw();
        y1 = y2;
        y2 = y1 + 0.5*(yCtrl2-yCtrl1);
        fPadCtrlEv2 = new TPad("pad_control","",0.5,y1,1,y2);
        //fPadCtrlEv2 -> SetMargin(0.12,0.05,0.20,0.12);
        fPadCtrlEv2 -> SetMargin(0.02,0.02,0.30,0.02);
        fPadCtrlEv2 -> SetNumber(4);
        fPadCtrlEv2 -> Draw();

        fCanvas -> Modified();
        fCanvas -> Update();

        AddInteractivePad(fPad2DEvent);
        AddInteractivePad(fPadChannel);
        AddInteractivePad(fPadCtrlEv1);
        AddInteractivePad(fPadCtrlEv2);
    }

    return fCanvas;
}

TH2* ATMicromegas::GetHist(Option_t *option)
{
    if (fHist2DEvent==nullptr)
    {
        if (fDefinePositionByPixelIndex)
            fHist2DEvent = new TH2D("ATMicromegas_2DEvent","AToM-X Micromeagas;z;x",fNZ,0,fNZ,fNX,0,fNX);
        else {
            fHist2DEvent = new TH2D("ATMicromegas_2DEvent","AToM-X Micromeagas;z;x",fNZ,-0.5*fDZActive,+0.5*fDZActive,fNX,-0.5*fDXActive,+0.5*fDXActive);
        }
        fHist2DEvent -> GetYaxis() -> SetNdivisions(512);
        fHist2DEvent -> GetXaxis() -> SetNdivisions(512);
        fHist2DEvent -> SetStats(0);
        fHist2DEvent -> GetXaxis() -> SetTickSize(0);
        fHist2DEvent -> GetYaxis() -> SetTickSize(0);
        fGSel2DEvent = new TGraph();
        fGSel2DEvent -> SetLineColor(kRed);

        const int maxPads = fNZ*fNX;
        const int maxBins = (fNZ+2)*(fNX+2);
        fMapPadIdxToBin = new int[maxPads]; for (auto i=0; i<maxPads; ++i) fMapPadIdxToBin[i] = -1;
        fMapBinToPadIdx = new int[maxBins]; for (auto i=0; i<maxBins; ++i) fMapBinToPadIdx[i] = -1;

        fMapZXToBin = new int*[fNZ];
        for (auto iz=0; iz<fNZ; ++iz)
            fMapZXToBin[iz] = new int[fNX];

        for (auto iz=0; iz<fNZ; ++iz) {
            for (auto ix=0; ix<fNX; ++ix) {
                auto bing = fHist2DEvent -> GetBin(iz+1,ix+1);
                auto padID = iz + ix*fNZ;
                fMapZXToBin[iz][ix] = bing;
                fMapPadIdxToBin[padID] = bing;
                if (bing>=0)
                    fMapBinToPadIdx[bing] = padID;
            }
        }

        fHistChannel = new TH1D("ATMicromegas_Channel",";tb;y",512,0,512);
        fHistChannel -> SetStats(0);
        fHistChannel -> SetLineColor(kBlack);
        fHistChannel -> GetXaxis() -> SetTitleSize(0.06);
        fHistChannel -> GetYaxis() -> SetTitleSize(0.06);
        fHistChannel -> GetYaxis() -> SetTitleOffset(1.0);
        fHistChannel -> GetXaxis() -> SetLabelSize(0.06);
        fHistChannel -> GetYaxis() -> SetLabelSize(0.06);

        gStyle -> SetHistMinimumZero(); // this will draw text even when content is 0

        double binTextSize = 6.0;
        double ctrlLabelSize = 0.20;

        fHistCtrlEv1 = new TH2D("ATMicromegas_CtrlEv1","",6,0,6,1,0,1);
        fHistCtrlEv1 -> SetStats(0);
        fBinCtrlFrst = fHistCtrlEv1 -> GetBin(1,1);
        fBinCtrlPr50 = fHistCtrlEv1 -> GetBin(2,1);
        fBinCtrlPrev = fHistCtrlEv1 -> GetBin(3,1);
        fBinCtrlNext = fHistCtrlEv1 -> GetBin(4,1);
        fBinCtrlNe50 = fHistCtrlEv1 -> GetBin(5,1);
        fBinCtrlLast = fHistCtrlEv1 -> GetBin(6,1);
        fHistCtrlEv1 -> GetXaxis() -> SetTickSize(0);
        fHistCtrlEv1 -> GetYaxis() -> SetTickSize(0);
        fHistCtrlEv1 -> GetYaxis() -> SetBinLabel(1,"");
        fHistCtrlEv1 -> GetXaxis() -> SetLabelSize(ctrlLabelSize);
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(1,"First");
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(2,"-50");
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(3,"Prev.");
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(4,"Next");
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(5,"+50");
        fHistCtrlEv1 -> GetXaxis() -> SetBinLabel(6,"Last");
        fHistCtrlEv1 -> SetBinContent(fBinCtrlFrst,0);
        if (fRun!=nullptr)
            fHistCtrlEv1 -> SetBinContent(fBinCtrlLast,fRun->GetNumEvents()-1);
        else {
            fHistCtrlEv1 -> SetBinContent(fBinCtrlLast,0);
        }
        fHistCtrlEv1 -> SetMarkerSize(binTextSize);
        fHistCtrlEv1 -> SetMinimum(0);

        fHistCtrlEv2 = new TH2D("ATMicromegas_CtrlEv2","",6,0,6,1,0,1);
        fHistCtrlEv2 -> SetStats(0);
        fBinCtrlEngyMax = fHistCtrlEv2 -> GetBin(1,1);
        fBinCtrl4200Max = fHistCtrlEv2 -> GetBin(2,1);
        fBinCtrlAcmltCh = fHistCtrlEv2 -> GetBin(3,1);
        fBinCtrlFitChan = fHistCtrlEv2 -> GetBin(4,1);
        fBinCtrlNEEL500 = fHistCtrlEv2 -> GetBin(5,1);
        fBinCtrlNEEL203 = fHistCtrlEv2 -> GetBin(6,1);
        fHistCtrlEv2 -> GetXaxis() -> SetTickSize(0);
        fHistCtrlEv2 -> GetYaxis() -> SetTickSize(0);
        fHistCtrlEv2 -> GetYaxis() -> SetBinLabel(1,"");
        fHistCtrlEv2 -> GetXaxis() -> SetLabelSize(ctrlLabelSize);
        fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(1,"E(3)");  // fBinCtrlEngyMax
        //fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(2,"E2=4096");  // fBinCtrl4200Max
        fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(3,"Acc Ch.");  // fBinCtrlAcmltCh
        fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(4,"Fit Ch.");  // fBinCtrlFitChan
        fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(5,"@E>=500");  // fBinCtrlNEEL500
        fHistCtrlEv2 -> GetXaxis() -> SetBinLabel(6,"@E>=2000"); // fBinCtrlNEEL203
        fHistCtrlEv2 -> SetBinContent(fBinCtrlEngyMax, 1);
        //fHistCtrlEv2 -> SetBinContent(fBinCtrl4200Max, 4096);
        fHistCtrlEv2 -> SetBinContent(fBinCtrlAcmltCh, 0);
        fHistCtrlEv2 -> SetBinContent(fBinCtrlFitChan, 1);
        fHistCtrlEv2 -> SetBinContent(fBinCtrlNEEL500, 500);
        fHistCtrlEv2 -> SetBinContent(fBinCtrlNEEL203, 2000);
        fHistCtrlEv2 -> SetMarkerSize(binTextSize);
        fHistCtrlEv2 -> SetMinimum(0);
    }
    return (TH2*) fHist2DEvent;
}

void ATMicromegas::DrawHist(Option_t *option)
{
    FillDataToHist(option);

    auto hist = GetHist();
    if (hist==nullptr)
        return;

    GetCanvas();
    fPad2DEvent -> cd();
    hist -> Draw("colz");
}

bool ATMicromegas::SetDataFromBranch()
{
    LKPhysicalPad *pad = nullptr;
    TIter next(fChannelArray);
    while (pad = (LKPhysicalPad*) next())
        pad -> SetDataIndex(-1);

    if (fRawDataArray==nullptr)
        return false;

    fSelPadIdx = 0;
    fSelRawDataIdx = 0;
    double selEnergy = 0;

    auto numChannels = fRawDataArray -> GetEntries();
    for (auto iRawData=0; iRawData<numChannels; ++iRawData)
    {
        auto channel = (GETChannel*) fRawDataArray -> At(iRawData);
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto pad = FindPad(cobo,asad,aget,chan);
        if (pad==nullptr)
        {
            if (chan!=11&&chan!=22&&chan!=45&&chan!=56)
                lk_error << "Pad doesn't exist! CAAC= " << cobo << " " << asad << " " << aget << " " << chan << endl;
            continue;
        }
        pad -> SetTime(channel->GetTime());
        pad -> SetEnergy(channel->GetEnergy());
        pad -> SetPedestal(channel->GetPedestal());
        pad -> SetDataIndex(iRawData);

        if (channel->GetEnergy()>selEnergy) {
            auto padID = fMapCAACToPadID[cobo][asad][aget][chan];
            fSelPadIdx = padID;
            fSelRawDataIdx = iRawData;
            selEnergy = channel->GetEnergy();
        }
    }
    return true;
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

void ATMicromegas::DriftElectronBack(int padID, double tb, TVector3 &posReco, double &driftLength)
{
    auto pad = (LKPhysicalPad*) fChannelArray -> At(padID);
    LKVector3 pos(fAxis3);
    pos.SetI(pad->GetI());
    pos.SetJ(pad->GetJ());
    if (fAxis3!=fAxisDrift)
        pos.SetK(fTbToLength*tb+fPosition);
    else
        pos.SetK((-fTbToLength)*tb+fPosition);
    posReco = pos.GetXYZ();
    driftLength = fTbToLength*tb;
}

void ATMicromegas::FillDataToHist(Option_t* option)
{
    GetHist();
    fHist2DEvent -> Reset();

    TString optionString(option);
    optionString.ToLower();

    TIter next(fChannelArray);
    LKPhysicalPad *pad = nullptr;
    TString title;

    if (optionString.Index("caac")>=0) {
        lk_info << "Filling caac to plane" << endl;
        title = ("caac");
        int maxCAAC = 0;
        while ((pad = (LKPhysicalPad *) next())) {
            auto caac = pad -> GetCAAC();
            if (caac>maxCAAC) maxCAAC = caac;
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),caac);
        }
        fEnergyMaxMode = maxCAAC;
    }
    else if (optionString.Index("cobo")>=0) {
        fEnergyMaxMode = 4;
        lk_info << "Filling cobo to plane" << endl;
        title = ("cobo");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetCoboID());
    }
    else if (optionString.Index("asad")>=0) {
        fEnergyMaxMode = 4;
        lk_info << "Filling asad to plane" << endl;
        title = ("asad");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetAsadID());
    }
    else if (optionString.Index("aget")>=0) {
        fEnergyMaxMode = 4;
        lk_info << "Filling aget to plane" << endl;
        title = ("aget");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetAgetID());
    }
    else if (optionString.Index("chan")>=0) {
        fEnergyMaxMode = 70;
        lk_info << "Filling chan to plane" << endl;
        title = ("chan");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetChannelID());
    }

    else if (optionString.Index("section")>=0) {
        fEnergyMaxMode = 100;
        lk_info << "Filling section to plane" << endl;
        title = ("section");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetSection());
    }
    else if (optionString.Index("layer")>=0) {
        fEnergyMaxMode = 100;
        lk_info << "Filling layer to plane" << endl;
        title = ("layer");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetLayer());
    }
    else if (optionString.Index("row")>=0) {
        fEnergyMaxMode = 100;
        lk_info << "Filling row to plane" << endl;
        title = ("raw");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetRow());
    }

    else if (optionString.Index("padid")>=0) {
        fEnergyMaxMode = 100;
        lk_info << "Filling pad id to plane" << endl;
        title = ("id");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetPadID());
    }
    else if (optionString.Index("nhit")>=0) {
        fEnergyMaxMode = 10;
        lk_info << "Filling number of hits to plane" << endl;
        title = ("nhit");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetNumHits());
    }

    else if (fRawDataArray!=nullptr)
    {
        lk_info << "Filling raw data to plane" << endl;
        title = ("Raw Data");
        while (pad = (LKPhysicalPad*) next())
        {
            auto idx = pad -> GetDataIndex();
            if (idx<0)
                continue;;
            auto channel = (GETChannel*) fRawDataArray -> At(idx);
            auto i = pad -> GetI();
            auto j = pad -> GetJ();
            auto energy = channel -> GetEnergy();
            fHist2DEvent -> Fill(i,j,energy);
        }
    }

    if (fRun!=nullptr)
        fHist2DEvent -> SetTitle(Form("%s (event %lld)", fRun->GetInputFile()->GetName(), fRun->GetCurrentEventID()));
}

void ATMicromegas::ExecMouseClickEventOnPad(TVirtualPad *pad, double xOnClick, double yOnClick)
{
    if (pad==fPad2DEvent) Clicked2DEvent(xOnClick, yOnClick);
    if (pad==fPadCtrlEv1) ClickedCtrlEv1(xOnClick, yOnClick);
    if (pad==fPadCtrlEv2) ClickedCtrlEv2(xOnClick, yOnClick);
}
void ATMicromegas::Clicked2DEvent(double xOnClick, double yOnClick)
{
    if (fHist2DEvent==nullptr)
        return;

    int selectedBin = fHist2DEvent -> FindBin(xOnClick, yOnClick);
    auto padIdx = fMapBinToPadIdx[selectedBin];
    if (padIdx<0) {
        lk_error << "Pad index is " << padIdx << ". gbin = " << selectedBin << endl;
        return;
    }

    fSelPadIdx = padIdx;

    auto pad = (LKPhysicalPad*) fChannelArray -> At(fSelPadIdx);
    if (pad==nullptr) {
        lk_error << "pad at " << fSelPadIdx << " is nullptr" << endl;
        return;
    }

    pad -> Print();

    UpdateChannel();
}

void ATMicromegas::ClickedCtrlEv1(double xOnClick, double yOnClick)
{
    if (fHistCtrlEv1==nullptr)
        return;

    if (fRun==nullptr)
        return;

    int selectedBin = fHistCtrlEv1 -> FindBin(xOnClick, yOnClick);

    auto currentEventID = fRun -> GetCurrentEventID();
    auto lastEventID = fRun -> GetNumEvents() - 1;

    if (selectedBin==fBinCtrlFrst) { lk_info << "First event" << endl; fRun -> ExecuteFirstEvent(); }
    if (selectedBin==fBinCtrlPr50) { lk_info << "Event +50"   << endl; fRun -> ExecuteEvent((currentEventID-50<0?0:currentEventID-50)); }
    if (selectedBin==fBinCtrlPrev) { lk_info << "Prev. event" << endl; fRun -> ExecutePreviousEvent(); }
    if (selectedBin==fBinCtrlNext) { lk_info << "Next event"  << endl; fRun -> ExecuteNextEvent(); }
    if (selectedBin==fBinCtrlNe50) { lk_info << "Event -50"   << endl; fRun -> ExecuteEvent((currentEventID+50>lastEventID?lastEventID:currentEventID+50)); }
    if (selectedBin==fBinCtrlLast) { lk_info << "Last event"  << endl; fRun -> ExecuteLastEvent(); }

    Draw();
}

void ATMicromegas::ClickedCtrlEv2(double xOnClick, double yOnClick)
{
    if (fHistCtrlEv2==nullptr)
        return;

    int selectedBin = fHistCtrlEv2 -> FindBin(xOnClick, yOnClick);

    Long64_t currentEventID;
    Long64_t lastEventID;

    if (selectedBin==fBinCtrlNEEL500 || selectedBin==fBinCtrlNEEL203)
    {
        if (fRun==nullptr)
            return;
        auto currentEventID = fRun -> GetCurrentEventID();
        auto lastEventID = fRun -> GetNumEvents() - 1;

        double energyCut = 500;
        if (selectedBin==fBinCtrlNEEL500) energyCut = 500;
        else if (selectedBin==fBinCtrlNEEL203) energyCut = 2000;

        if (fRawDataArray==nullptr)
            return;

        auto testEventID = currentEventID;
        while (currentEventID<=lastEventID+1)
        {
            testEventID++;
            lk_info << "Testing " << testEventID << endl;

            //fRun -> ExecuteNextEvent();
            fRun -> GetEvent(testEventID);

            double maxEnergy = 0;
            auto numChannels = fRawDataArray -> GetEntries();
            for (auto iRawData=0; iRawData<numChannels; ++iRawData)
            {
                auto channel = (GETChannel*) fRawDataArray -> At(iRawData);
                if (channel->GetEnergy()>maxEnergy) {
                    maxEnergy = channel->GetEnergy();
                    break;
                }
            }
            if (maxEnergy>energyCut)
                break;

            if (testEventID==lastEventID)
                break;
        }
        if (testEventID==lastEventID) {
            lk_error << "No event with energy " << energyCut << endl;
            return;
        }

        fRun -> ExecuteEvent(testEventID);
        lk_info << "Event with energy " << energyCut << " : " << currentEventID << endl;
    }
    if (selectedBin==fBinCtrlEngyMax) {
        if (fEnergyMaxMode==0) {
            fEnergyMaxMode = 1;
            fHistCtrlEv2 -> SetBinContent(fBinCtrlEngyMax, 2500);
            lk_info << "Set energy range automatic" << endl;
        }
        else if (fEnergyMaxMode==1) {
            fEnergyMaxMode = 2;
            fHistCtrlEv2 -> SetBinContent(fBinCtrlEngyMax, 4200);
            lk_info << "Set energy range to 2500" << endl;
        }
        else //if (fEnergyMaxMode==2)
        {
            fEnergyMaxMode = 0;
            fHistCtrlEv2 -> SetBinContent(fBinCtrlEngyMax, 1);
            lk_info << "Set energy range to 4200" << endl;
        }
    }
    if (selectedBin==fBinCtrl4200Max) { return; }
    if (selectedBin==fBinCtrlAcmltCh)
    {
        if (fAccumulateChannel) {
            fCountChannelGraph = 0;
            fAccumulateChannel = false;
            fHistCtrlEv2 -> SetBinContent(fBinCtrlAcmltCh, 0);
        }
        else {
            fCountChannelGraph = 0;
            fAccumulateChannel = true;
            fHistCtrlEv2 -> SetBinContent(fBinCtrlAcmltCh, 1);
        }
        UpdateChannel();
        return;
    }
    if (selectedBin==fBinCtrlFitChan)
    {
        lk_info << "Fit channel" << endl;
        fFitChannel = true;
        UpdateChannel();
        return;
    }

    Draw();
}
