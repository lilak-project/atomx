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
}

LKChannelAnalyzer* ATMicromegas::GetChannelAnalyzer(int id)
{
    if (fChannelAnalyzer==nullptr)
    {
        fChannelAnalyzer = new LKChannelAnalyzer();
        //fChannelAnalyzer -> SetPulse(pulseFileName);
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
    fPar -> UpdatePar(fMappingFileName,"ATMicromegas/Mapping {lilak_common}/atomx_micromegas_mapping.txt # cobo asad aget chan x y");
    fPar -> UpdatePar(fDefinePositionByPixelIndex,"ATMicromegas/fDefinePositionByPixelIndex false");

    gStyle -> SetPalette(kRainbow);
    gStyle -> SetNumberContours(99);

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

    const int maxPads = fNZ*fNX;
    fMapPadIDToPadIdx = new int[maxPads];
    for (auto i=0; i<maxPads; ++i) fMapPadIDToPadIdx[i] = -1;

    ifstream fileCAACMap(fMappingFileName);
    if (!fileCAACMap.is_open()) {
        lk_error << "Cannot open " << fMappingFileName << endl;
        return false;
    }
    lk_info << "mapping: " << fMappingFileName << endl;

    int cobo, asad, aget, chan, iz1, ix1;
    while (fileCAACMap >> cobo >> asad >> aget >> chan >> iz1 >> ix1)
    {
        auto padID = (iz1-1) + (ix1-1)*fNZ;
        fMapCAACToPadID[cobo][asad][aget][chan] = padID;
        LKPhysicalPad* pad = new LKPhysicalPad();
        pad -> SetPadID(padID);
        pad -> SetPlaneID(0);
        pad -> SetCoboID(cobo);
        pad -> SetAsadID(asad);
        pad -> SetAgetID(aget);
        pad -> SetChannelID(chan);
        auto posz = (iz1-fNZ/2-0.5)*fDZPad;
        auto posx = (ix1-fNX/2-0.5)*fDXPad;
        if (fDefinePositionByPixelIndex) {
            posz = iz1-1;
            posx = ix1-1;
        }
        pad -> SetPosition(posz,posx);
        pad -> SetSection(0);
        pad -> SetLayer(iz1);
        pad -> SetRow(ix1);
        pad -> SetSortValue(10000-padID);
        auto padIdx = fChannelArray -> GetEntries();
        fMapPadIDToPadIdx[padID] = padIdx;
        fChannelArray -> Add(pad);
    }

    //fChannelArray -> Sort();
    auto numPads = fChannelArray -> GetEntries();
    lk_info << numPads << " pads are mapped!" << endl;


    if (numPads==fNZ*fNX)
        fAllChannelsAreMapped = true;

    GetCanvas();
    GetHist();

    if (fRun!=nullptr)
        fRawDataArray = fRun -> GetBranchA("RawData");

    return true;
}

void ATMicromegas::Draw(Option_t *option)
{
    GetCanvas();
    GetHist();
    SetDataFromBranch();
    FillDataToHist(option);
    UpdateAll();
}

void ATMicromegas::UpdateAll()
{
    Update2DEvent();
    UpdateChannel();
    UpdateControl();
}

void ATMicromegas::Update2DEvent()
{
    if (fHist2DEvent==nullptr) 
        return;
    fPad2DEvent -> cd();
    fPad2DEvent -> SetGrid();
    if (fFixEnergyMax) 
        fHist2DEvent -> SetMaximum(5000);
    else
        fHist2DEvent -> SetMaximum(-1111);
    fHist2DEvent -> Draw("colz");
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

    fGSel2DEvent -> Set(0);
    fGSel2DEvent -> SetPoint(0,z1,x1);
    fGSel2DEvent -> SetPoint(1,z1,x2);
    fGSel2DEvent -> SetPoint(2,z2,x2);
    fGSel2DEvent -> SetPoint(3,z2,x1);
    fGSel2DEvent -> SetPoint(4,z1,x1);
    fGSel2DEvent -> SetLineColor(kGray+1);

    fSelRawDataIdx = pad -> GetDataIndex();

    if (fRawDataArray!=nullptr&&fSelRawDataIdx>=0)
    {
        auto channel = (GETChannel*) fRawDataArray -> At(fSelRawDataIdx);
        channel -> FillHist(fHistChannel);
        fGSel2DEvent -> SetLineColor(kRed);

        fPad2DEvent -> cd();
        fGSel2DEvent -> Draw("samel");

        fPadChannel -> cd();
        if (fFixEnergyMax) 
            fHistChannel -> SetMaximum(5000);
        else
            fHistChannel -> SetMaximum(-1111);
        fHistChannel -> Draw("colz");

        if (fFitChannel)
        {
            fChannelAnalyzer -> Analyze(channel->GetWaveformY());
            auto numHits = fChannelAnalyzer -> GetNumHits();
            fPadChannel -> cd();
            auto graphPedestal = fChannelAnalyzer -> GetPedestalGraph();
            graphPedestal -> Draw("samel");
            for (auto iHit=0; iHit<numHits; ++iHit)
            {
                auto tbHit = fChannelAnalyzer -> GetTbHit(iHit);
                auto amplitude = fChannelAnalyzer -> GetAmplitude(iHit);
                auto pedestal = fChannelAnalyzer -> GetPedestal();
                auto graph = fChannelAnalyzer -> GetPulseGraph(tbHit,amplitude,pedestal);
                graph -> SetLineColor(kBlue-4);
                graph -> SetLineStyle(2);
                graph -> Draw("samel");
            }
        }
        fFitChannel = false;
    }
}

void ATMicromegas::UpdateControl()
{
    if (fHistControl==nullptr) 
        return;
    fPadControl -> cd();
    fPadControl -> SetGrid();
    auto currentEventID = fRun -> GetCurrentEventID();
    auto lastEventID = fRun -> GetNumEvents() - 1;
    fHistControl -> SetTitle(Form("%s (%lld)", fRun->GetInputFile()->GetName(), currentEventID));
    fHistControl -> SetBinContent(fBinCtrlPr50, (currentEventID-50<0?0:currentEventID-50));
    fHistControl -> SetBinContent(fBinCtrlPrev, (currentEventID==0?0:currentEventID-1));
    fHistControl -> SetBinContent(fBinCtrlNext, (currentEventID==lastEventID?lastEventID:currentEventID+1));
    fHistControl -> SetBinContent(fBinCtrlNe50, (currentEventID+50>lastEventID?lastEventID:currentEventID+50));
    fHistControl -> Draw("col text");
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
        fPadControl = new TPad("pad_control","",0.5,0,1,230./700);
        fPadControl -> SetMargin(0.12,0.05,0.20,0.12);
        fPadControl -> SetNumber(4);
        fPadControl -> Draw();
        fCanvas -> Modified();
        fCanvas -> Update();

        AddInteractivePad(fPad2DEvent);
        AddInteractivePad(fPadControl);
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
                auto padIdx = fMapPadIDToPadIdx[padID];
                fMapZXToBin[iz][ix] = bing;
                if (padIdx>=0)
                    fMapPadIdxToBin[padIdx] = bing;
                if (bing>=0)
                    fMapBinToPadIdx[bing] = padIdx;
            }
        }

        fHistChannel = new TH1D("ATMicromegas_Channel",";tb;y",512,0,512);
        fHistChannel -> SetStats(0);
        fHistChannel -> SetLineColor(kBlack);

        gStyle -> SetHistMinimumZero(); // this will draw text even when content is 0
        fHistControl = new TH2D("ATMicromegas_Control","",6,0,6,2,0,2);
        fHistControl -> SetStats(0);
        fBinCtrlFrst = fHistControl -> GetBin(1,1);
        fBinCtrlPr50 = fHistControl -> GetBin(2,1);
        fBinCtrlPrev = fHistControl -> GetBin(3,1);
        fBinCtrlNext = fHistControl -> GetBin(4,1);
        fBinCtrlNe50 = fHistControl -> GetBin(5,1);
        fBinCtrlLast = fHistControl -> GetBin(6,1);
        fBinCtrlNextE300 = fHistControl -> GetBin(4,2);
        fBinCtrlNextE500 = fHistControl -> GetBin(5,2);
        fBinCtrlNextE1000= fHistControl -> GetBin(6,2);
        fBinCtrlAutoMax = fHistControl -> GetBin(1,2);
        fBinCtrl5000Max = fHistControl -> GetBin(2,2);
        fBinCtrlFitChannel = fHistControl -> GetBin(3,2);
        fHistControl -> GetYaxis() -> SetTickSize(0);
        fHistControl -> GetYaxis() -> SetBinLabel(1,"");
        fHistControl -> GetYaxis() -> SetLabelSize(0.08);
        fHistControl -> GetXaxis() -> SetTickSize(0);
        fHistControl -> GetXaxis() -> SetBinLabel(1,"First");
        fHistControl -> GetXaxis() -> SetBinLabel(2,"-50");
        fHistControl -> GetXaxis() -> SetBinLabel(3,"Prev.");
        fHistControl -> GetXaxis() -> SetBinLabel(4,"Next");
        fHistControl -> GetXaxis() -> SetBinLabel(5,"+50");
        fHistControl -> GetXaxis() -> SetBinLabel(6,"Last");
        fHistControl -> GetXaxis() -> SetLabelSize(0.08);
        fHistControl -> SetBinContent(fBinCtrlFrst,0);
        //fHistControl -> SetBinContent(2,2);
        //fHistControl -> SetBinContent(3,3);
        fHistControl -> SetBinContent(fBinCtrlLast,fRun->GetNumEvents()-1);
        fHistControl -> SetBinContent(fBinCtrlAutoMax,   1);
        fHistControl -> SetBinContent(fBinCtrl5000Max,   5000);
        fHistControl -> SetBinContent(fBinCtrlFitChannel,999);
        fHistControl -> SetBinContent(fBinCtrlNextE300,  300);
        fHistControl -> SetBinContent(fBinCtrlNextE500,  500);
        fHistControl -> SetBinContent(fBinCtrlNextE1000, 1000);
        fHistControl -> SetMarkerSize(2.0);
        //fHistControl -> SetMaximum(4);
        fHistControl -> SetMinimum(0);
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
            lk_error << "Pad doesn't exist! CAAC= " << cobo << " " << asad << " " << aget << " " << chan << endl;
            continue;
        }
        pad -> SetTime(channel->GetTime());
        pad -> SetEnergy(channel->GetEnergy());
        pad -> SetPedestal(channel->GetPedestal());
        pad -> SetDataIndex(iRawData);

        if (channel->GetEnergy()>selEnergy) {
            auto padID = fMapCAACToPadID[cobo][asad][aget][chan];
            auto padIdx = fMapPadIDToPadIdx[padID];
            fSelPadIdx = padIdx;
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
    if (fAllChannelsAreMapped) {
        pad = (LKPhysicalPad*) fChannelArray -> At(padID);
        return pad;
    }
    auto padIdx = fMapPadIDToPadIdx[padID];
    if (padIdx>=0) {
        pad = (LKPhysicalPad*) fChannelArray -> At(padIdx);
        return pad;
    }
    /*
    TIter next(fChannelArray);
    while (pad = (LKPhysicalPad*) next()) {
        if (pad->GetPadID()==padID)
            return pad;
    }
    */
    return (LKPhysicalPad*) nullptr;
}

