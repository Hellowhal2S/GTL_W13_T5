#pragma once
#include <sol/sol.hpp>
#include "Runtime/Core/Math/Vector.h"
#include "Runtime/Engine/UserInterface/Console.h"
#include "Developer/LuaUtils/LuaBindMacros.h"

#include "Engine/Engine.h"
#include "World/World.h"
#include "Physics/PhysicsManager.h"
#include "LuaScriptComponent.h"  // ULuaScriptComponent 클래스를 위해 추가

namespace LuaBindingHelpers
{
    // FVector 타입 바인딩 함수
    inline void BindFVector(sol::state& Lua)
    {
        Lua.Lua_NewUserType(
            FVector,
    
            // Constructors
            LUA_BIND_CONSTRUCTORS(
                FVector(),
                FVector(float, float, float),
                FVector(float)
            ),
    
            // Member variables
            LUA_BIND_MEMBER(&FVector::X),
            LUA_BIND_MEMBER(&FVector::Y),
            LUA_BIND_MEMBER(&FVector::Z),
    
            // Utility functions
            LUA_BIND_FUNC(&FVector::Dot),
            LUA_BIND_FUNC(&FVector::Cross),
            LUA_BIND_FUNC(&FVector::Equals),
            LUA_BIND_FUNC(&FVector::AllComponentsEqual),
            LUA_BIND_FUNC(&FVector::Length),
            LUA_BIND_FUNC(&FVector::SquaredLength),
            LUA_BIND_FUNC(&FVector::SizeSquared),
            LUA_BIND_FUNC(&FVector::Normalize),
            LUA_BIND_FUNC(&FVector::GetUnsafeNormal),
            LUA_BIND_FUNC(&FVector::GetSafeNormal),
            LUA_BIND_FUNC(&FVector::ComponentMin),
            LUA_BIND_FUNC(&FVector::ComponentMax),
            LUA_BIND_FUNC(&FVector::IsNearlyZero),
            LUA_BIND_FUNC(&FVector::IsZero),
            LUA_BIND_FUNC(&FVector::IsNormalized),
            LUA_BIND_FUNC(&FVector::ToString),
    
            // Static Method
            LUA_BIND_FUNC(&FVector::Zero),
            LUA_BIND_FUNC(&FVector::One),
    
            LUA_BIND_FUNC(&FVector::UnitX),
            LUA_BIND_FUNC(&FVector::UnitY),
            LUA_BIND_FUNC(&FVector::UnitZ),
    
            LUA_BIND_FUNC(&FVector::Distance),
            LUA_BIND_FUNC(&FVector::DotProduct),
            LUA_BIND_FUNC(&FVector::CrossProduct),
    
            // Static properties
            LUA_BIND_STATIC(FVector::ZeroVector),
            LUA_BIND_STATIC(FVector::OneVector),
            LUA_BIND_STATIC(FVector::UpVector),
            LUA_BIND_STATIC(FVector::DownVector),
            LUA_BIND_STATIC(FVector::ForwardVector),
            LUA_BIND_STATIC(FVector::BackwardVector),
            LUA_BIND_STATIC(FVector::RightVector),
            LUA_BIND_STATIC(FVector::LeftVector),
            LUA_BIND_STATIC(FVector::XAxisVector),
            LUA_BIND_STATIC(FVector::YAxisVector),
            LUA_BIND_STATIC(FVector::ZAxisVector),
    
            // Operators
            sol::meta_function::addition, LUA_BIND_OVERLOAD_WITHOUT_NAME2(\
                &FVector::operator+,
                FVector(const FVector&) const,
                FVector(float) const
            ),
            sol::meta_function::subtraction, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
                &FVector::operator-,
                FVector(const FVector&) const,
                FVector(float) const
            ),
            sol::meta_function::multiplication, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
                &FVector::operator*,
                FVector(const FVector&) const,
                FVector(float) const
            ),
            sol::meta_function::division, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
                &FVector::operator/,
                FVector(const FVector&) const,
                FVector(float) const
            ),
            sol::meta_function::equal_to, &FVector::operator==,
    
            // 연산자 메서드
            "AddAssign", [](FVector& Self, const FVector& Other) { return Self += Other; },
            "SubAssign", [](FVector& Self, const FVector& Other) { return Self -= Other; },
            "MulAssign", [](FVector& Self, float Scalar) { return Self *= Scalar; },
            "DivAssign", [](FVector& Self, float Scalar) { return Self /= Scalar; }
        );
    }

    inline void BindFRotator(sol::state& Lua)
    {
        Lua.new_usertype<FRotator>("FRotator",
            // 생성자
            sol::constructors<
                FRotator(), 
                FRotator(float, float, float)
            >(),

            // 속성
            "Pitch", &FRotator::Pitch,
            "Yaw",   &FRotator::Yaw,
            "Roll",  &FRotator::Roll,

            // 연산자
            sol::meta_function::addition,      [](const FRotator& a, const FRotator& b){ return a + b; },
            sol::meta_function::subtraction,   [](const FRotator& a, const FRotator& b){ return a - b; }
        );
    }
    
    // UE_LOG 바인딩 함수
    inline void BindUE_LOG(sol::state& Lua)
    {
        Lua.set_function("UE_LOG",
            [](const std::string& Level, const std::string& Msg)
            {
                FString Converted = FString(Msg.c_str());
                if (Level == "Error")
                {
                    UE_LOG(ELogLevel::Error, TEXT("%s"), *Converted);
                }
                else if (Level == "Warning")
                {
                    UE_LOG(ELogLevel::Warning, TEXT("%s"), *Converted);
                }
                else
                {
                    UE_LOG(ELogLevel::Display, TEXT("%s"), *Converted);
                }
            }
        );
    }

    // Lua print 함수 바인딩 (콘솔 + 화면)
    inline void BindPrint(sol::state& Lua)
    {
        Lua.set_function("print",
            [](const std::string& Msg)
            {
                // 로그에 출력
                UE_LOG(ELogLevel::Error, TEXT("%s"), Msg.c_str());
                // 화면에 출력
                OutputDebugStringA(Msg.c_str()); // 디버그 창에 출력
            }
        );
    }

    inline void BindController(sol::state& Lua)
    {
        Lua.set_function("controller",
            [&Lua](const std::string& Key, const std::function<void(float)>& Callback)
            {
                if (GEngine && GEngine->ActiveWorld)
                {
                    APlayerController* PC = GEngine->ActiveWorld->GetPlayerController();
                    if (PC && PC->GetInputComponent())
                    {
                        // 현재 실행 중인 LuaScriptComponent를 Lua 상태에서 가져오기
                        sol::object ScriptComponentObj = Lua["__current_script_component"];
                        if (ScriptComponentObj.valid())
                        {
                            if (ULuaScriptComponent* ScriptComponent = ScriptComponentObj.as<ULuaScriptComponent*>())
                            {
                                // 새로운 핸들 반환 메서드를 사용하여 델리게이트 핸들 추적
                                FString KeyString = FString(Key.c_str());
                                FDelegateHandle Handle = PC->BindActionWithHandle(KeyString, Callback);
                                
                                // public 메서드를 통해 델리게이트 핸들 추가
                                ScriptComponent->AddDelegateHandle(Handle, KeyString);
                                
                                UE_LOG(ELogLevel::Display, TEXT("Controller binding registered for key: %s with handle"), *KeyString);
                            }
                        }
                        else
                        {
                            // 폴백: 핸들 추적 없이 바인딩
                            PC->BindAction(FString(Key.c_str()), Callback);
                            UE_LOG(ELogLevel::Warning, TEXT("Controller binding registered for key: %s without handle tracking"), *FString(Key.c_str()));
                        }
                    }
                }
            }
        );
    }
    
    // 물리 함수 바인딩
    inline void BindPhysics(sol::state& Lua)
    {
        // PhysicsManager를 통한 토크 적용
        Lua.set_function("ApplyTorque", 
            [](const FVector& Torque, int ForceMode)
            {
                if (GEngine && GEngine->PhysicsManager)
                {
                    if (AActor* CurrentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor())
                    {
                        GEngine->PhysicsManager->ApplyTorqueToActor(CurrentActor, Torque, ForceMode);
                    }
                }
            }
        );
        
        // PhysicsManager를 통한 힘 적용
        Lua.set_function("ApplyForce", 
            [](const FVector& Force, int ForceMode)
            {
                if (GEngine && GEngine->PhysicsManager)
                {
                    if (AActor* CurrentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor())
                    {
                        GEngine->PhysicsManager->ApplyForceToActor(CurrentActor, Force, ForceMode);
                    }
                }
            }
        );
        
        // PhysicsManager를 통한 위치별 힘 적용
        Lua.set_function("ApplyForceAtPosition", 
            [](const FVector& Force, const FVector& Position, int ForceMode)
            {
                if (GEngine && GEngine->PhysicsManager)
                {
                    if (AActor* CurrentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor())
                    {
                        GEngine->PhysicsManager->ApplyForceAtPositionToActor(CurrentActor, Force, Position, ForceMode);
                    }
                }
            }
        );
        
        // PhysicsManager를 통한 점프 충격 적용
        Lua.set_function("ApplyJumpImpulse", 
            [](float JumpForce)
            {
                if (GEngine && GEngine->PhysicsManager)
                {
                    if (AActor* CurrentActor = GEngine->ActiveWorld->GetPlayerController()->GetPossessedActor())
                    {
                        GEngine->PhysicsManager->ApplyJumpImpulseToActor(CurrentActor, JumpForce);
                    }
                }
            }
        );
        
        // 특정 액터에 토크 적용
        Lua.set_function("ApplyTorqueToActor", 
            [](AActor* Actor, const FVector& Torque, int ForceMode)
            {
                if (GEngine && GEngine->PhysicsManager && Actor)
                {
                    GEngine->PhysicsManager->ApplyTorqueToActor(Actor, Torque, ForceMode);
                }
            }
        );
        
        // 특정 액터에 힘 적용
        Lua.set_function("ApplyForceToActor", 
            [](AActor* Actor, const FVector& Force, int ForceMode)
            {
                if (GEngine && GEngine->PhysicsManager && Actor)
                {
                    GEngine->PhysicsManager->ApplyForceToActor(Actor, Force, ForceMode);
                }
            }
        );
        
        // PhysX Contact 이벤트 바인딩
        Lua.set_function("BindContactEvents", 
            [&Lua]()
            {
                // 현재 실행 중인 LuaScriptComponent를 Lua 상태에서 가져오기
                sol::object ScriptComponentObj = Lua["__current_script_component"];
                if (ScriptComponentObj.valid())
                {
                    if (ULuaScriptComponent* ScriptComponent = ScriptComponentObj.as<ULuaScriptComponent*>())
                    {
                        // 현재 Actor 가져오기
                        AActor* Owner = ScriptComponent->GetOwner();
                        if (Owner && GEngine && GEngine->PhysicsManager)
                        {
                            // Lua 함수들 가져오기
                            sol::function OnContactBegin = Lua["OnContactBegin"];
                            sol::function OnContactEnd = Lua["OnContactEnd"];
                            
                            if (OnContactBegin.valid() && OnContactEnd.valid())
                            {
                                // PhysicsManager에 콜백 등록
                                GEngine->PhysicsManager->RegisterContactCallback(Owner, OnContactBegin, OnContactEnd);
                                UE_LOG(ELogLevel::Display, TEXT("Contact events bound successfully for Actor: %s"), *Owner->GetName());
                            }
                            else
                            {
                                UE_LOG(ELogLevel::Warning, TEXT("OnContactBegin or OnContactEnd function not found in Lua script"));
                            }
                        }
                    }
                }
            }
        );
    }
}

namespace LuaDebugHelper
{
    /**
     * @brief 바인딩 전 sol::state 의 globals() 키를 TArray<FString>로 캡처
     */
    inline TArray<FString> CaptureGlobalNames(sol::state& Lua)
    {
        TArray<FString> names;
        sol::table G = Lua.globals();
        for (auto& kv : G)
        {
            if (kv.first.is<std::string>())
            {
                names.Add(FString(kv.first.as<std::string>().c_str()));
            }
        }
        return names;
    }

    /**
     * @brief 바인딩 후 globals()에서 before에 없던 새 키만 로그로 출력
     */
    inline void LogNewBindings(sol::state& Lua, const TArray<FString>& Before)
    {
        sol::table G = Lua.globals();
        for (auto& kv : G)
        {
            if (!kv.first.is<std::string>())
                continue;

            FString name = FString(kv.first.as<std::string>().c_str());
            // Before 배열에 포함되지 않은 경우만 출력
            if (!Before.Contains(name))
            {
                UE_LOG(ELogLevel::Error,TEXT("Lua binding added: %s"), *name);
            }
        }
    }
}
