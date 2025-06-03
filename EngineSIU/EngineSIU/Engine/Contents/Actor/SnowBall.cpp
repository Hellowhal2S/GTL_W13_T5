#include "SnowBall.h"

#include "ATarget.h"
#include "PhysicsManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/GameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "SphereTargetComponent.h"
#include "Engine/Contents/ObstacleFireball.h"

ASnowBall::ASnowBall()
{
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/FreshSnow/SnowBall.obj"));
    SetRootComponent(SnowBallComponent);
    
    SphereCollision = AddComponent<USphereTargetComponent>("Collision");
    SphereCollision->SetupAttachment(SnowBallComponent);
    // SphereCollision->SetRadius(5.0f);


}


void ASnowBall::BeginPlay()
{
    AActor::BeginPlay();
    SpawnLocation = GetActorLocation();
    // InitialScale 초기화를 첫 번째 Tick으로 지연
    bIsRespawned = true; // 스케일 초기화 플래그로 활용

    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.AddLambda(
[](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    if (Cast<ATarget>(OtherActor) != nullptr)
    {
        Cast<ATarget>(OtherActor)->bDead = true;
        UE_LOG(ELogLevel::Display,"Kill  Target");
    }
}
);
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
        GetComponentByClass<USphereTargetComponent>()->SetRadius(InitialRadius*1.5);
        InitialMass = SnowBallComponent->BodyInstance->MassInKg; // 초기 질량 저장
        //UE_LOG(ELogLevel::Error, "Initial Scale: %.2f", InitialScale.X);
        bIsRespawned = false;
    }

    if (!Engine->PIEWorld->GetGameMode()->bGameOver && GetActorLocation().Z < -200.f)
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
                GetComponentByClass<USphereTargetComponent>()->SetRadius(InitialRadius*1.5);
                bIsRespawned = true; // 리스폰 시 스케일 재초기화
            }
        }
    }
}

