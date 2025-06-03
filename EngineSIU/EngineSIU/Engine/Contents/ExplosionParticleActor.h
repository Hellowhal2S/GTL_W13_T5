#pragma once
#include "ParticleActor.h"

class UParticleSystem;

class AExplosionParticleActor : public AParticleActor
{
    DECLARE_CLASS(AExplosionParticleActor, AParticleActor);

public:
    AExplosionParticleActor();
    virtual ~AExplosionParticleActor() override = default;

    virtual void Tick(float DeltaTime) override;

public:
    UParticleSystem* ParticleSystem = nullptr;

private:
    float AccumTime = 0;
};

