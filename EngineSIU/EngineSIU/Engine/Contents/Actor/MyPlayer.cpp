#include "MyPlayer.h"

#include "SnowBall.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"
#include "GameFramework/GameMode.h"
#include "Components/SphereTriggerComponent.h"
#include "Lua/LuaScriptManager.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"

AMyPlayer::AMyPlayer()
{
    DefaultSceneComponent = AddComponent<USceneComponent>(TEXT("DefaultSceneComponent"));
    SetRootComponent(DefaultSceneComponent);
    Camera = AddComponent<UCameraComponent>("Camera");
    Camera->SetupAttachment(DefaultSceneComponent);
    USphereTriggerComponent* SphereTrigger = AddComponent<USphereTriggerComponent>(TEXT("SphereTrigger"));
    SphereTrigger->bGenerateOverlapEvents = true;
    SphereTrigger->SetupAttachment(DefaultSceneComponent);
}

void AMyPlayer::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AMyPlayer, sol::bases<APlayer>(),

        )
}

void AMyPlayer::BeginPlay()
{
    APlayer::BeginPlay();

    RegisterLuaType(FLuaScriptManager::Get().GetLua());
    
    // PIE 모드에서만 마우스 커서 숨기기
    if (GEngine && GEngine->ActiveWorld && GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        ShowCursor(FALSE);
    }
}

void AMyPlayer::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
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
    
    // PIE 모드에서만 마우스 제어 처리
    if (GEngine && GEngine->ActiveWorld && GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        // 마우스 모드 전환 처리
        bool bCurrentAltState = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
        
        // Alt 키가 눌렸을 때 (이전에 안 눌려있었고 현재 눌려있음)
        if (!bPrevAltState && bCurrentAltState || Engine->PIEWorld->GetGameMode()->bGameOver)
        {
            bCameraMode = false;
            ShowCursor(TRUE);  // 마우스 커서 보이기
        }
        
        // 마우스 왼쪽 버튼 클릭 시 카메라 모드로 복귀
        if (!bCameraMode && (GetAsyncKeyState(VK_LBUTTON) & 0x8000) && !Engine->PIEWorld->GetGameMode()->bGameOver)
        {
            // 마우스 커서가 활성 뷰포트 영역 안에 있는지 확인
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            
            // 활성 뷰포트 클라이언트 가져오기
            if (auto* LevelEditor = GEngineLoop.GetLevelEditor())
            {
                if (auto ActiveViewport = LevelEditor->GetActiveViewportClient())
                {
                    FVector2D mousePos(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
                    
                    if (ActiveViewport->GetViewport()->bIsHovered(mousePos))
                    {
                        bCameraMode = true;
                        ShowCursor(FALSE); // 마우스 커서 숨기기
                        CursorInit = false; // 카메라 모드 재초기화
                    }
                }
            }
        }
        
        bPrevAltState = bCurrentAltState;
        
        // 카메라 모드일 때만 카메라 회전 업데이트
        if (bCameraMode)
        {
            UpdateCameraRotation();
        }
    }
}

void AMyPlayer::UpdateCameraRotation()
{
    // 화면 중앙 좌표 계산
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;
    
    // 1) 최초 호출 시에는 마우스를 화면 중앙으로 이동
    if (!CursorInit)
    {
        SetCursorPos(centerX, centerY);
        PrevCursorPos.x = centerX;
        PrevCursorPos.y = centerY;
        CursorInit = true;
        return;
    }

    POINT curPos;
    GetCursorPos(&curPos);

    int deltaX = curPos.x - PrevCursorPos.x;

    Yaw += deltaX * Sensitivity;

    // 5) 마우스 커서를 화면 중앙으로 다시 이동 (무한 회전을 위해)
    SetCursorPos(centerX, centerY);
    PrevCursorPos.x = centerX;
    PrevCursorPos.y = centerY;

    SetActorRotation(FRotator(GetActorRotation().Pitch, Yaw, GetActorRotation().Roll));
}
