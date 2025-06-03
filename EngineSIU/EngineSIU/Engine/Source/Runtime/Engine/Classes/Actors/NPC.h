#pragma once
#include "GameFramework/Actor.h"

class USphereTriggerComponent;
class USkeletalMeshComponent;
class ULuaScriptAnimInstance;
class UPrimitiveComponent;

class ANPC : public AActor
{
    DECLARE_CLASS(ANPC, AActor);
    
public:
    ANPC();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void PostSpawnInitialize() override;
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, USphereTriggerComponent*, TriggerComponent, = nullptr)
    UPROPERTY(EditAnywhere, USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr)
    UPROPERTY(EditAnywhere, ULuaScriptAnimInstance*, AnimInstance, = nullptr )
};

