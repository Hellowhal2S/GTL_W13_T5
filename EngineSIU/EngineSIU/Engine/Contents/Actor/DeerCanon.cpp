#include "DeerCanon.h"

#include "Actors/FireballActor.h"
#include "Engine/Engine.h"
#include "Engine/FObjLoader.h"
#include "Engine/Contents/ObstacleFireball.h"
#include "World/World.h"

ADeerCanon::ADeerCanon()
{
    Deer = AddComponent<UStaticMeshComponent>("DeerCanon");
    Deer->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/deer.obj"));

    SpawnPoint = AddComponent<USceneComponent>("SpawnPoint");
    SpawnPoint->SetupAttachment(Deer);
    SetActorScale(FVector(10.0f, 10.0f, 10.0f));
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
        FireBall->SetActorRotation(FRotator(GetActorRotation().Pitch, GetActorRotation().Yaw + 90, GetActorRotation().Roll));
        FireBall->SetActorScale(GetActorScale() * 0.3f);
    }
}
