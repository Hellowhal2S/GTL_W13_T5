#pragma once
#include "SphereComponent.h"
class USphereTriggerComponent :
    public USphereComponent
{
    DECLARE_CLASS(USphereTriggerComponent, USphereComponent)
public:

    USphereTriggerComponent();
    virtual ~USphereTriggerComponent() = default;

};

