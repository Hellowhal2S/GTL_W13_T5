#include "DeerCanon.h"

#include "Actors/FireballActor.h"
#include "Engine/Engine.h"
#include "Engine/FObjLoader.h"
#include "Engine/Contents/ObstacleFireball.h"
#include "World/World.h"
#include "Engine/Contents/Actor/SphereTargetComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/FObjLoader.h"
#include "Particles/ParticleSystem.h"

ADeerCanon::ADeerCanon()
{
    Deer = AddComponent<UStaticMeshComponent>("DeerCanon");
    Deer->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/deer.obj"));

    SpawnPoint = AddComponent<USceneComponent>("SpawnPoint");
    SpawnPoint->SetupAttachment(Deer);
    SpawnPoint->SetRelativeLocation(FVector(0, 413, 617));
    SetActorScale(FVector(500.0f, 500.0, 500.0f));

    auto* ParticleSystemComp = AddComponent<UParticleSystemComponent>("SauronParticleComponent");
    ParticleSystemComp->SetupAttachment(SpawnPoint);
    ParticleSystemComp->SetRelativeScale3D(FVector::OneVector * 0.001f);
    auto FireballParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    FName FullPath = TEXT("Contents/ParticleSystem/UParticleSystem_1411");
    FireballParticleSystem = UAssetManager::Get().GetParticleSystem(FullPath);

    ParticleSystemComp->SetParticleSystem(FireballParticleSystem);
}

ADeerCanon::~ADeerCanon()
{
}

void ADeerCanon::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    CurCoolTime += DeltaTime;
    if (CurCoolTime >= CoolTime)
    {
        CurCoolTime -= CoolTime;
        AObstacleFireball* FireBall = GEngine->ActiveWorld->SpawnActor<AObstacleFireball>();
        SpawnPoint = GetComponentByClass<USceneComponent>();
        FireBall->SetActorLocation(SpawnPoint->GetComponentLocation());
        FireBall->Fire(GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor()->GetActorLocation() - SpawnPoint->GetComponentLocation());
    }
}
