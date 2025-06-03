#include "AnimStateMachine.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Lua/LuaScriptManager.h"
#include "Animation/AnimSequence.h"

UAnimStateMachine::UAnimStateMachine()
{
    
}

void UAnimStateMachine::Initialize(USkeletalMeshComponent* InOwner, ULuaScriptAnimInstance* InAnimInstance)
{
    OwningComponent = InOwner;
    OwningAnimInstance = InAnimInstance;

    LuaScriptName = OwningComponent->StateMachineFileName;
    InitLuaStateMachine();

}

void UAnimStateMachine::ProcessState(float DeltaTime)
{
    if (!LuaTable.valid())
        return;

    sol::function UpdateFunc = LuaTable["Update"];
    if (!UpdateFunc.valid())
    {
        UE_LOG(ELogLevel::Warning, TEXT("Lua Update function not valid!"));
        return;
    }

    sol::object result = UpdateFunc(LuaTable, DeltaTime);

    sol::table StateInfo = result.as<sol::table>();
    FString StateName = StateInfo["anim"].get_or(std::string("")).c_str();
    float Blend = StateInfo["blend"].get_or(0.f);

    // 디버깅 출력 추가
    //UE_LOGFMT(ELogLevel::Display, "ProcessState - Animation Name: {}", StateName);
    //UE_LOGFMT(ELogLevel::Display, "ProcessState - Blend Time: {}", Blend);

    if (OwningAnimInstance)
    {
        UAnimSequence* NewAnim = Cast<UAnimSequence>(UAssetManager::Get().GetAnimation(StateName));
        if (NewAnim)
        {
            //UE_LOGFMT(ELogLevel::Display, "Found animation: {}", NewAnim->GetName());
            OwningAnimInstance->SetAnimation(NewAnim, Blend, false, false);
        }
        else
        {
            //UE_LOGFMT(ELogLevel::Warning, "Animation not found: {}", StateName);
        }
    }
    else
    {
        //UE_LOG(ELogLevel::Warning, TEXT("OwningAnimInstance is null"));
    }
}

void UAnimStateMachine::InitLuaStateMachine()
{
    if (LuaScriptName.IsEmpty())
    {
        return;
    }
    LuaTable = FLuaScriptManager::Get().CreateLuaTable(LuaScriptName);

    FLuaScriptManager::Get().RegisterActiveAnimLua(this);
    if (!LuaTable.valid())
        return;

    LuaTable["OwnerCharacter"] = Cast<AActor>(OwningComponent->GetOwner());
}



