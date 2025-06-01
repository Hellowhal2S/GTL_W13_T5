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
    for (auto iter : GetWorld()->GetActiveLevel()->Actors)
    {
        if (Cast<ASnowBall>(iter))
        {
            Target = Cast<ASnowBall>(iter);
        }
    }

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

    // 2) 현재 마우스 좌표 얻기
    POINT curPos;
    GetCursorPos(&curPos);

    // 3) ΔX 계산 (왼쪽: 음수, 오른쪽: 양수)
    int deltaX = curPos.x - PrevCursorPos.x;

    // 4) Yaw 갱신 (감도 곱해서)
    Yaw += deltaX * Sensitivity;
    //    만약 도 단위로 쓰려면: g_fYaw += deltaX * (sensitivity_in_degrees);

    // 5) 필요하면 ΔY를 이용해 Pitch도 조정 가능
    // int deltaY = curPos.y - g_prevCursorPos.y;
    // g_fPitch += deltaY * someVerticalSensitivity;

    // 6) 이전 좌표 갱신
    PrevCursorPos = curPos;

    SetActorRotation(FRotator(GetActorRotation().Pitch, Yaw, GetActorRotation().Roll));
    // 7) g_fYaw를 기반으로 실제 카메라/오브젝트 회전 행렬 혹은 쿼터니언을 생성해서 적용
    //    예시 (DirectXMath 사용 가정):
    //
    //    XMMATRIX rotY = XMMatrixRotationY(g_fYaw);
    //    // 월드 행렬 혹은 뷰 행렬에 rotY를 곱해서 넘기면 됩니다.
    //
    //    // 또는 쿼터니언 형태로:
    //    XMVECTOR q = XMQuaternionRotationRollPitchYaw(0.0f, g_fYaw, 0.0f);
    //    XMMATRIX rotQuat = XMMatrixRotationQuaternion(q);
    //    // rotQuat로도 동일하게 사용 가능

    // (이후, 렌더러에 rotY / rotQuat 등을 넘겨서 실제 화면상 회전이 이루어지도록 처리)
}
