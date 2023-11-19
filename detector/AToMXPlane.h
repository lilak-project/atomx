#ifndef ATOMXPLANE_HH
#define ATOMXPLANE_HH

#include "LKDetectorPlane.h"
#include "LKLogger.h"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK detector plane class
 *
 * # Given members in LKDetectorPlane class
 *
 * ## public:
 * - void SetDetector(LKDetector *detector);
 * - void SetPlaneID(Int_t id);
 * - Int_t GetPlaneID() const;
 * - void AddChannel(LKChannel *channel);
 * - LKChannel *GetChannelFast(Int_t idx);
 * - LKChannel *GetChannel(Int_t idx);
 * - Int_t GetNChannels();
 * - TObjArray *GetChannelArray();
 * - LKVector3::Axis GetAxis1();
 * - LKVector3::Axis GetAxis2();
 *
 * ## protected:
 * - TObjArray *fChannelArray = nullptr;
 * - Int_t fPlaneID = -1;
 * - TCanvas *fCanvas = nullptr;
 * - TH2 *fH2Plane = nullptr;
 * - LKVector3::Axis fAxis1 = LKVector3::kX;
 * - LKVector3::Axis fAxis2 = LKVector3::kY;
 * - LKDetector *fDetector = nullptr;
 *
 */

class AToMXPlane : public LKDetectorPlane
{
    public:
        AToMXPlane();
        virtual ~AToMXPlane() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        bool IsInBoundary(Double_t x, Double_t y);
        Int_t FindChannelID(Double_t x, Double_t y);
        Int_t FindChannelID(Int_t section, Int_t row, Int_t layer);

        //TCanvas* GetCanvas(Option_t *option="");
        TH2* GetHist(Option_t *option="");

        //bool SetDataFromBranch();
        void DrawFrame(Option_t *option="");
        //void Draw(Option_t *option="");

        //void MouseClickEvent(int iPlane);
        //void ClickedAtPosition(Double_t x, Double_t y);
        //TVector3 GlobalToLocalAxis(TVector3 xGlobal);
        //TVector3 LocalToGlobalAxis(TVector3 xLocal);

    ClassDef(AToMXPlane,1);
};

#endif
