#ifndef DetectorMessenger_h
#define DetectorMessenger_h

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIdirectory.hh"

class DetectorConstruction;

class DetectorMessenger : public G4UImessenger
{
public:
    DetectorMessenger(DetectorConstruction* det);
    ~DetectorMessenger() override;

    virtual void SetNewValue(G4UIcommand* command, G4String newValue);

private:
    DetectorConstruction* fDetector;

    G4UIdirectory* fDetDir;
    G4UIcmdWithADoubleAndUnit* fDet2AngleCmd;
};

#endif

