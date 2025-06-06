#pragma once

#include "Core/HAL/PlatformType.h" // TCHAR 재정의 문제때문에 다른 헤더들보다 앞에 있어야 함

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>
#include <pvd/PxPvd.h>

#include "Container/Array.h"
#include "Container/Map.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "sol/forward.hpp"


enum class ERigidBodyType : uint8;
struct FBodyInstance;
class UBodySetup;
class UWorld;

using namespace physx;
using namespace DirectX;

class UPrimitiveComponent;

// 게임 오브젝트
struct GameObject {
    PxRigidDynamic* DynamicRigidBody = nullptr;
    PxRigidStatic* StaticRigidBody = nullptr;
    XMMATRIX WorldMatrix = XMMatrixIdentity();
    
    // 삭제 상태 추적을 위한 플래그
    mutable bool bIsBeingDestroyed = false;
    mutable bool bIsDestroyed = false;
    
    // 고유 ID (디버깅용)
    static uint32 NextID;
    uint32 ID;
    
    GameObject() : ID(++NextID) {}

    void UpdateFromPhysics(PxScene* Scene) {
        if (bIsDestroyed || !DynamicRigidBody) return;
        
        PxSceneReadLock scopedReadLock(*Scene);
        PxTransform t = DynamicRigidBody->getGlobalPose();
        PxMat44 mat(t);
        WorldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&mat));
    }

    void SetRigidBodyType(ERigidBodyType RigidBody) const;
    
    // 안전한 삭제를 위한 헬퍼 함수
    bool IsValid() const {
        return !bIsDestroyed && !bIsBeingDestroyed && (DynamicRigidBody || StaticRigidBody);
    }
    
    void MarkForDestruction() const {
        bIsBeingDestroyed = true;
    }
    
    void MarkDestroyed() {
        bIsDestroyed = true;
         DynamicRigidBody = nullptr;
        StaticRigidBody = nullptr;
    }
};

// PhysX Contact 이벤트 콜백 클래스
class FPhysXContactCallback : public PxSimulationEventCallback
{
public:
    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {}
    virtual void onWake(PxActor** actors, PxU32 count) override {}
    virtual void onSleep(PxActor** actors, PxU32 count) override {}
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override {}
    virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}
    
    // Contact 이벤트 처리
    virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
};

class FPhysicsManager
{
public:
    FPhysicsManager();
    ~FPhysicsManager();

    void InitPhysX();
    
    PxScene* CreateScene(UWorld* World);
    PxScene* GetScene(UWorld* World) { return SceneMap[World]; }
    void RemoveScene(UWorld* World);
    void SetCurrentScene(UWorld* World) { CurrentScene = SceneMap[World]; }
    void SetCurrentScene(PxScene* Scene) { CurrentScene = Scene; }
    
    void DestroyGameObject(GameObject* GameObject) const;
    
    // 토크와 힘 적용 함수들
    void ApplyTorque(GameObject* Obj, const FVector& Torque, int ForceMode = 0);
    void ApplyTorqueToActor(class AActor* Actor, const FVector& Torque, int ForceMode = 0);
    void ApplyForce(GameObject* Obj, const FVector& Force, int ForceMode = 0);
    void ApplyForceToActor(class AActor* Actor, const FVector& Force, int ForceMode = 0);
    void ApplyForceAtPosition(GameObject* Obj, const FVector& Force, const FVector& Position, int ForceMode = 0);
    void ApplyForceAtPositionToActor(class AActor* Actor, const FVector& Force, const FVector& Position, int ForceMode = 0);
    void ApplyJumpImpulse(GameObject* Obj, float JumpForce);
    void ApplyJumpImpulseToActor(class AActor* Actor, float JumpForce);

    // 직접 속도 제어 함수들 (질량 무시)
    void SetLinearVelocity(GameObject* Obj, const FVector& Velocity);
    void SetLinearVelocityToActor(class AActor* Actor, const FVector& Velocity);
    void AddLinearVelocity(GameObject* Obj, const FVector& Velocity);
    void AddLinearVelocityToActor(class AActor* Actor, const FVector& Velocity);
    void SetAngularVelocity(GameObject* Obj, const FVector& AngularVelocity);
    void SetAngularVelocityToActor(class AActor* Actor, const FVector& AngularVelocity);
    void AddAngularVelocity(GameObject* Obj, const FVector& AngularVelocity);
    void AddAngularVelocityToActor(class AActor* Actor, const FVector& AngularVelocity);
    