void ATMicromegas::FillDataToHist(Option_t* option)
{
    auto hist = GetHist();
    hist -> Reset();

    TString optionString = TString(option);
    TIter next(fChannelArray);
    LKPhysicalPad *pad = nullptr;
    TString title;

    if (optionString.Index("caac")>=0) {
        lk_info << "Filling caac to plane" << endl;
        title = ("caac");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetCAAC());
    }
    else if (optionString.Index("cobo")>=0) {
        lk_info << "Filling cobo to plane" << endl;
        title = ("cobo");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetCoboID());
    }
    else if (optionString.Index("asad")>=0) {
        lk_info << "Filling asad to plane" << endl;
        title = ("asad");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetAsadID());
    }
    else if (optionString.Index("aget")>=0) {
        lk_info << "Filling aget to plane" << endl;
        title = ("aget");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetAgetID());
    }
    else if (optionString.Index("chan")>=0) {
        lk_info << "Filling chan to plane" << endl;
        title = ("chan");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetChannelID());
    }
    else if (optionString.Index("section")>=0) {
        lk_info << "Filling section to plane" << endl;
        title = ("section");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetSection());
    }
    else if (optionString.Index("row")>=0) {
        lk_info << "Filling row to plane" << endl;
        title = ("raw");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetRow());
    }
    else if (optionString.Index("layer")>=0) {
        lk_info << "Filling layer to plane" << endl;
        title = ("layer");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetLayer());
    }
    else if (optionString.Index("padid")>=0) {
        lk_info << "Filling pad id to plane" << endl;
        title = ("id");
        while ((pad = (LKPhysicalPad *) next()))
            fHist2DEvent -> Fill(pad->GetI(),pad->GetJ(),pad->GetPadID());
    }
    else if (optionString.Index("nhit")>=0) {
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
            hist -> Fill(i,j,energy);
        }
    }

    if (fRun!=nullptr)
        title = Form("Event %lld (%s)", fRun->GetCurrentEventID(), title.Data());
    hist -> SetTitle(title);
}

