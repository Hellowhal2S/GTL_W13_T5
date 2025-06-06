#include "LuaScriptManager.h"
#include <filesystem>
#include <UserInterface/Console.h>

#include "SoundManager.h"
#include "Lua/LuaTypes/LuaUserTypes.h"
#include "Lua/LuaScriptComponent.h"
#include "Animation/AnimStateMachine.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/Contents/Actor/SnowBall.h"
#include "World/World.h"
#include "Physics/PhysicsManager.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Contents/Actor/SphereTargetComponent.h"
TMap<FString, FLuaTableScriptInfo> FLuaScriptManager::ScriptCacheMap;
TSet<ULuaScriptComponent*> FLuaScriptManager::ActiveLuaComponents;
TSet<UAnimStateMachine*> FLuaScriptManager::ActiveAnimLua;

FLuaScriptManager::FLuaScriptManager()
{
    LuaState.open_libraries(
        sol::lib::base,       // Lua를 사용하기 위한 기본 라이브러리 (print, type, pcall 등)
        // sol::lib::package,    // 모듈 로딩(require) 및 패키지 관리 기능
        sol::lib::coroutine,  // 코루틴 생성 및 관리 기능 (yield, resume 등)
        sol::lib::string,     // 문자열 검색, 치환, 포맷팅 등 문자열 처리 기능
        sol::lib::math,       // 수학 함수 (sin, random, pi 등) 및 상수 제공
        sol::lib::table,      // 테이블(배열/딕셔너리) 생성 및 조작 기능 (insert, sort 등)
        // sol::lib::io,         // 파일 읽기/쓰기 등 입출력 관련 기능
        // sol::lib::os,         // 운영체제 관련 기능 (시간, 날짜, 파일 시스템 접근 등)
        sol::lib::debug,      // 디버깅 및 introspection 기능 (traceback, getinfo 등)
        sol::lib::bit32,      // 32비트 정수 대상 비트 연산 기능 (Lua 5.2 이상)
        // sol::lib::jit,        // LuaJIT의 JIT 컴파일러 제어 기능 (LuaJIT 전용)
        // sol::lib::ffi,        // 외부 C 함수 및 데이터 구조 접근 기능 (LuaJIT 전용)
        sol::lib::utf8        // UTF-8 인코딩 문자열 처리 기능 (Lua 5.3 이상)
    );

    SetLuaDefaultTypes();
}

FLuaScriptManager::~FLuaScriptManager()
{
    ScriptCacheMap.Empty();
    ActiveLuaComponents.Empty();
    ActiveAnimLua.Empty();
}

void FLuaScriptManager::SetLuaDefaultTypes()
{
    sol::table TypeTable = LuaState.create_table("EngineTypes");

    // Default Math Types.
    LuaTypes::FBindLua<FColor>::Bind(TypeTable);
    LuaTypes::FBindLua<FLinearColor>::Bind(TypeTable);
    LuaTypes::FBindLua<FVector>::Bind(TypeTable);
    LuaTypes::FBindLua<FVector2D>::Bind(TypeTable);
    LuaTypes::FBindLua<FVector4>::Bind(TypeTable);
    LuaTypes::FBindLua<FRotator>::Bind(TypeTable);
    LuaTypes::FBindLua<FQuat>::Bind(TypeTable);
    LuaTypes::FBindLua<FMatrix>::Bind(TypeTable);
    
    // 엔진 API 바인딩 추가
    BindEngineAPIs();
}

