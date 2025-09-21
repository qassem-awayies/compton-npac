// NaISD.cc
#include "NaISD.hh"
#include "G4Step.hh"
#include "G4Track.hh"

NaISD::NaISD(const G4String& name) : G4VSensitiveDetector(name), fEdep(0.) {}
NaISD::~NaISD() {}

G4bool NaISD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    fEdep += step->GetTotalEnergyDeposit();
    return true;
}

