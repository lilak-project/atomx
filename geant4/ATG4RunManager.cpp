#include "ATG4RunManager.h"
#include "ATSteppingAction.h"
#include "TRandom.h"

ATG4RunManager::ATG4RunManager()
:LKG4RunManager()
{
}

ATG4RunManager::~ATG4RunManager()
{
}

void ATG4RunManager::Initialize()
{
    if (GetUserSteppingAction() == nullptr) SetUserAction(new ATSteppingAction(this));

    LKG4RunManager::Initialize();
}

void ATG4RunManager::NextEvent()
{
    LKG4RunManager::NextEvent();

    fRandomNumber = gRandom -> Uniform(0,350);
}
