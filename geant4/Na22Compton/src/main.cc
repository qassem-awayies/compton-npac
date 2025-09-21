#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "DetectorMessenger.hh"
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

// Include the physics list header
#include "G4VModularPhysicsList.hh"
#include "FTFP_BERT.hh"   // standard physics list
#include "G4EmStandardPhysics_option4.hh"  // optional EM physics if needed

int main(int argc, char** argv) {
    auto runManager = new G4RunManager();

    // Detector
    runManager->SetUserInitialization(new DetectorConstruction());
    auto det = new DetectorConstruction();
    (void) new DetectorMessenger(det);
    runManager->SetUserInitialization(det);

    

    // Physics
    G4VModularPhysicsList* physicsList = new FTFP_BERT();
    physicsList->RegisterPhysics(new G4EmStandardPhysics_option4());
    runManager->SetUserInitialization(physicsList);

    // User actions
    runManager->SetUserInitialization(new ActionInitialization());

    // Initialize
    runManager->Initialize();

    auto visManager = new G4VisExecutive();
    visManager->Initialize();

    auto ui = new G4UIExecutive(argc, argv);
    G4UImanager::GetUIpointer()->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();

    delete ui;
    delete visManager;
    delete runManager;
}

