// NaISD.hh
#ifndef NaISD_h
#define NaISD_h 1

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"

class NaISD : public G4VSensitiveDetector
{
public:
    NaISD(const G4String& name);
    virtual ~NaISD();
    
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    double GetTotalEnergyDeposit() const { return fEdep; }
    void Reset() { fEdep = 0.; }

private:
    double fEdep;
};

#endif

