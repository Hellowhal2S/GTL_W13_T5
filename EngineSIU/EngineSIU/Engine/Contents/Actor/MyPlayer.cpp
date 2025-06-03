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
#include "Math/JungleMath.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/FObjLoader.h"
#include "Particles/ParticleSystem.h"

FVector AMyPlayer::InitialVector = FVector::ZeroVector;
FRotator AMyPlayer::InitialRotator = FRotator::ZeroRotator; 

AMyPlayer::AMyPlayer()
{
    DefaultSceneComponent = AddComponent<USceneComponent>(TEXT("DefaultSceneComponent"));
    SetRootComponent(DefaultSceneComponent);
    Camera = AddComponent<UCameraComponent>("Camera");
    Camera->SetupAttachment(DefaultSceneComponent);

    ParticleSystemComp = AddComponent<UParticleSystemComponent>("SnowParticleSystemComponent");
    ParticleSystemComp->SetupAttachment(Camera);
    auto FireballParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    FName FullPath = TEXT("Contents/ParticleSystem/UParticleSystem_1159");
    FireballParticleSystem = UAssetManager::Get().GetParticleSystem(FullPath);

    ParticleSystemComp->SetParticleSystem(FireballParticleSystem);
}

void AMyPlayer::BeginPlay()
{
    APlayer::BeginPlay();
    
    // PIE 모드에서만 마우스 커서 숨기기
    // if (GEngine && GEngine->ActiveWorld && GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    // {
    //     ShowCursor(FALSE);
    // }
    //

}

void AMyPlayer::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);
    AccTime += DeltaTime;
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!FirstCT)
    {
        GEngine->ActiveWorld->GetPlayerController()->PlayerCameraManager->ViewTarget.POV.Location = FVector(-2130,-1700,-100);
        GEngine->ActiveWorld->GetPlayerController()->PlayerCameraManager->ViewTarget.POV.Rotation = FRotator(0.f, 90.f, 0.f);
        
        FViewTargetTransitionParams Params;
        Params.BlendTime = 10.0f;
        Params.BlendFunction = VTBlend_Cubic;
        Params.BlendExp = 3.f;
        
        AActor* TargetActor = GEngine->ActiveWorld->SpawnActor<AActor>();
    
        TargetActor->SetActorLocation(FVector(300,0,600));
        TargetActor->SetActorRotation(FRotator(0.f, 180.f, 0.f));
        GEngine->ActiveWorld->GetPlayerController()->SetViewTarget(TargetActor, Params);
        
        
        SetActorLocation(FVector(300,0,600));
        SetActorRotation(FRotator(0.f, 180.f, 0.f));
        FirstCT = true;
    }
    else if (!SecondCT && AccTime > 10.05f && AccTime < 15.0f)
    {
        GEngine->ActiveWorld->GetPlayerController()->PlayerCameraManager->ViewTarget.POV.Location = FVector(300,0,600);
        GEngine->ActiveWorld->GetPlayerController()->PlayerCameraManager->ViewTarget.POV.Rotation = FRotator(0.f, 180.f, 0.f);
        
        FViewTargetTransitionParams Params;
        Params.BlendTime = 5.0f;
        Params.BlendFunction = VTBlend_Cubic;
        Params.BlendExp = 3.f;
        
        AActor* TargetActor = GEngine->ActiveWorld->SpawnActor<AActor>();
    
        TargetActor->SetActorLocation(FVector(0 ,0,550));
        TargetActor->SetActorRotation(FRotator(0.f, 180.f, 0.f));
        GEngine->ActiveWorld->GetPlayerController()->SetViewTarget(TargetActor, Params);
        
        GEngine->ActiveWorld->GetPlayerController()->SetActorLocation(FVector(0 ,0,550));
        GEngine->ActiveWorld->GetPlayerController()->SetActorRotation(FRotator(0.f, 180.f, 0.f));
        
        SecondCT = true;
    }
    else if (AccTime > 15.0f)
    {
        if (!bInitiatlize)
        {
            CameraInitialize();
        }
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
                Camera =GetComponentByClass<UCameraComponent>();
                Camera->SetRelativeLocation(FVector(-50-Target->SnowBallComponent->GetRelativeScale3D().X * 2,0.0,50+Target->SnowBallComponent->GetRelativeScale3D().X*2));
                UpdateCameraRotation();
            }
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
        bSkipFirstFrame = true; // 다음 프레임을 스킵하도록 설정
        return;
    }

    // 2) 마우스 위치 초기화 후 첫 프레임은 스킵
    if (bSkipFirstFrame)
    {
        POINT curPos;
        GetCursorPos(&curPos);
        PrevCursorPos.x = curPos.x;
        PrevCursorPos.y = curPos.y;
        bSkipFirstFrame = false;
        return;
    }

    POINT curPos;
    GetCursorPos(&curPos);

    int deltaX = curPos.x - PrevCursorPos.x;
    int deltaY = curPos.y - PrevCursorPos.y;

    Yaw += deltaX * Sensitivity;
    Pitch -= deltaY * Sensitivity; // Y축은 반대로 (마우스 위로 움직이면 카메라가 위를 봄)
    
    // Pitch 각도 제한 (너무 위아래로 돌아가지 않도록)
    Pitch = FMath::Clamp(Pitch, -40.0f, 20.0f);

    // 마우스 커서를 화면 중앙으로 다시 이동 (무한 회전을 위해)
    SetCursorPos(centerX, centerY);
    PrevCursorPos.x = centerX;
    PrevCursorPos.y = centerY;

    SetActorRotation(FRotator(Pitch, Yaw, GetActorRotation().Roll));
}

void AMyPlayer::CameraInitialize()
{
    GetComponentByClass<UCameraComponent>()->SetRelativeLocation(FVector(-50,0,50));
    GetComponentByClass<UCameraComponent>()->SetRelativeRotation(FRotator(-40,0,0));
    
    // 인트로가 끝나고 카메라 초기화 시 DoF 설정 적용
    // FocalDistance를 원하는 값으로 설정하여 DoF 활성화
    if (APlayerController* PC = GEngine->ActiveWorld->GetPlayerController())
    {
        if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
        {
            // F_Stop: 4.0, SensorWidth: 5000mm, FocalDistance: 65cm (0.65m)
            PCM->SetDoFSettings(5.6f, 5000.f, 65.0f);
        }
    }
    
    bInitiatlize = true;
}
