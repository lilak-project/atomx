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

    G4ThreeVector degraderSize(350*mm, 200*mm, 0.1*mm);
    if (par -> CheckPar("atomx/degraderThickness"))
        degraderSize.setZ(par->GetParDouble("atomx/degraderThickness")*mm);
    G4double degraderOffset = -0.5*degraderSize.z() -100*mm;

    G4NistManager *nist = G4NistManager::Instance();
    G4Element* elementC = nist -> FindOrBuildElement("C", false);
    G4Element* elementO = nist -> FindOrBuildElement("O" , false);
    G4Element* elementAl = nist -> FindOrBuildElement("Al" , false);
    G4Material* matAl = new G4Material("Aluminium", 2.702*g/cm3, 1);
    matAl -> AddElement(elementAl, 1.0);
    G4Material *matGas = new G4Material("matCO2 ", densityGas, 2, kStateGas, labTemperature);
    matGas -> AddElement(elementC, 1);
    matGas -> AddElement(elementO, 2);
    G4Material *matVacuum = nist -> FindOrBuildMaterial("G4_Galactic");

    G4int detectorNo = -1;

    auto solidWorld = new G4Box("World", 1*m, 1*m, 2*m);
    auto logicWorld = new G4LogicalVolume(solidWorld, matVacuum, "World");
    auto physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, detectorNo=0, true);

    auto solidAtomx = new G4Box("atomx", 0.5*tpcSize.x(), 0.5*tpcSize.y(), 0.5*tpcSize.z());
    auto logicAtomx = new G4LogicalVolume(solidAtomx, matGas, "atomx");
    logicAtomx -> SetUserLimits(new G4UserLimits(1.*mm));
    auto pvpAtomx = new G4PVPlacement(0, G4ThreeVector(0,0,tpcZOffset), logicAtomx, "atomx", logicWorld, false, detectorNo=1, true);

    auto solidDegrader = new G4Box("degrader", 0.5*degraderSize.x(), 0.5*degraderSize.y(), 0.5*degraderSize.z());
    auto logicDegrader = new G4LogicalVolume(solidDegrader, matAl, "degrader");
    auto pvpDegrader = new G4PVPlacement(0, G4ThreeVector(0,0,degraderOffset), logicDegrader, "degrader", logicWorld, false, detectorNo=2, true);

    // Register to LKG4RunManager
    runManager -> SetSensitiveDetector(pvpAtomx);

    return physWorld;
}
