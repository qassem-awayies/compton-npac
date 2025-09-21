#ifndef ACTIONINITIALIZATION_H
#define ACTIONINITIALIZATION_H

#include "G4VUserActionInitialization.hh"

class ActionInitialization : public G4VUserActionInitialization {
public:
    ActionInitialization();
    virtual ~ActionInitialization();
    virtual void Build() const override;
};

#endif

