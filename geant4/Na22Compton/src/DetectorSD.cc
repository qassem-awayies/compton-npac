// DetectorSD.cc - Fixed to match existing header interface
#include "DetectorSD.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"
#include "G4ios.hh"

DetectorSD::DetectorSD(const G4String& name) 
 : G4VSensitiveDetector(name),
   fTotalEdep(0.),
   fEnergyResolution(0.08) // 8% energy resolution for NaI
{}

G4bool DetectorSD::ProcessHits(G4Step* step, G4TouchableHistory*) 
{
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false;
    
    // Accumulate total energy
    fTotalEdep += edep;
    return true;
}

G4double DetectorSD::GetTotalEnergyWithResolution() const
{
  if (fTotalEdep <= 0.) return 0.;
  
  // Apply Gaussian energy resolution: σ/E = fEnergyResolution
  // For NaI, resolution typically follows: σ/E = a/√E + b
  // Simplified version: constant fractional resolution
  G4double sigma = fEnergyResolution * fTotalEdep;
  G4double measuredEnergy = G4RandGauss::shoot(fTotalEdep, sigma);
  
  // Ensure non-negative energy
  return std::max(measuredEnergy, 0.);
}

void DetectorSD::Clear() 
{
    fTotalEdep = 0.;
}
