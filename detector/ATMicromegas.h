#ifndef ATMICROMEGAS_HH
#define ATMICROMEGAS_HH

#include "LKEvePlane.h"
#include "LKPhysicalPad.h"
#include "TPad.h"

class ATMicromegas : public LKEvePlane
{
    public:
        ATMicromegas();
        virtual ~ATMicromegas() {};

        virtual void Clear(Option_t *option = "");
        virtual void Print(Option_t *option = "") const;
        virtual bool Init();

        //virtual void FillDataToHist(Option_t *option=""); ///< Implementation recommanded. Fill data to histograms.
        //virtual void FillDataToHistEventDisplay1(Option_t *option="");
        virtual void FillDataToHistEventDisplay2(Option_t *option="");

    public:
        virtual TPad* Get3DEventPad();

        virtual TH2D* GetHistEventDisplay1(Option_t *option="-1");
        virtual TH2D* GetHistEventDisplay2(Option_t *option="-1");
        virtual TH1D* GetHistChannelBuffer();

        virtual int FindPadID(int cobo, int asad, int aget, int chan);
        virtual LKPhysicalPad* FindPad(int cobo, int asad, int aget, int chan);
        virtual int FindPadIDFromHistEventDisplay1Bin(int hbin);
        virtual int FindZFromHistEventDisplay2Bin(int hbin);

        virtual void ClickedEventDisplay2(double xOnClick, double yOnClick);
        virtual void UpdateEventDisplay2();
        virtual void UpdateChannelBuffer();

    private:
        bool fDefinePositionByPixelIndex = true;

        const int fNZ = 72;
        const int fNX = 64;
        int fNY = 512;
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
        int *fMapBin1ToPadID; ///< histogram bin to pad-id mapping
        int *fMapBin2ToIZ; ///< histogram bin to pad-id mapping

        int fCurrentView = 1;
        double fThreshold = 300;
        int fSelIZ = -1;

    ClassDef(ATMicromegas, 1)
};

#endif
