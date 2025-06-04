#include "ObstacleFireball.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/FObjLoader.h"
#include "Particles/ParticleSystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Contents/Actor/SphereTargetComponent.h"

#include "ExplosionParticleActor.h"
#include "PhysicsManager.h"

#include "Engine/EditorEngine.h"
#include "Engine/Contents/Actor/SnowBall.h"
AObstacleFireball::AObstacleFireball() : OvelapHandler()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Obstacle/FireBall.obj"));
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>("ParticleSystemComponent_0");
    ParticleSystemComponent->AttachToComponent(RootComponent);
    InitializeParticle();

    Collision = AddComponent<USphereTargetComponent>("SphereCollision");
    Collision->SetupAttachment(StaticMeshComponent);
    Collision->SetRadius(5.0f);
    
    SpeedGenerator.MaxValue = 1200;
    SpeedGenerator.MinValue = 800;

}

AObstacleFireball::~AObstacleFireball()
{
}

void AObstacleFireball::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

void AObstacleFireball::BeginPlay()
{
    Super::BeginPlay();
    Speed = SpeedGenerator.GetValue();
    //Fire(GetActorForwardVector());
    OvelapHandler =  GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.AddLambda(
    [this](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
    {
        if (Cast<ASnowBall>(OtherActor) != nullptr)
        {
            GEngine->PhysicsManager->ApplyForceToActor(OtherActor, this->GetActorForwardVector() * 5000000, 0);
            float Delta = -5;
            GEngine->PhysicsManager->GrowBall(OtherActor, Delta);
            Cast<ASnowBall>(OtherActor)->SnowBallComponent->AddScale(FVector(Delta));
            Cast<ASnowBall>(OtherActor)->GetComponentByClass<USphereTargetComponent>()->SetRadius(Cast<ASnowBall>(OtherActor)->GetComponentByClass<USphereTargetComponent>()->GetRadius() + (Delta)*1.2);
            this->CreateExplosion();
        }
        }
    );
    OvelapEndHandler = GetComponentByClass<USphereTargetComponent>()->OnComponentEndOverlap.AddLambda(
        [](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex)
        {
        }
    );
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

    if (AccumulatedTime >= 10.0f) // Fireball lifetime
    {
        bIsFired = false;

        if (!bIsCreatedExplosion)
        {
            CreateExplosion();
        }
    }

    if (bIsCreatedExplosion)
    {
        DestroyTime += DeltaTime;
    }

    if (DestroyTime >= 4.0f)
    {
        Destroy();
    }
    ///////////////
}

void AObstacleFireball::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.Remove(OvelapHandler);
    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.Remove(OvelapEndHandler);

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
    SetActorLocation(FVector::DownVector * 10000.0f);
    bIsCreatedExplosion = true;
}
