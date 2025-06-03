#include "SnowBall.h"

#include "PhysicsManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/GameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASnowBall::ASnowBall()
{
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Apple/apple_mid.obj"));
    SetRootComponent(SnowBallComponent);
}

void ASnowBall::BeginPlay()
{
    AActor::BeginPlay();
    SpawnLocation = GetActorLocation();
    // InitialScale 초기화를 첫 번째 Tick으로 지연
    bIsRespawned = true; // 스케일 초기화 플래그로 활용
}

void ASnowBall::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    SnowBallComponent = GetComponentByClass<UStaticMeshComponent>();
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    // 첫 번째 Tick에서 실제 스케일 값을 가져옴
    if (bIsRespawned && SnowBallComponent)
    {
        InitialScale = SnowBallComponent->GetComponentScale3D(); // 월드 스케일 사용
        TArray<PxShape*> Shapes;
        Shapes.SetNum(1);
        PxU32 RetrievedShapes = SnowBallComponent->BodyInstance->BIGameObject->DynamicRigidBody->getShapes(Shapes.GetData(), 1);
        InitialRadius = Shapes[0]->getGeometry().sphere().radius;
        InitialMass = SnowBallComponent->BodyInstance->MassInKg; // 초기 질량 저장
        //UE_LOG(ELogLevel::Error, "Initial Scale: %.2f", InitialScale.X);
        bIsRespawned = false;
    }

    if (!Engine->PIEWorld->GetGameMode()->bGameOver && GetActorLocation().Z < 0.f)
    {
        if (Engine)
        {
            Engine->PIEWorld->GetGameMode()->Life--;
            if (Engine->PIEWorld->GetGameMode()->Life > 0)
            {
                Engine->PhysicsManager->DestroyGameObject(SnowBallComponent->BodyInstance->BIGameObject);
                SetActorLocation(SpawnLocation);
                SnowBallComponent->SetWorldScale3D(InitialScale); // 월드 스케일로 설정
                SnowBallComponent->CreatePhysXGameObject();
                bIsRespawned = true; // 리스폰 시 스케일 재초기화
            }
        }
    }
}

