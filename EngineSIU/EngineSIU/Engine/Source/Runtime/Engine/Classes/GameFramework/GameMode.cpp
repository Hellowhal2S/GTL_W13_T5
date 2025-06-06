#include "GameMode.h"
#include "EngineLoop.h"
#include "SoundManager.h"
#include "InputCore/InputCoreTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/Contents/Actor/SnowBall.h"
#include "Engine/World/World.h"
#include "Components/TextComponent.h"
void AGameMode::PostSpawnInitialize()
{
    AActor::PostSpawnInitialize();

    OnGameInit.AddLambda([]() { UE_LOG(ELogLevel::Display, TEXT("Game Initialized")); });
    
    SetActorTickInEditor(false); // PIE 모드에서만 Tick 수행

    // if (FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler())
    // {
    //     /*Handler->OnPIEModeStartDelegate.AddLambda([this]()
    //     {
    //         this->InitGame();
    //     });*/
    //     // Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
    //     // {
    //     //     // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
    //     //     // if (KeyEvent.GetKeyCode() == VK_SPACE &&
    //     //     //     !bGameRunning && bGameEnded)
    //     //     // {
    //     //     //     StartMatch();
    //     //     // }
    //     // });
    //     //
    //     // Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
    //     //     {
    //     //         // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
    //     //         if (KeyEvent.GetKeyCode() == VK_RCONTROL && 
    //     //             bGameRunning && !bGameEnded)
    //     //         {
    //     //             EndMatch(false);
    //     //         }
    //     //     });
    // }
}

void AGameMode::InitializeComponent()
{

}

UObject* AGameMode::Duplicate(UObject* InOuter)
{
    AGameMode* NewActor = Cast<AGameMode>(Super::Duplicate(InOuter));

    if (NewActor)
    {
        NewActor->bGameRunning = bGameRunning;
        NewActor->bGameEnded = bGameEnded;
        NewActor->GameInfo = GameInfo;
    }
    return NewActor;
}


void AGameMode::InitGame()
{
    OnGameInit.Broadcast();
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    int32 tmp = Engine->CurSceneName.Find("Main");
    if ( tmp != -1 )
    {
        FSoundManager::GetInstance().PlaySound("CreepyHollow");
    }
    else
    {
        FSoundManager::GetInstance().PlaySound("Ice1");
    }
}

void AGameMode::StartMatch()
{
    return;
    bGameRunning = true;
    bGameEnded = false;
    GameInfo.ElapsedGameTime = 0.0f;
    GameInfo.TotalGameTime = 0.0f;
    
    OnGameStart.Broadcast();
}

void AGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bGameRunning && !bGameEnded)
    {
        // TODO: 아래 코드에서 DeltaTime을 2로 나누는 이유가?
        GameInfo.ElapsedGameTime += DeltaTime / 2.0f;
    }
    if (bGameOver)
        return;
    FWString WScore = std::to_wstring(Score);
    if (ScoreFlag)
    {
        for (auto iter : TObjectRange<UTextComponent>())
        {
            if (iter->GetWorld()== GetWorld())
            {
                Cast<UTextComponent>(iter)->SetText(WScore);
            }
        }
    }
    
    if (Life <= 0)
    {
        bGameOver = true;
    }
}

void AGameMode::EndMatch(bool bIsWin)
{
    // 이미 종료된 상태라면 무시
    if (!bGameRunning || bGameEnded)
    {
        return;
    }

    this->Reset();
    
    GameInfo.TotalGameTime = GameInfo.ElapsedGameTime;
    
    OnGameEnd.Broadcast(bIsWin);
}

void AGameMode::Reset()
{
    bGameRunning = false;
    bGameEnded = true;
}