void FLuaScriptManager::BindEngineAPIs()
{
    // print 함수 바인딩
    LuaState.set_function("print", [](sol::variadic_args va, sol::this_state ts) {
        std::string out;
        bool first = true;
        lua_State* L = ts;
        for (auto&& arg : va) {
            if (!first) out += " ";
            first = false;
            switch (arg.get_type()) {
            case sol::type::string:
                out += arg.as<std::string>();
                break;
            case sol::type::number:
                out += std::to_string(arg.as<double>());
                break;
            case sol::type::boolean:
                out += arg.as<bool>() ? "true" : "false";
                break;
            case sol::type::nil:
                out += "nil";
                break;
            default:
                // stack_proxy는 lua_topointer로 포인터 주소를 얻을 수 있음
                out += "<";
                out += sol::type_name(L, arg.get_type());
                out += ">";
                break;
            }
        }
        UE_LOG(ELogLevel::Display, TEXT("%s"), *FString(out.c_str()));
    });

    // UE_LOG 함수 바인딩  
    LuaState.set_function("UE_LOG", [](const std::string& level, const std::string& msg) {
        FString converted = FString(msg.c_str());
        if (level == "Error") {
            UE_LOG(ELogLevel::Error, TEXT("%s"), *converted);
        } else if (level == "Warning") {
            UE_LOG(ELogLevel::Warning, TEXT("%s"), *converted);
        } else {
            UE_LOG(ELogLevel::Display, TEXT("%s"), *converted);
        }
    });

    // controller 함수 바인딩
    LuaState.set_function("controller", [this](const std::string& key, const std::function<void(float)>& callback) {
        if (GEngine && GEngine->ActiveWorld) {
            APlayerController* PC = GEngine->ActiveWorld->GetPlayerController();
            if (PC && PC->GetInputComponent()) {
                PC->BindAction(FString(key.c_str()), callback);
                UE_LOG(ELogLevel::Display, TEXT("Controller binding registered for key: %s"), *FString(key.c_str()));
            }
        }
    });

    // controllerRelease 함수 바인딩 추가
    LuaState.set_function("controllerRelease", [this](const std::string& key, const std::function<void()>& callback) {
        if (GEngine && GEngine->ActiveWorld) {
            APlayerController* PC = GEngine->ActiveWorld->GetPlayerController();
            if (PC && PC->GetInputComponent()) {
                // 키 릴리즈 이벤트를 위한 델리게이트 바인딩
                FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
                if (Handler) {
                    // 키 릴리즈 델리게이트에 바인딩
                    Handler->OnKeyUpDelegate.AddLambda([key, callback](const FKeyEvent& InKeyEvent) {
                        // 지정된 키가 릴리즈되었는지 확인
                        FString keyName = key.c_str();
                        if (keyName == "SpaceBar" && InKeyEvent.GetKey() == EKeys::SpaceBar) {
                            callback();
                        }
                        else if (keyName == "W" && InKeyEvent.GetKey() == EKeys::W) {
                            callback();
                        }
                        else if (keyName == "S" && InKeyEvent.GetKey() == EKeys::S) {
                            callback();
                        }
                        else if (keyName == "A" && InKeyEvent.GetKey() == EKeys::A) {
                            callback();
                        }
                        else if (keyName == "D" && InKeyEvent.GetKey() == EKeys::D) {
                            callback();
                        }
                        // 필요한 다른 키들도 추가 가능
                    });
                    UE_LOG(ELogLevel::Display, TEXT("Controller release binding registered for key: %s"), *FString(key.c_str()));
                }
            }
        }
    });

    // 물리 함수들 바인딩
    LuaState.set_function("ApplyForce", [](const FVector& force, int forceMode) {
        if (GEngine && GEngine->PhysicsManager) {
            if (AActor* currentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor()) {
                GEngine->PhysicsManager->ApplyForceToActor(currentActor, force, forceMode);
            }
        }
    });

    LuaState.set_function("ApplyTorque", [](const FVector& torque, int forceMode) {
        if (GEngine && GEngine->PhysicsManager) {
            if (AActor* currentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor()) {
                GEngine->PhysicsManager->ApplyTorqueToActor(currentActor, torque, forceMode);
            }
        }
    });

    // @note SnowBall에 대한 하드코딩
    LuaState.set_function("ApplyTorqueToSnowBall", [](const FVector& torque, int forceMode) {
        if (GEngine && GEngine->PhysicsManager) {
            for(const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->ApplyTorqueToActor(SnowBallActor, torque, forceMode);
                    break;
                }
            }
        }
    });

    // 점프 함수 바인딩 추가
    LuaState.set_function("ApplyJumpImpulseToSnowBall", [](float jumpForce) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->ApplyJumpImpulseToActor(SnowBallActor, jumpForce);
                    break;
                }
            }
        }
    });

    // === 새로운 속도 제어 함수들 바인딩 추가 ===
    
    // 각속도 직접 설정 함수들
    LuaState.set_function("SetAngularVelocityToSnowBall", [](const FVector& angularVelocity) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->SetAngularVelocityToActor(SnowBallActor, angularVelocity);
                    break;
                }
            }
        }
    });

    LuaState.set_function("AddAngularVelocityToSnowBall", [](const FVector& angularVelocity) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->AddAngularVelocityToActor(SnowBallActor, angularVelocity);
                    break;
                }
            }
        }
    });

    // 선형속도 직접 설정 함수들
    LuaState.set_function("SetLinearVelocityToSnowBall", [](const FVector& velocity) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->SetLinearVelocityToActor(SnowBallActor, velocity);
                    break;
                }
            }
        }
    });

    LuaState.set_function("AddLinearVelocityToSnowBall", [](const FVector& velocity) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->AddLinearVelocityToActor(SnowBallActor, velocity);
                    break;
                }
            }
        }
    });

    // 속도 조회 함수들
    LuaState.set_function("GetLinearVelocityFromSnowBall", []() -> FVector {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    return GEngine->PhysicsManager->GetLinearVelocityFromActor(SnowBallActor);
                }
            }
        }
        return FVector::ZeroVector;
    });

    LuaState.set_function("GetAngularVelocityFromSnowBall", []() -> FVector {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    return GEngine->PhysicsManager->GetAngularVelocityFromActor(SnowBallActor);
                }
            }
        }
        return FVector::ZeroVector;
    });

    // 특정 위치에 힘 적용 함수 바인딩 추가
    LuaState.set_function("ApplyForceAtPosition", [](const FVector& force, const FVector& position, int forceMode) {
        if (GEngine && GEngine->PhysicsManager) {
            if (AActor* currentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor()) {
                GEngine->PhysicsManager->ApplyForceAtPositionToActor(currentActor, force, position, forceMode);
            }
        }
    });

    LuaState.set_function("GrowSnowBall", [](float deltaRadius) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->GrowBall(SnowBallActor, deltaRadius);
                    SnowBallActor->SnowBallComponent->AddScale(FVector(deltaRadius));
                    SnowBallActor->GetComponentByClass<USphereTargetComponent>()->SetRadius(SnowBallActor->GetComponentByClass<USphereTargetComponent>()->GetRadius() + deltaRadius*1.2);
                    break;
                }
            }
        }
    });

    // Contact 이벤트 바인딩 함수 추가
    LuaState.set_function("RegisterContactCallback", [](AActor* actor, sol::function onContactBegin, sol::function onContactEnd) {
        if (GEngine && GEngine->PhysicsManager && actor) {
            GEngine->PhysicsManager->RegisterContactCallback(actor, onContactBegin, onContactEnd);
        }
    });

    LuaState.set_function("UnregisterContactCallback", [](AActor* actor) {
        if (GEngine && GEngine->PhysicsManager && actor) {
            GEngine->PhysicsManager->UnregisterContactCallback(actor);
        }
    });

    // @note SnowBall에 대한 하드코딩
    LuaState.set_function("RegisterSnowBallContactCallback", [](sol::function onContactBegin, sol::function onContactEnd) {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->RegisterContactCallback(SnowBallActor, onContactBegin, onContactEnd);
                    break;
                }
            }
        }
    });

    LuaState.set_function("UnregisterSnowBallContactCallback", []() {
        if (GEngine && GEngine->PhysicsManager) {
            for (const AActor* actor : GEngine->ActiveWorld->GetActiveLevel()->Actors) {
                ASnowBall* SnowBallActor = Cast<ASnowBall>(actor);
                if (SnowBallActor) {
                    GEngine->PhysicsManager->UnregisterContactCallback(SnowBallActor);
                    break;
                }
            }
        }
    });

    LuaState.set_function("GetActorName", [](AActor* actor) {
        return GetData(actor->GetActorLabel());
    });

    LuaState.set_function("GetActorRotation", [](AActor* actor) {
        return actor->GetActorRotation();
    });

    LuaState.set_function("GetActorForwardVector", [](AActor* actor) {
        return actor->GetActorForwardVector();
    });

    LuaState.set_function("GetActorRightVector", [](AActor* actor) {
        return actor->GetActorRightVector();
    });

    LuaState.set_function("GetActorLocation", [](AActor* actor) {
        return actor->GetActorLocation();
    });
    LuaState.set_function("PlayJumpSound", []() {
        FSoundManager::GetInstance().PlaySound("jump");
    });
    LuaState.set_function("PlayRockSound", []() {
            FSoundManager::GetInstance().PlaySound("rock");
        });
    UE_LOG(ELogLevel::Display, TEXT("Engine APIs bound to Lua"));
}

