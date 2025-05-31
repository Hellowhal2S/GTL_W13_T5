#include "LuaScriptComponent.h"
#include "LuaBindingHelpers.h"
#include "LuaScriptFileUtils.h"
#include "World/World.h"
#include "Engine/EditorEngine.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
// #include "Engine/Engine.h"

ULuaScriptComponent::ULuaScriptComponent()
{
}

ULuaScriptComponent::~ULuaScriptComponent()
{
}

void ULuaScriptComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("ScriptPath"), *ScriptPath);
    OutProperties.Add(TEXT("DisplayName"), *DisplayName);
}

void ULuaScriptComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    const FString* TempStr = nullptr;

    TempStr = Properties.Find(TEXT("ScriptPath"));
    if (TempStr)
    {
        this->ScriptPath = *TempStr;
    }
    TempStr = Properties.Find(TEXT("DisplayName"));
    if (TempStr)
    {
        this->DisplayName = *TempStr;
    }
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();

    // PIE 모드에서 중복 초기화 방지
    if (bScriptValid)
    {
        // 이미 초기화된 경우 델리게이트만 정리하고 다시 바인딩
        CleanupDelegateHandles();
    }
    
    InitializeLuaState();
    
    CallLuaFunction("BeginPlay");
}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{ 
    Super::EndPlay(EndPlayReason);

    CallLuaFunction("EndPlay");
    
    // 델리게이트 핸들 정리 - 공통 메서드 사용
    CleanupDelegateHandles();
    
    // Lua 상태 정리
    if (bScriptValid)
    {
        LuaState = sol::state(); // Lua 상태 재설정으로 메모리 정리
        bScriptValid = false;
    }
}

UObject* ULuaScriptComponent::Duplicate(UObject* InOuter)
{
    ULuaScriptComponent* NewComponent = Cast<ULuaScriptComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->ScriptPath = ScriptPath;
        NewComponent->DisplayName = DisplayName;
        NewComponent->bScriptValid = false; // 복제된 컴포넌트는 새로 초기화되어야 함
        NewComponent->LastWriteTime = LastWriteTime;
        
        // ExposedProperties 복사
        NewComponent->ExposedProperties = ExposedProperties;
        
        // PIE 모드에서는 델리게이트 핸들을 복사하지 않음 - 새로 바인딩될 것임
        // DelegateHandles와 DelegateHandleToKeyMap은 빈 상태로 시작
        NewComponent->DelegateHandles.Empty();
        NewComponent->DelegateHandleToKeyMap.Empty();
        
        UE_LOG(ELogLevel::Display, TEXT("LuaScriptComponent duplicated for PIE mode"));
    }
    return NewComponent;
}

/* ActorComponent가 Actor와 World에 등록이 되었다는 전제하에 호출됩니다
 * So That we can use GetOwner() and GetWorld() safely
 */
void ULuaScriptComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (ScriptPath.IsEmpty()) {
        bool bSuccess = LuaScriptFileUtils::MakeScriptPathAndDisplayName(
            L"template.lua",
            GetOwner()->GetWorld()->GetName().ToWideString(),
            GetOwner()->GetName().ToWideString(),
            ScriptPath,
            DisplayName
        );
        if (!bSuccess) {
            UE_LOG(ELogLevel::Error, TEXT("Failed to create script from template"));
            return;
        }
    }
}

void ULuaScriptComponent::SetScriptPath(const FString& InScriptPath)
{
    ScriptPath = InScriptPath;
    bScriptValid = false;
}

void ULuaScriptComponent::InitializeLuaState()
{
    /*if (ScriptPath.IsEmpty()) {
        bool bSuccess = LuaScriptFileUtils::CopyTemplateToActorScript(
            L"template.lua",
            GetOwner()->GetWorld()->GetName().ToWideString(),
            GetOwner()->GetName().ToWideString(),
            ScriptPath,
            DisplayName
        );
        if (!bSuccess) {
            UE_LOG(ELogLevel::Error, TEXT("Failed to create script from template"));
            return;
        }
    }*/

    LuaState.open_libraries();
    BindEngineAPI();

    try {
        LuaState.script_file((*ScriptPath));
        bScriptValid = true;
        const std::wstring FilePath = ScriptPath.ToWideString();
        LastWriteTime = std::filesystem::last_write_time(FilePath);
    }
    catch (const sol::error& err) {
        UE_LOG(ELogLevel::Error, TEXT("Lua Initialization error: %s"), err.what());
    }

    CallLuaFunction("InitializeLua");
}

