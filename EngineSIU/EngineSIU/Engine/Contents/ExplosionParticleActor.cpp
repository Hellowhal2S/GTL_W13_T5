#include "ExplosionParticleActor.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"

AExplosionParticleActor::AExplosionParticleActor()
{
    ParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    FName FullPath = TEXT("Contents/ParticleSystem/UParticleSystem_Explosion");
    ParticleSystem = UAssetManager::Get().GetParticleSystem(FullPath);
    
    ParticleSystemComponent->SetParticleSystem(ParticleSystem);
    ParticleSystemComponent->bOnceBurstOnBegin = true;
}

void AExplosionParticleActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    AccumTime += DeltaTime;
    if (AccumTime >= 4.0f)
    {
        Destroy();
    }
}

