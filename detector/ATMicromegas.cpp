#include "ATMicromegas.h"

ClassImp(ATMicromegas)


ATMicromegas::ATMicromegas()
    :ATMicromegas("ATMicromegas","AToM-X Micromegas")
{
}

ATMicromegas::ATMicromegas(const char *name, const char *title)
    :LKMicromegas(name, title)
{
    fDetName = "atomx";
}