void ULuaScriptComponent::BindEngineAPI()
{
    // [1] 바인딩 전 글로벌 키 스냅샷
    TArray<FString> Before = LuaDebugHelper::CaptureGlobalNames(LuaState);

    // 현재 컴포넌트 인스턴스를 Lua 상태에 설정하여 바인딩 함수들이 접근할 수 있도록 함
    LuaState["__current_script_component"] = this;

    LuaBindingHelpers::BindPrint(LuaState);    // 0) Print 바인딩
    LuaBindingHelpers::BindFVector(LuaState);   // 2) FVector 바인딩
    LuaBindingHelpers::BindFRotator(LuaState);
    LuaBindingHelpers::BindController(LuaState);
    LuaBindingHelpers::BindPhysics(LuaState);    // 물리 함수 바인딩
    
    auto ActorType = LuaState.new_usertype<AActor>("Actor",
        sol::constructors<>(),
        "Location", sol::property(
            &AActor::GetActorLocation,
            &AActor::SetActorLocation
        ),
        "Rotator", sol::property(
            &AActor::GetActorRotation,
            &AActor::SetActorRotation
        ),
        "Forward", &AActor::GetActorForwardVector
    );
    
    // 프로퍼티 바인딩
    LuaState["actor"] = GetOwner();

    // [2] 바인딩 후, 새로 추가된 글로벌 키만 자동 로그
    LuaDebugHelper::LogNewBindings(LuaState, Before);
}

bool ULuaScriptComponent::CheckFileModified()
{
    if (ScriptPath.IsEmpty()) return false;

    try {
        std::wstring FilePath = ScriptPath.ToWideString();
        const auto CurrentTime = std::filesystem::last_write_time(FilePath);

        if (CurrentTime > LastWriteTime) {
            LastWriteTime = CurrentTime;
            return true;
        }
    }
    catch (const std::exception& e) {
        UE_LOG(ELogLevel::Error, TEXT("Failed to check lua script file"));
    }
    return false;
}

void ULuaScriptComponent::ReloadScript()
{
    UE_LOG(ELogLevel::Display, TEXT("Starting HotReload for script: %s"), *ScriptPath);
    
    // 1. PersistentData 백업
    sol::table PersistentData;
    if (bScriptValid && LuaState["PersistentData"].valid()) {
        PersistentData = LuaState["PersistentData"];
    }
    
    // 2. 기존 델리게이트 핸들 정리 (EndPlay와 동일한 로직)
    CleanupDelegateHandles();
    
    // 3. Lua 상태 안전하게 재생성
    try {
        LuaState = sol::state();
        bScriptValid = false;
        
        // 4. 새로운 Lua 환경 초기화
        InitializeLuaState();
        
        // 5. PersistentData 복원
        if (PersistentData.valid()) {
            LuaState["PersistentData"] = PersistentData;
        }
        
        // 6. HotReload 이벤트 호출
        CallLuaFunction("OnHotReload");
        CallLuaFunction("BeginPlay");
        
        UE_LOG(ELogLevel::Display, TEXT("HotReload completed successfully"));
    }
    catch (const sol::error& err) {
        UE_LOG(ELogLevel::Error, TEXT("HotReload failed: %s"), err.what());
        bScriptValid = false;
    }
    catch (const std::exception& e) {
        UE_LOG(ELogLevel::Error, TEXT("HotReload failed with exception: %s"), e.what());
        bScriptValid = false;
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    CallLuaFunction("Tick", DeltaTime);

    if (CheckFileModified()) {
        try {
            ReloadScript();
            UE_LOG(ELogLevel::Display, TEXT("Lua script reloaded"));
        }
        catch (const sol::error& e) {
            UE_LOG(ELogLevel::Error, TEXT("Failed to reload lua script"));
        }
    }
}

void ULuaScriptComponent::CleanupDelegateHandles()
{
    UE_LOG(ELogLevel::Display, TEXT("Cleaning up delegate handles"));
    
    try {
        if (GEngine && GEngine->ActiveWorld)
        {
            APlayerController* PC = GEngine->ActiveWorld->GetPlayerController();
            if (PC && PC->GetInputComponent())
            {
                for (FDelegateHandle& Handle : DelegateHandles)
                {
                    if (Handle.IsValid())
                    {
                        // 핸들과 키 매핑에서 키를 찾아서 InputComponent에서 바인딩 해제
                        if (FString* Key = DelegateHandleToKeyMap.Find(Handle))
                        {
                            PC->GetInputComponent()->UnbindAction(*Key, Handle);
                            UE_LOG(ELogLevel::Display, TEXT("Unbound controller action for key: %s"), **Key);
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        UE_LOG(ELogLevel::Error, TEXT("Exception during delegate cleanup: %s"), e.what());
    }
    
    // 컨테이너 정리
    DelegateHandles.Empty();
    DelegateHandleToKeyMap.Empty();
}

void ULuaScriptComponent::AddDelegateHandle(const FDelegateHandle& Handle, const FString& Key)
{
    if (Handle.IsValid())
    {
        DelegateHandles.Add(Handle);
        DelegateHandleToKeyMap.Add(Handle, Key);
        UE_LOG(ELogLevel::Display, TEXT("Added delegate handle for key: %s"), *Key);
    }
    else
    {
        UE_LOG(ELogLevel::Warning, TEXT("Attempted to add invalid delegate handle for key: %s"), *Key);
    }
}

