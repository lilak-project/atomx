#include "ATDetectorConstruction.h"
#include "LKLogger.h"

ATDetectorConstruction::ATDetectorConstruction()
{
    ;
}

G4VPhysicalVolume* ATDetectorConstruction::Construct()
{
    auto runManager = (LKG4RunManager *) G4RunManager::GetRunManager();
    auto par = runManager -> GetParameterContainer();

    G4double labTemperature = 293.15 * kelvin;
    G4double densityGas = 0.00012 * g/cm3;
    G4ThreeVector tpcSize(350*mm, 200*mm, 350*mm);
    if (par -> CheckPar("atomx/gasDensity"))
        densityGas = par -> GetParDouble("atomx/gasDensity") * g/cm3;
    if (par -> CheckPar("atomx/tpcSize")) {
        tpcSize.setX(par->GetParDouble("atomx/tpcSize",0)*mm);
        tpcSize.setY(par->GetParDouble("atomx/tpcSize",1)*mm);
        tpcSize.setZ(par->GetParDouble("atomx/tpcSize",2)*mm);
    }
    G4double tpcZOffset = 0.5 * tpcSize.z();

    G4NistManager *nist = G4NistManager::Instance();
    G4Element* elementC = nist -> FindOrBuildElement("C", false);
    G4Element* elementO = nist -> FindOrBuildElement("O" , false);
    G4Material *matGas = new G4Material("matCO2 ", densityGas, 2, kStateGas, labTemperature);
    matGas -> AddElement(elementC, 1);
    matGas -> AddElement(elementO, 2);
    G4Material *matVacuum = nist -> FindOrBuildMaterial("G4_Galactic");

    auto solidWorld = new G4Box("World", 1*m, 1*m, 1*m);
    auto logicWorld = new G4LogicalVolume(solidWorld, matVacuum, "World");
    auto physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, -1, true);

    auto solidDetector = new G4Box("atomx", 0.5*tpcSize.x(), 0.5*tpcSize.y(), 0.5*tpcSize.z());
    auto logicDetector = new G4LogicalVolume(solidDetector, matGas, "atomx");
    logicDetector -> SetUserLimits(new G4UserLimits(1.*mm));
    auto pvp = new G4PVPlacement(0, G4ThreeVector(0,0,tpcZOffset), logicDetector, "atomx", logicWorld, false, 0, true);

    // Register to LKG4RunManager
    runManager -> SetSensitiveDetector(pvp);

    return physWorld;
}

