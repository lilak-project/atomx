#ifndef ATSTEPPINGACTION_HH
#define ATSTEPPINGACTION_HH

#define TEST_KILL_TRACK

//#define LKG4_DEBUG_STEPPINGACTION

#include "LKG4RunManager.h"
#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "globals.hh"

class ATSteppingAction : public G4UserSteppingAction
{
    public:
        ATSteppingAction();
        ATSteppingAction(LKG4RunManager *man);
        virtual ~ATSteppingAction() {}

        virtual void UserSteppingAction(const G4Step*);

    private:
        LKG4RunManager *fRunManager = nullptr;
};

#endif
