#pragma once

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class DetectorConstruction;

class DetectorMessenger : public G4UImessenger {
public:
    DetectorMessenger(DetectorConstruction* det);
    ~DetectorMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorConstruction* fDetector;
    G4UIcmdWithADoubleAndUnit* fDet2AngleCmd;
};