FLuaScriptManager& FLuaScriptManager::Get()
{
    static FLuaScriptManager Instance;
    return Instance;
}

sol::state& FLuaScriptManager::GetLua()
{
    return LuaState;
}

sol::table FLuaScriptManager::CreateLuaTable(const FString& ScriptName)
{
    if (!std::filesystem::exists(*ScriptName))
    {
        UE_LOG(ELogLevel::Error, TEXT("InValid Lua File name."));
        //assert(false && "Lua File not found");
        return sol::table();
    }

    if (!ScriptCacheMap.Contains(ScriptName))
    {
        sol::protected_function_result Result = LuaState.script_file(*ScriptName);
        if (!Result.valid())
        {
            sol::error err = Result;
            UE_LOG(ELogLevel::Error, TEXT("Lua Error: %s"), *FString(err.what()));
            return sol::table();
        }
        
        sol::object ReturnValue = Result.get<sol::object>();
        if (!ReturnValue.is<sol::table>())
        {
            UE_LOG(ELogLevel::Error, TEXT("Lua Error: %s"), *FString("Script file did not return a table."));
            return sol::table();
        }
        
        FLuaTableScriptInfo NewInfo;
        NewInfo.ScriptTable = ReturnValue.as<sol::table>();
        NewInfo.LastWriteTime = std::filesystem::last_write_time(ScriptName.ToWideString());
        ScriptCacheMap.Add(ScriptName, NewInfo);
    }

    //return ScriptCacheMap[ScriptName];

    sol::table& ScriptClass = ScriptCacheMap[ScriptName].ScriptTable;

    sol::table NewEnv = LuaState.create_table();
    for (auto& pair : ScriptClass)
    {
        NewEnv.set(pair.first, pair.second);
    }

    return NewEnv;
}

