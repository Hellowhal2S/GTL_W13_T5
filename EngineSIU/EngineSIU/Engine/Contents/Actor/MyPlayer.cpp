#include "MyPlayer.h"

#include "SnowBall.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"

AMyPlayer::AMyPlayer()
{
    DefaultSceneComponent = AddComponent<USceneComponent>(TEXT("DefaultSceneComponent"));
    SetRootComponent(DefaultSceneComponent);
    Camera = AddComponent<UCameraComponent>("Camera");
    Camera->SetupAttachment(DefaultSceneComponent);
}

void AMyPlayer::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);
    bool bExist = false;
    for (auto iter : GetWorld()->GetActiveLevel()->Actors)
    {
        if (Cast<ASnowBall>(iter))
        {
            bExist = true;
            Target = Cast<ASnowBall>(iter);
        }
    }
    if (bExist)
        SetActorLocation(Target->SnowBallComponent->GetRelativeLocation());
    UpdateCameraRotation();
    
    
}

void AMyPlayer::UpdateCameraRotation()
{
    // 1) 최초 호출 시에는 이전 좌표를 초기화
    if (!CursorInit)
    {
        // 현재 마우스 커서의 절대 좌표(스크린 좌표) 가져오기
        GetCursorPos(&PrevCursorPos);
        CursorInit = true;
        return;
    }

    POINT curPos;
    GetCursorPos(&curPos);

    int deltaX = curPos.x - PrevCursorPos.x;

    Yaw += deltaX * Sensitivity;

    PrevCursorPos = curPos;

    SetActorRotation(FRotator(GetActorRotation().Pitch, Yaw, GetActorRotation().Roll));
}
