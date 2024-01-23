#ifndef ATG4RUNMANAGER_HH
#define ATG4RUNMANAGER_HH

#include "LKG4RunManager.h"

class ATG4RunManager : public LKG4RunManager
{
    public:
        ATG4RunManager();
        virtual ~ATG4RunManager();

        virtual void Initialize();
        virtual void NextEvent();

        G4double fRandomNumber;
};

#endif
