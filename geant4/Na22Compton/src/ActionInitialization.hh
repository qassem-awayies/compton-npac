#pragma once
#include "G4VUserActionInitialization.hh"
#include "DetectorSD.hh"

class ActionInitialization : public G4VUserActionInitialization {
public:
    ActionInitialization(DetectorSD* det1, DetectorSD* det2);
    ~ActionInitialization() override {}
    void Build() const override;

private:
    DetectorSD* fDet1;
    DetectorSD* fDet2;
};

