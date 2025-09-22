#pragma once

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "globals.hh"

class DetectorSD : public G4VSensitiveDetector {
public:
    DetectorSD(const G4String& name);
    ~DetectorSD() override = default;

    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    void Clear();

    // Get total energy deposited (raw)
    G4double GetTotalEnergy() const { return fTotalEdep; }
    
    // Get energy with detector resolution applied
    G4double GetTotalEnergyWithResolution() const;
    
    // Set energy resolution (fractional, e.g., 0.08 for 8%)
    void SetEnergyResolution(G4double resolution) { fEnergyResolution = resolution; }

private:
    G4double fTotalEdep;
    G4double fEnergyResolution; // Energy resolution parameter
};
