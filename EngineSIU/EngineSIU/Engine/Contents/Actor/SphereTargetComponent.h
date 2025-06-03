#pragma once
#include "Components/SphereComponent.h"

class USphereTargetComponent : public USphereComponent
{
    DECLARE_CLASS(USphereTargetComponent, USphereComponent)
public:
    USphereTargetComponent();
    virtual ~USphereTargetComponent() = default;
};
