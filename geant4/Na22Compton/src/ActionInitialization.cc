// ActionInitialization.cc - Updated to pass DetectorSD pointers to EventAction
#include "ActionInitialization.hh"
#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"

ActionInitialization::ActionInitialization(DetectorSD* det1, DetectorSD* det2)
 : fDet1(det1), fDet2(det2) {}

void ActionInitialization::Build() const {
    SetUserAction(new PrimaryGeneratorAction());
    SetUserAction(new EventAction(fDet1, fDet2));
}
