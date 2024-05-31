#ifndef ATMICROMEGAS_HH
#define ATMICROMEGAS_HH

#include "LKMicromegas.h"

class ATMicromegas : public LKMicromegas
{
    public:
        ATMicromegas();
        ATMicromegas(const char *name, const char *title);
        virtual ~ATMicromegas() {};

    ClassDef(ATMicromegas, 1)
};

#endif
