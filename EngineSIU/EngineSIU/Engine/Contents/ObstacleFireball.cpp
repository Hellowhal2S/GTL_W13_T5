#include "ObstacleFireball.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/FObjLoader.h"
#include "Particles/ParticleSystem.h"
#include "Engine/AssetManager.h"
#include "UserInterface/Console.h"

#include "ExplosionParticleActor.h"

#include "Engine/EditorEngine.h"

AObstacleFireball::AObstacleFireball()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Obstacle/FireBall.obj"));
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>("ParticleSystemComponent_0");
    ParticleSystemComponent->AttachToComponent(RootComponent);
    InitializeParticle();

    auto* CollisionComp = AddComponent<USphereComponent>("SphereCollision");
    CollisionComp->AttachToComponent(RootComponent);
    CollisionComp->SetRadius(2.0f);

    SpeedGenerator.MaxValue = 100;
    SpeedGenerator.MinValue = 30;
}

void AObstacleFireball::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

void AObstacleFireball::BeginPlay()
{
    Super::BeginPlay();
    Speed = SpeedGenerator.GetValue();
    Fire(GetActorForwardVector());
}

void AObstacleFireball::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsFired)
    {
        FVector NewLocation = GetActorLocation() + Direction * DeltaTime * Speed; // Adjust speed as needed
        SetActorLocation(NewLocation);
    }

    // Test Code //
    AccumulatedTime += DeltaTime;
    if (AccumulatedTime >= 2.0f) // Fireball lifetime
    {
        bIsFired = false;

        if (!bIsCreatedExplosion)
        {
            CreateExplosion();
            bIsCreatedExplosion = true;
        }

        SetActorLocation(FVector::DownVector * 1000.0f);

        if (AccumulatedTime >= 4.0f)
        {
            Destroy();
        }
    }
    ///////////////
}

void AObstacleFireball::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (EndPlayReason == EEndPlayReason::Destroyed)
    {

    }
    else if (EndPlayReason == EEndPlayReason::WorldTransition)
    {
    }
    else if (EndPlayReason == EEndPlayReason::Quit)
    {
    }
}

void AObstacleFireball::InitializeParticle()
{
    FireballParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    FName FullPath = TEXT("Contents/ParticleSystem/UParticleSystem_Fireball");
    FireballParticleSystem = UAssetManager::Get().GetParticleSystem(FullPath);

    ParticleSystemComponent->SetParticleSystem(FireballParticleSystem);
}

void AObstacleFireball::Fire(FVector InDirection)
{
    bIsFired = true;
    Direction = InDirection.GetSafeNormal();
}

void AObstacleFireball::CreateExplosion()
{
    UWorld* World = GEngine->ActiveWorld;
    if (World && World->WorldType == EWorldType::PIE)
    {
        auto* ExplosionActor = World->SpawnActor<AExplosionParticleActor>();
        ExplosionActor->SetActorLabel(TEXT("Explosion Particle"));
        ExplosionActor->SetActorLocation(GetActorLocation());
    }
}
