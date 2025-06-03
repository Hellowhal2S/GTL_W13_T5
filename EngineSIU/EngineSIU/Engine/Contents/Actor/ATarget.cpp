#include "ATarget.h"

#include "PhysicsManager.h"
#include "SnowBall.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "SphereTargetComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "GameFramework/GameMode.h"
#include "World/World.h"
#include "Engine/AssetManager.h"

ATarget::ATarget()
{
    Target = AddComponent<UStaticMeshComponent>(TEXT("Target"));
    Target->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/Penguin.obj"));
    Target->bSimulate = true;
    SetRootComponent(Target);

    /*Target = AddComponent<USkeletalMeshComponent>(TEXT("Target"));
    Target->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/SkeletalPenguin/SkeletalPenguin"));
    Target->bSimulate = true;
    SetRootComponent(Target);*/

    SphereComponent = AddComponent<USphereTargetComponent>(TEXT("Collision"));
    SphereComponent->SetupAttachment(Target);
}

ATarget::~ATarget()
{
}

void ATarget::BeginPlay()
{
    AActor::BeginPlay();
    
    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.AddLambda(
[](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
        {
            if (Cast<ASnowBall>(OtherActor) != nullptr)
            {
                UE_LOG(ELogLevel::Error, TEXT("Penguin Killed"));
            }
        }
        );
    
}

void ATarget::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    if (bDead)
    {
        AccTime += DeltaTime;
        if (AccTime >2.0f)
        {
            UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
            GetComponentByClass<UStaticMeshComponent>()->bSimulate = false;
            EditorEngine->PhysicsManager->DestroyGameObject(GetComponentByClass<UStaticMeshComponent>()->BodyInstance->BIGameObject);
            SetActorLocation(FVector(-200,-200,-200));
            // GetComponentByClass<UStaticMeshComponent>()->CreatePhysXGameObject();
            if (GEngine->ActiveWorld->GetGameMode())
                GEngine->ActiveWorld->GetGameMode()->Score+= 100;
            bDead = false;

        }
    }
}
