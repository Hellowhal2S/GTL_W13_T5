#include "ParticleActor.h"
#include "Particles/ParticleSystemComponent.h"

AParticleActor::AParticleActor()
{
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>("ParticleSystemComponent_1");

    RootComponent = ParticleSystemComponent;
}
