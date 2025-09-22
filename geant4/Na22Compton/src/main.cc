#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "G4RunManager.hh"
#include "G4VModularPhysicsList.hh"
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"

int main(int argc, char** argv) {

    // --- Run manager ---
    auto runManager = new G4RunManager();

    // --- Detector construction ---
    auto det = new DetectorConstruction();
    runManager->SetUserInitialization(det);

    // --- Physics list ---
    G4VModularPhysicsList* physicsList = new FTFP_BERT();
    physicsList->RegisterPhysics(new G4EmStandardPhysics_option4());
    runManager->SetUserInitialization(physicsList);

    // Initialize - this calls ConstructSDandField()
    runManager->Initialize();

    // --- User actions (after initialization) ---
    runManager->SetUserInitialization(new ActionInitialization(det->GetDet1SD(), det->GetDet2SD()));

    // --- Visualization ---
    auto visManager = new G4VisExecutive();
    visManager->Initialize();

    auto ui = new G4UIExecutive(argc, argv);

    // --- Execute macro ---
    G4UImanager::GetUIpointer()->ApplyCommand("/control/execute run.mac");

    // --- Start UI session ---
    ui->SessionStart();

    // --- Cleanup ---
    delete ui;
    delete visManager;
    delete runManager;

    return 0;
}
