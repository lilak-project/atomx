#include "ATOMX_NPTOOL_DC.h"
#include "MaterialManager.hh"
#include "BeamReaction.hh"
#include "Decay.hh"

ATOMX_NPTOOL_DC::ATOMX_NPTOOL_DC()
{
}

G4VPhysicalVolume* ATOMX_NPTOOL_DC::Construct()
{
    auto runManager = (LKG4RunManager *) G4RunManager::GetRunManager();
    auto par = runManager -> GetParameterContainer();

    /////////////////////////////////////////////////////////
    // General parameters
    /////////////////////////////////////////////////////////
    auto dmWorld        = par -> InitPar(TVector3(2000,2000,2000),  "ATOMX_NPTOOL_DC/WorldSize      ?? # mm");
    auto dmChamber      = par -> InitPar(TVector3(1000,1000,1000),  "ATOMX_NPTOOL_DC/ChamberSize    ?? # mm");
    auto dmGasVolume    = par -> InitPar(TVector3(800,800,800),     "ATOMX_NPTOOL_DC/GasVolumeSize  ?? # mm");
    auto dmPadVolume    = par -> InitPar(TVector3(600,2,600),       "ATOMX_NPTOOL_DC/PadVolumeSize  ?? # mm");
    auto dmMMSVolume    = par -> InitPar(TVector3(600,220,600),     "ATOMX_NPTOOL_DC/MMSVolumeSize  ?? # mm");
    auto yPad           = par -> InitPar(350,                       "ATOMX_NPTOOL_DC/yPad           ?? # mm");
    auto mylarRadius    = par -> InitPar(35,                        "ATOMX_NPTOOL_DC/mylarRadius    ?? # mm");
    auto mylarThickness = par -> InitPar(0.007,                     "ATOMX_NPTOOL_DC/mylarThickness ?? # mm");
    auto pressure       = par -> InitPar(0.1,                       "ATOMX_NPTOOL_DC/Pressure    ?? # bar");
    auto temperature    = par -> InitPar(295,                       "ATOMX_NPTOOL_DC/Temperature ?? # kelvin");
    auto lfGasMatName   = par -> InitPar(std::vector<TString>{"He","CO2"}, "ATOMX_NPTOOL_DC/GasMaterial ?? # gas material name");
    auto lfGasFraction  = par -> InitPar(std::vector<double>{.97,.03},     "ATOMX_NPTOOL_DC/GasFraction ?? # gas material factction in must add upto 1");
    auto vReactionZ     = par -> InitPar(std::vector<double>{}, "ATOMX_NPTOOL_DC/ReactionZ   ?? # z range to limit the reaction in local-gas coordinate (mm)");
    auto stepLimitInGas = par -> InitPar(0.5,  "ATOMX_NPTOOL_DC/StepLimitInGas        ?? # mm");
    auto productionCutEl= par -> InitPar(1000, "ATOMX_NPTOOL_DC/ProductionCutElectron ?? # mm");

    dmWorld     = dmWorld * mm;
    dmChamber   = dmChamber * mm;
    dmGasVolume = dmGasVolume * mm;
    dmPadVolume = dmPadVolume * mm;
    dmMMSVolume = dmMMSVolume * mm;
    mylarRadius = mylarRadius * mm;
    mylarThickness = mylarThickness * mm;
    pressure    = pressure * bar;
    temperature = temperature * kelvin;
    stepLimitInGas  = stepLimitInGas * mm;
    productionCutEl = productionCutEl * mm;

    /////////////////////////////////////////////////////////
    // Reaction region parameters
    /////////////////////////////////////////////////////////
    bool limitReactionZ = false;
    double reactionZ1 = 0;
    double reactionZ2 = 0;
    double reactionZWidth = 0;
    if (vReactionZ.size()>0)
    {
        if (vReactionZ.size()!=2)
            e_error << "size of vReactionZ should be 2 but now it is " << vReactionZ.size() << endl;
        reactionZ1 = vReactionZ[0];
        reactionZ2 = vReactionZ[1];
        if (reactionZ2!=reactionZ1)
        {
            if (reactionZ2<reactionZ1) {
                reactionZ1 = vReactionZ[1];
                reactionZ2 = vReactionZ[0];
            }
            reactionZWidth = reactionZ2 - reactionZ1;
            limitReactionZ = true;
            e_info << "Limiting sensitive region : " << reactionZ1 << " -> " << reactionZ2 << endl;
        }
    }

    /////////////////////////////////////////////////////////
    // Physical volume
    /////////////////////////////////////////////////////////
    G4Box*  solidWorld     = new G4Box ("solidWorld",   dmWorld.x()     * 0.5, dmWorld.y()     * 0.5, dmWorld.z()     * 0.5);
    G4Box*  solidChamber   = new G4Box ("solidChamber", dmChamber.x()   * 0.5, dmChamber.y()   * 0.5, dmChamber.z()   * 0.5);
    G4Box*  solidGas       = new G4Box ("solidGas",     dmGasVolume.x() * 0.5, dmGasVolume.y() * 0.5, dmGasVolume.z() * 0.5);
    G4Box*  solidPad       = new G4Box ("solidPad",     dmPadVolume.x() * 0.5, dmPadVolume.y() * 0.5, dmPadVolume.z() * 0.5);
    G4Box*  solidMMS       = new G4Box ("solidMMS",     dmMMSVolume.x() * 0.5, dmMMSVolume.y() * 0.5, dmMMSVolume.z() * 0.5);
    G4Tubs* solidWindow    = new G4Tubs("solidWindow", 0, mylarRadius, mylarThickness * 0.5, 0 * deg, 360 * deg);
    G4Box*  solidReactionGas = nullptr;
    if (limitReactionZ)
        solidReactionGas = new G4Box("solidReactionGas", dmGasVolume.x() * 0.5, dmGasVolume.y() * 0.5, reactionZWidth * 0.5);

    /////////////////////////////////////////////////////////
    // Material
    /////////////////////////////////////////////////////////
    G4Material* Cu = MaterialManager::getInstance()->GetMaterialFromLibrary("Cu");
    G4Material* Al = MaterialManager::getInstance()->GetMaterialFromLibrary("Al");
    G4Material* Mylar = MaterialManager::getInstance()->GetMaterialFromLibrary("Mylar");
    //
    double density = 0;
    double density_sum = 0;
    vector<G4Material*> GasComponent;
    vector<double> FractionMass;
    int numberOfGasMix = lfGasMatName.size();
    if (lfGasMatName.size()>0&&lfGasFraction.size()>0)
    {
        numberOfGasMix = lfGasMatName.size();
        double total_fraction = 0;
        for (auto fraction : lfGasFraction) total_fraction += fraction;
        if (abs(1.-total_fraction)>0.01)
            e_error << "Sum of fraction of gas material should sum up to 1! (now=" << total_fraction << ")" << endl;
    }
    for (unsigned int i = 0; i < numberOfGasMix; i++) {
        GasComponent.push_back(MaterialManager::getInstance()->GetGasFromLibrary(lfGasMatName[i].Data(), pressure, temperature));
        density += lfGasFraction[i] * GasComponent[i]->GetDensity();
        density_sum += GasComponent[i]->GetDensity();
    }
    for (unsigned int i = 0; i < numberOfGasMix; i++) {
        FractionMass.push_back(GasComponent[i]->GetDensity() / density_sum);
    }
    G4Material* materialGas = new G4Material("GasMix", density, numberOfGasMix, kStateGas, temperature, pressure);
    e_info << "== Gas mixture:" << endl;
    for (unsigned int i = 0; i < numberOfGasMix; i++) {
        materialGas -> AddMaterial(GasComponent[i], FractionMass[i]);
        e_info << "   gas-" << i << " = " << lfGasMatName[i] << " (" << FractionMass[i] << ")" << endl;
    }
    e_info << "   density = " << density << "cm3/g" << endl;
    G4NistManager *nist = G4NistManager::Instance();
    G4Material *materialVacuum = nist -> FindOrBuildMaterial("G4_Galactic");

    /////////////////////////////////////////////////////////
    // Logical volume
    /////////////////////////////////////////////////////////
    auto logicWorld = new G4LogicalVolume(solidWorld, materialVacuum, "logicWorld", 0, 0, 0);
    auto logicChamber = new G4LogicalVolume(solidChamber, materialVacuum, "logicChamber", 0, 0, 0);
    auto logicGas = new G4LogicalVolume(solidGas, materialGas, "logicGas", 0, 0, 0);
    G4LogicalVolume* logicReactionGas = nullptr;
    if (limitReactionZ)
        logicReactionGas = new G4LogicalVolume(solidReactionGas, materialGas, "logicReactionGas", 0, 0, 0);
    auto logicPad = new G4LogicalVolume(solidPad, Cu, "logicPad", 0, 0, 0);
    auto logicMMS = new G4LogicalVolume(solidMMS, Al, "logicMMS", 0, 0, 0);
    auto logicWindow = new G4LogicalVolume(solidWindow, Mylar, "logicWindow", 0, 0, 0);

    /////////////////////////////////////////////////////////
    // Placement
    /////////////////////////////////////////////////////////
    G4PVPlacement *physicsWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "World", 0, false, 0, true);
    new G4PVPlacement(G4Transform3D(), logicChamber, "ATOMXChamber", logicWorld, false, 0);
    G4PVPlacement *physicsWindow = new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, -(0.5*dmGasVolume.z()+0.5*mylarThickness))), logicWindow, "ATOMXWindow", logicChamber, false, 0);
    runManager -> SetSensitiveDetector(physicsWindow);
    G4PVPlacement *physicsGas = new G4PVPlacement(G4Transform3D(), logicGas, "ATOMXGas", logicChamber, false, 0);
    runManager -> SetSensitiveDetector(physicsGas);
    G4PVPlacement *physicsReactionGas = nullptr;
    if (limitReactionZ) {
        G4Transform3D transformLimit(G4RotationMatrix(), G4ThreeVector(0, 0, -0.5*dmGasVolume.z() + 0.5*(reactionZ1+reactionZ2)));
        physicsReactionGas = new G4PVPlacement(transformLimit, logicReactionGas, "ATOMXReactionGas", logicGas, false, 0);
        runManager -> SetSensitiveDetector(physicsReactionGas);
    }
    new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, yPad, 0)), logicPad, "ATOMXPad", logicGas, false, 0);

    /////////////////////////////////////////////////////////
    // Region
    /////////////////////////////////////////////////////////
    G4ProductionCuts* ecut = new G4ProductionCuts();
    ecut -> SetProductionCut(productionCutEl,"e-");
    auto regionNPSimulation = new G4Region("NPSimulationProcess");
    regionNPSimulation -> SetProductionCuts(ecut);
    regionNPSimulation -> SetUserLimits(new G4UserLimits(stepLimitInGas));    
    if (limitReactionZ) {
        regionNPSimulation -> AddRootLogicalVolume(logicReactionGas);
        auto regionGas = new G4Region("NonReactionGas");
        regionGas -> SetProductionCuts(ecut);
        regionGas -> SetUserLimits(new G4UserLimits(stepLimitInGas));    
        regionGas -> AddRootLogicalVolume(logicGas);
    }
    else {
        regionNPSimulation -> AddRootLogicalVolume(logicGas);
        auto regionJustPad = new G4Region("PadRegion");
        regionJustPad -> AddRootLogicalVolume(logicPad);
    }

    G4FastSimulationManager* simulationManager = regionNPSimulation -> GetFastSimulationManager();
    vector<G4VFastSimulationModel*> listSimulationModel;
    listSimulationModel.push_back(new NPS::BeamReaction("BeamReaction", regionNPSimulation));
    listSimulationModel.push_back(new NPS::Decay("Decay", regionNPSimulation));

    /////////////////////////////////////////////////////////
    // Attributes
    /////////////////////////////////////////////////////////
    auto visWorld    = new G4VisAttributes(G4Colour::Grey());
    auto visChamber  = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.3));
    auto visWindow   = new G4VisAttributes(G4Colour(1, 0, 0, 0.25));
    auto visGas      = new G4VisAttributes(G4Colour(0, 0.5, 0.5, 0.3));
    auto visPads     = new G4VisAttributes(G4Colour(255, 223, 50, 0.8));
    auto visMMS      = new G4VisAttributes(G4Colour(100, 100, 100, 0.4));
    auto visReaction = new G4VisAttributes(G4Colour::Yellow());
    visWorld    -> SetForceWireframe(true);
    visChamber  -> SetForceWireframe(true);
    visWindow   -> SetForceWireframe(true);
    visGas      -> SetForceWireframe(true);
    visPads     -> SetForceWireframe(true);
    visMMS      -> SetForceWireframe(true);
    visReaction -> SetForceWireframe(true);

    logicWorld   -> SetVisAttributes(visWorld);
    logicChamber -> SetVisAttributes(visChamber);
    logicGas     -> SetVisAttributes(visGas);
    logicWindow  -> SetVisAttributes(visWindow);
    logicPad     -> SetVisAttributes(visPads);
    logicMMS     -> SetVisAttributes(visMMS);
    if (limitReactionZ)
        logicReactionGas -> SetVisAttributes(visReaction);
    
    return physicsWorld;
}
