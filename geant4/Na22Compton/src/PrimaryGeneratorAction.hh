// PrimaryGeneratorAction.hh
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

class G4ParticleGun;
class G4Event;

/// The primary generator action class with particle gun.
///
/// The default kinematic is a gamma ray with 511 keV energy, 
/// randomly distributed in front of the phantom across 80% of the 
/// transverse (X,Y) phantom size.

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();

  // method from the base class
  virtual void GeneratePrimaries(G4Event*);

  // method to access particle gun
  const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

private:
  G4ParticleGun* fParticleGun; // pointer a to G4 gun class
  
  // Helper methods for Na-22 decay simulation
  void GenerateAnnihilationPhotons(const G4ThreeVector& pos, G4Event* anEvent);
  void Generate1274keVPhoton(const G4ThreeVector& pos, G4Event* anEvent);
};

#endif
