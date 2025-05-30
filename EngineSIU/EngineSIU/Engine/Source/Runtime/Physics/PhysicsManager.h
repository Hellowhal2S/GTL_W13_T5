#pragma once

#include "Core/HAL/PlatformType.h" // TCHAR 재정의 문제때문에 다른 헤더들보다 앞에 있어야 함

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>
#include <pvd/PxPvd.h>

#include "Container/Array.h"
#include "Container/Map.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"


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

    void UpdateFromPhysics(PxScene* Scene) {
        PxSceneReadLock scopedReadLock(*Scene);
        PxTransform t = DynamicRigidBody->getGlobalPose();
        PxMat44 mat(t);
        WorldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&mat));
    }

    void SetRigidBodyType(ERigidBodyType RigidBody) const;
};

class FPhysicsManager
{
public:
    FPhysicsManager();
    ~FPhysicsManager() = default;

    void InitPhysX();
    
    PxScene* CreateScene(UWorld* World);
    PxScene* GetScene(UWorld* World) { return SceneMap[World]; }
    void RemoveScene(UWorld* World);
    void SetCurrentScene(UWorld* World) { CurrentScene = SceneMap[World]; }
    void SetCurrentScene(PxScene* Scene) { CurrentScene = Scene; }
    
    void DestroyGameObject(GameObject* GameObject) const;
    
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

private:
    PxDefaultAllocator Allocator;
    PxDefaultErrorCallback ErrorCallback;
    PxFoundation* Foundation = nullptr;
    PxPhysics* Physics = nullptr;
    TMap<UWorld*, PxScene*> SceneMap;
    PxScene* CurrentScene = nullptr;
    PxMaterial* Material = nullptr;
    PxDefaultCpuDispatcher* Dispatcher = nullptr;
    
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
};