void FLuaScriptManager::RegisterActiveLuaComponent(ULuaScriptComponent* LuaComponent)
{
    ActiveLuaComponents.Add(LuaComponent);
}

void FLuaScriptManager::UnRigisterActiveLuaComponent(ULuaScriptComponent* LuaComponent)
{
    if (ActiveLuaComponents.Contains(LuaComponent))
        ActiveLuaComponents.Remove(LuaComponent);
}

void FLuaScriptManager::RegisterActiveAnimLua(UAnimStateMachine* AnimInstance)
{
    ActiveAnimLua.Add(AnimInstance);
}

void FLuaScriptManager::UnRegisterActiveAnimLua(UAnimStateMachine* AnimInstance)
{
    if (ActiveAnimLua.Contains(AnimInstance))
        ActiveAnimLua.Remove(AnimInstance);
}

void FLuaScriptManager::HotReloadLuaScript()
{
    TSet<FString> ChangedScriptName;
    TMap<FString, FLuaTableScriptInfo> CopiedScriptCacheMap = ScriptCacheMap;
    for (const TMap<FString, FLuaTableScriptInfo>::PairType& ScriptCached : CopiedScriptCacheMap)
    {
        FString ScriptName = ScriptCached.Key;
        FLuaTableScriptInfo LuaScriptInfo = ScriptCached.Value;
        auto depTime = std::filesystem::last_write_time(GetData(ScriptName));
        if (LuaScriptInfo.LastWriteTime != depTime)
        {
            if (ScriptCacheMap.Contains(ScriptName))
            {
                ScriptCacheMap.Remove(ScriptName);
            }

            ChangedScriptName.Add(ScriptName);
        }
    }

    for (const FString& ChangedScript : ChangedScriptName)
    {
        for (const ULuaScriptComponent* LuaComponent : ActiveLuaComponents)
        {
            const FString FullPath = "LuaScripts/Actors/" + LuaComponent->GetScriptName() + ".lua";
            if (FullPath == ChangedScript)
            {
                LuaComponent->GetOwner()->BindSelfLuaProperties();
                UE_LOG(ELogLevel::Display, TEXT("Lua Script Reloaded: %s"), *ChangedScript);
            } 
        }

        for (UAnimStateMachine* AnimStateMachine : ActiveAnimLua)
        {
            if (AnimStateMachine->GetLuaScriptName() == ChangedScript)
            {
                AnimStateMachine->InitLuaStateMachine();
                UE_LOG(ELogLevel::Display, TEXT("Lua Script Reloaded: %s"), *ChangedScript);
            }
        }
    }
}
