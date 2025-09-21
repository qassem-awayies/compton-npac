#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::Build() const {
    // Primary generator action
    SetUserAction(new PrimaryGeneratorAction());

    // Event action
    SetUserAction(new EventAction());
}

