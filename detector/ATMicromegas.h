#ifndef ATMICROMEGAS_HH
#define ATMICROMEGAS_HH

#include "LKDetectorPlane.h"
#include "LKPadInteractive.h"
#include "LKPhysicalPad.h"
#include "TPad.h"

class ATMicromegas : public LKDetectorPlane, public LKPadInteractive
{
    public:
        ATMicromegas();
        virtual ~ATMicromegas() {};

        virtual void Clear(Option_t *option = "");
        virtual void Print(Option_t *option = "") const;
        virtual bool Init();

        virtual void Draw(Option_t *option = ""); ///< Implementation recommanded for event display. Draw event display to the canvas.
        virtual TCanvas *GetCanvas(Option_t *option = ""); ///< Implementation recommanded for event display.
        virtual TH2* GetHist(Option_t *option = "-1");
        virtual void DrawFrame(Option_t *option = "") {}
        virtual void DrawHist(Option_t *option = "");
        virtual int GetNumCPads() { return 1; } ///< Get number of inner pads of canvas TODO
        virtual TPad *GetCPad(int iPad) { return (TPad*) GetCanvas(); } ///< For grabbing inner pads of canvas TODO

        virtual double PadDisplacement() const { return 5; } ///< Return average pad displacement to each other TODO
        virtual bool IsInBoundary(double i, double j) { return true; } ///< Implementation recommanded. Return if position (i,j) is inside the effective detector plane boundary TODO
        virtual TVector3 GlobalToLocalAxis(TVector3 posGlobal) { return TVector3(); } ///< Implementation recommanded. Convert global position to local detector plane position TODO
        virtual TVector3 LocalToGlobalAxis(TVector3 posLocal) { return TVector3(); } ///< Implementation recommanded. Convert local position to global detector plane position TODO

        virtual int FindChannelID(double i, double j) { return -1; }
        virtual int FindChannelID(int section, int layer, int row) { return -1; }
        virtual int FindPadID(int cobo, int asad, int aget, int chan);
        //virtual int FindPadID(double i, double j) { return FindChannelID(i,j); }
        //virtual int FindPadID(int section, int layer, int row) { return FindChannelID(section,layer,row); }

        virtual LKChannelAnalyzer* GetChannelAnalyzer(int id=0);

        LKPhysicalPad* FindPad(int cobo, int asad, int aget, int chan);
        virtual void DriftElectronBack(int padID, double tb, TVector3 &posReco, double &driftLength);

        virtual bool SetDataFromBranch(); ///< Implementation recommanded. Set waveform and hit data from input tree branches to pads
        virtual void FillDataToHist(Option_t *option = ""); ///< Implementation recommanded. Fill data to histograms.

        virtual TPad* Get3DEventPad() { return GetPad3DEvent(); }

        virtual void ExecMouseClickEventOnPad(TVirtualPad *pad, double xOnClick, double yOnClick);
        void ClickedCtrlEv1(double xOnClick, double yOnClick);
        void ClickedCtrlEv2(double xOnClick, double yOnClick);
        void Clicked2DEvent(double xOnClick, double yOnClick);
        TPad *GetPad2DEvent() { return fPad2DEvent; }
        TPad *GetPad3DEvent() { return fPad3DEvent; }
        TPad *GetPadChannel() { return fPadChannel; }
        TPad *GetPadCtrlEv1() { return fPadCtrlEv1; }
        TPad *GetPadCtrlEv2() { return fPadCtrlEv2; }

        void UpdateAll();
        void Update2DEvent();
        void UpdateChannel();
        void UpdateCtrlEv1();
        void UpdateCtrlEv2();

    private:
        const int fNZ = 72;
        const int fNX = 64;
        const double fDZPad = 4;
        const double fDXPad = 4;
        const double fDZActive = fNZ*fDZPad; // 288
        const double fDXActive = fNX*fDXPad; // 256
        const double fDZBoard = 388;
        const double fDXBoard = 356;

        TString fMappingFileName;
        int fNumCobo = 6;
        int fNumAsad = 4;
        int fNumAget = 4;
        int fNumChan = 68;

        int ****fMapCAACToPadID; ///< [cobo][asad][aget][chan] to pad-id mapping
        int *fMapPadIdxToBin; ///< pad-id to histogram bin mapping
        int *fMapBinToPadIdx; ///< histogram bin to pad-id mapping
        int **fMapZXToBin; ///< pixel(Z,X) to histogram bin mapping

        TClonesArray *fRawDataArray = nullptr;

        bool fDefinePositionByPixelIndex = true;

        TPad* fPad2DEvent = nullptr;
        TPad* fPad3DEvent = nullptr;
        TPad* fPadChannel = nullptr;
        TPad* fPadCtrlEv1 = nullptr;
        TPad* fPadCtrlEv2 = nullptr;

        TH2D* fHist2DEvent = nullptr;
        TH1D* fHistChannel = nullptr;
        TH2D* fHistCtrlEv1 = nullptr;
        TH2D* fHistCtrlEv2 = nullptr;

        TGraph* fGSel2DEvent = nullptr;