void ATMicromegas::ExecMouseClickEventOnPad(TVirtualPad *pad, double xOnClick, double yOnClick)
{
    if (pad==fPad2DEvent) Clicked2DEvent(xOnClick, yOnClick);
    if (pad==fPadControl) ClickedControl(xOnClick, yOnClick);
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

void ATMicromegas::ClickedControl(double xOnClick, double yOnClick)
{
    if (fHistControl==nullptr)
        return;

    int selectedBin = fHistControl -> FindBin(xOnClick, yOnClick);

    auto currentEventID = fRun -> GetCurrentEventID();
    auto lastEventID = fRun -> GetNumEvents() - 1;
    if (selectedBin==fBinCtrlFrst) { lk_info << "First event" << endl; fRun -> ExecuteFirstEvent(); }
    if (selectedBin==fBinCtrlPr50) { lk_info << "Event +50"   << endl; fRun -> ExecuteEvent((currentEventID-50<0?0:currentEventID-50)); }
    if (selectedBin==fBinCtrlPrev) { lk_info << "Prev. event" << endl; fRun -> ExecutePreviousEvent(); }
    if (selectedBin==fBinCtrlNext) { lk_info << "Next event"  << endl; fRun -> ExecuteNextEvent(); }
    if (selectedBin==fBinCtrlNe50) { lk_info << "Event -50"   << endl; fRun -> ExecuteEvent((currentEventID+50>lastEventID?lastEventID:currentEventID+50)); }
    if (selectedBin==fBinCtrlLast) { lk_info << "Last event"  << endl; fRun -> ExecuteLastEvent(); }
    if (selectedBin==fBinCtrlNextE300 || selectedBin==fBinCtrlNextE500 || selectedBin==fBinCtrlNextE1000)
    {
        double energyCut = 300;
        if      (selectedBin==fBinCtrlNextE300)  energyCut = 300;
        else if (selectedBin==fBinCtrlNextE500)  energyCut = 500;
        else if (selectedBin==fBinCtrlNextE1000) energyCut = 1000;
        if (fRawDataArray!=nullptr)
        {
            while (currentEventID<=lastEventID+1)
            {
                fRun -> ExecuteNextEvent();
                currentEventID = fRun -> GetCurrentEventID();

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

                if (currentEventID==lastEventID+1)
                    break;
            }
        }
        if (currentEventID==lastEventID) {
            lk_error << "No event with energy " << energyCut << endl;
            return;
        }
        lk_info << "Event with energy " << energyCut << " : " << currentEventID << endl;
    }
    if (selectedBin==fBinCtrlAutoMax) { fFixEnergyMax = false; lk_info << "Fix energy maximum to 5000" << endl; }
    if (selectedBin==fBinCtrl5000Max) { fFixEnergyMax = true;  lk_info << "Use anto energy maximum" << endl; }
    if (selectedBin==fBinCtrlFitChannel)
    {
        lk_info << "Fit channel" << endl;
        fFitChannel = true;
    }

    Draw();
}