    // 속도 얻기 함수들
    FVector GetLinearVelocity(GameObject* Obj);
    FVector GetLinearVelocityFromActor(class AActor* Actor);
    FVector GetAngularVelocity(GameObject* Obj);
    FVector GetAngularVelocityFromActor(class AActor* Actor);

    void GrowBall(AActor* Actor, float DeltaRadius);
    
    GameObject* FindGameObjectByActor(class AActor* Actor);
    
    GameObject CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const;
    GameObject* CreateGameObject(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetup, ERigidBodyType RigidBodyType =
                                     ERigidBodyType::DYNAMIC) const;
    void CreateJoint(const GameObject* Obj1, const GameObject* Obj2, FConstraintInstance* ConstraintInstance, const FConstraintSetup* ConstraintSetup) const;

    PxShape* CreateBoxShape(const PxVec3& Pos, const PxQuat& Quat, const PxVec3& HalfExtents) const;
    PxShape* CreateSphereShape(const PxVec3& Pos, const PxQuat& Quat, float Radius) const;
    PxShape* CreateCapsuleShape(const PxVec3& Pos, const PxQuat& Quat, float Radius, float HalfHeight) const;
    PxQuat EulerToQuat(const PxVec3& EulerAngles) const;

    PxPhysics* GetPhysics() { return Physics; }
    PxMaterial* GetMaterial() const { return Material; }
    
    void Simulate(float DeltaTime);
    void ShutdownPhysX();
    void CleanupPVD();
    void CleanupScene();

    // PVD 관리 함수들
    bool IsPVDConnected() const { return Pvd && Pvd->isConnected(); }
    void ReconnectPVD();
    void ResetPVDSimulation(); // PIE 재시작 시 PVD 프레임 카운터 리셋

    // Contact 이벤트 콜백 관리
    void RegisterContactCallback(class AActor* Actor, sol::function OnContactBegin, sol::function OnContactEnd);
    void UnregisterContactCallback(class AActor* Actor);
    
    // Contact 이벤트 트리거 (내부 사용)
    void TriggerContactBegin(class AActor* Actor, class AActor* OtherActor, const FVector& ContactPoint);
    void TriggerContactEnd(class AActor* Actor, class AActor* OtherActor, const FVector& ContactPoint);

private:
    PxDefaultAllocator Allocator;
    PxDefaultErrorCallback ErrorCallback;
    PxFoundation* Foundation = nullptr;
    PxPhysics* Physics = nullptr;
    TMap<UWorld*, PxScene*> SceneMap;
    PxScene* CurrentScene = nullptr;
    PxMaterial* Material = nullptr;
    PxDefaultCpuDispatcher* Dispatcher = nullptr;
    
    // Contact 이벤트 콜백
    FPhysXContactCallback ContactCallback;
    
    // Actor별 Contact 콜백 함수들
    TMap<class AActor*, TPair<sol::function, sol::function>> ContactCallbacks;
    
    // PVD 관련 변수들 (간소화)
    PxPvd* Pvd = nullptr;
    PxPvdTransport* Transport = nullptr;

    // PVD 내부 함수들
    void InitializePVD();
    bool CreatePVDTransport();
    bool ConnectToPVD();
    void SetupPVDForScene(PxScene* Scene, UWorld* World);
    void RefreshPVDClientSettings(PxScene* Scene);
    void ConfigureSceneDesc(PxSceneDesc& SceneDesc);
    
    // 씬 정리 함수들
    void CleanupScenePVD(PxScene* Scene, UWorld* World);
    void CleanupAllScenes();

    PxRigidDynamic* CreateDynamicRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
    PxRigidStatic* CreateStaticRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
    void AttachShapesToActor(PxRigidActor* Actor, UBodySetup* BodySetup) const;
    void ApplyMassAndInertiaSettings(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const;
    void ApplyBodyInstanceSettings(PxRigidActor* Actor, const FBodyInstance* BodyInstance) const;
    void ApplyLockConstraints(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const;
    void ApplyCollisionSettings(const PxRigidActor* Actor, const FBodyInstance* BodyInstance) const;
    void ApplyShapeCollisionSettings(PxShape* Shape, const FBodyInstance* BodyInstance) const;
    
    // ForceMode 변환 헬퍼 함수
    PxForceMode::Enum ConvertForceMode(int ForceMode) const;
};