        int fBinCtrlEngyMax;
        int fBinCtrl4200Max;
        int fBinCtrlFitChan;
        int fBinCtrlAcmltCh;
        int fBinCtrlNEEL500; ///< Next Event with Energy Larger than > 500
        int fBinCtrlNEEL203; ///< Next Event with Energy Larger than > 2000

        int fBinCtrlFrst;
        int fBinCtrlPr50;
        int fBinCtrlPrev;
        int fBinCtrlNext;
        int fBinCtrlNe50;
        int fBinCtrlLast;

        int fSelPadIdx = 0;
        int fSelRawDataIdx = 0;

        int fEnergyMaxMode = 0;
        bool fFitChannel = false;
        bool fAccumulateChannel = false;
        TClonesArray *fChannelGraphArray = nullptr;
        int fCountChannelGraph = 0;
    /*
    protected:
        LKDetector *fDetector = nullptr;
        LKChannelAnalyzer* fChannelAnalyzer = nullptr;

        int fPlaneID = -1; ///< Detector plane id
        axis_t fAxis1 = LKVector3::kNon; ///< Axis-1 lying in plane. Must be set by input parameter.
        axis_t fAxis2 = LKVector3::kNon; ///< Axis-2 lying in plane. Must be set by input parameter.
        axis_t fAxis3 = LKVector3::kNon; ///< Axis-3 = Axis-1 x Axis-2 perpendicular to plane
        axis_t fAxisDrift = LKVector3::kNon; ///< Axis perpendicular to plane, in electron drifting direction. Must be set by input parameter.
        double fTbToLength = 1; ///< time-bin (tb) to position conversion factor. Must be set by input parameter.
        double fPosition; ///< Global position of pad plane center.

        TObjArray *fChannelArray = nullptr;
        TCanvas *fCanvas = nullptr;
        TH2 *fH2Plane = nullptr;
        TCanvas *fCvsChannelBuffer = nullptr;
        TGraph *fGraphChannelBoundary = nullptr;
        TGraph *fGraphChannelBoundaryNb[20] = {0};
        int fFreePadIdx = 0;

        bool fPadDataIsSet = false;
        bool fHitDataIsSet = false;
        */

    /*
    public:
        void SetDetector(LKDetector *detector) { fDetector = detector; }
        void SetPlaneID(int id) { fPlaneID = id; }
        int GetPlaneID() const { return fPlaneID; }
        axis_t GetAxis1() const { return fAxis1; }
        axis_t GetAxis2() const { return fAxis2; }
        axis_t GetAxis3() const { return fAxis3; }
        axis_t GetAxisDrift() const { return fAxisDrift; }
        double GetTbToLength() const { return fTbToLength; }
        double GetPosition() const { return fPosition; }

        virtual void DriftElectron(TVector3 posGlobal, TVector3 &posFinal, double &driftLength);
        virtual void DriftElectronBack(LKPad* pad, double tb, TVector3 &posReco, double &driftLength);
        virtual double DriftElectronBack(double tb);

        void AddChannel(LKChannel *channel) { fChannelArray -> Add(channel); } ///< Add channel to channel array in detector plane. For interanl use.
        void AddPad(LKPad *pad) { fChannelArray -> Add(pad); } ///< Add pad to pad array in detector plane. For interanl use.
        int GetNChannels() { return fChannelArray -> GetEntriesFast(); }
        int GetNumPads() { return GetNChannels(); }
        TObjArray *GetChannelArray() { return fChannelArray; }
        TObjArray *GetPadArray();

        LKChannel *GetChannelFast(int idx) { return (LKChannel *) fChannelArray -> At(idx); }
        LKPad *GetPadFast(int idx) { return (LKPad *) fChannelArray -> At(idx); }
        LKChannel *GetChannel(int idx);
        LKPad *GetPad(int padID);
        LKPad *GetPad(double i, double j);
        LKPad *GetPad(int section, int layer, int row);
        LKPad *GetPad(int cobo, int asad, int aget, int chan);

        void SetPadArray(TClonesArray *padArray);
        void SetHitArray(TClonesArray *hitArray);
        void FillBufferIn(double i, double j, double tb, double charge, int trackID = -1);
        void AddHit(LKHit *hit);

        virtual void ResetHitMap(); ///< For tracking use
        virtual void ResetEvent(); ///< For tracking use
        virtual LKHit *PullOutNextFreeHit(); ///< For tracking use
        void PullOutNeighborHits(vector<LKHit*> *hits, vector<LKHit*> *neighborHits); ///< For tracking use
        void PullOutNeighborHits(TVector2 p, int range, vector<LKHit*> *neighborHits); ///< For tracking use
        void PullOutNeighborHits(double x, double y, int range, vector<LKHit*> *neighborHits); ///< For tracking use
        void PullOutNeighborHits(double x, double y, int range, LKHitArray *neighborHits); ///< For tracking use
        void PullOutNeighborHits(LKHitArray *hits, LKHitArray *neighborHits); ///< For tracking use
        void GrabNeighborPads(vector<LKPad*> *pads, vector<LKPad*> *neighborPads); ///< For tracking use
        */

    ClassDef(ATMicromegas, 1)
};

#endif
