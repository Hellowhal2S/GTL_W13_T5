#include "PhysicsManager.h"

#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "World/World.h"
#include <thread>


void GameObject::SetRigidBodyType(ERigidBodyType RigidBodyType) const
{
    switch (RigidBodyType)
    {
    case ERigidBodyType::STATIC:
    {
        // TODO: 재생성해야 함, BodySetup은 어디서 받아오지?
        break;
    }
        case ERigidBodyType::DYNAMIC:
    {
        DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
        break;
    }
        case ERigidBodyType::KINEMATIC:
    {
        DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        DynamicRigidBody->setLinearVelocity(PxVec3(0));
        DynamicRigidBody->setAngularVelocity(PxVec3(0));
        break;
    }
    }
}

FPhysicsManager::FPhysicsManager()
{
}

FPhysicsManager::~FPhysicsManager()
{
    ShutdownPhysX();
}

void FPhysicsManager::InitPhysX()
{
    Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback);

    InitializePVD();
    
    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, PxTolerancesScale(), true, Pvd);
    
    Material = Physics->createMaterial(0.5f, 0.7f, 0.1f);

    PxInitExtensions(*Physics, Pvd);
}

void FPhysicsManager::InitializePVD()
{
    if (Pvd) {
        printf("PVD가 이미 초기화됨\n");
        return;
    }

    Pvd = PxCreatePvd(*Foundation);
    if (!Pvd) {
        printf("PVD 생성 실패\n");
        return;
    }

    // Transport 생성 시도
    if (!CreatePVDTransport()) {
        printf("PVD Transport 생성 실패\n");
        return;
    }

    // 연결 시도
    if (!ConnectToPVD()) {
        printf("PVD 연결 실패 - 오프라인 모드로 계속\n");
    }
}

bool FPhysicsManager::CreatePVDTransport()
{
    SAFE_RELEASE_PX(Transport)

    Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    return Transport != nullptr;
}

bool FPhysicsManager::ConnectToPVD()
{
    if (!Pvd || !Transport) {
        return false;
    }

    bool connected = Pvd->connect(*Transport, PxPvdInstrumentationFlag::eALL);
    if (connected) {
        printf("PVD 연결 성공\n");
    } else {
        printf("PVD 연결 실패\n");
    }
    
    return connected;
}

PxScene* FPhysicsManager::CreateScene(UWorld* World)
{
    // 기존 씬 처리
    if (SceneMap.Contains(World)) {
        PxScene* ExistingScene = SceneMap[World];
        
        // PVD 재연결 상태 확인 및 클라이언트 재설정
        if (IsPVDConnected()) {
            SetupPVDForScene(ExistingScene, World);
            printf("기존 씬 PVD 클라이언트 재설정 완료: %s\n", World ? "Valid World" : "NULL World");
        }
        //RefreshPVDClientSettings(ExistingScene);
        // CurrentScene 업데이트
        CurrentScene = ExistingScene;
        return ExistingScene;
    }
    
    PxSceneDesc SceneDesc(Physics->getTolerancesScale());
    ConfigureSceneDesc(SceneDesc);
    
    PxScene* NewScene = Physics->createScene(SceneDesc);
    SceneMap.Add(World, NewScene);
    CurrentScene = NewScene;

    // PVD 클라이언트 생성 및 씬 연결
    if (IsPVDConnected()) {
        SetupPVDForScene(NewScene, World);
        printf("새 씬 PVD 클라이언트 설정 완료: %s\n", World ? "Valid World" : "NULL World");
    } else {
        printf("PVD 연결되지 않음 - 오프라인 모드\n");
    }

    return NewScene;
}

GameObject FPhysicsManager::CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const
{
    GameObject Obj;
    
    PxTransform Pose(Pos);
    Obj.DynamicRigidBody = Physics->createRigidDynamic(Pose);
    
    PxShape* Shape = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    Obj.DynamicRigidBody->attachShape(*Shape);
    
    PxRigidBodyExt::updateMassAndInertia(*Obj.DynamicRigidBody, 10.0f);
    CurrentScene->addActor(*Obj.DynamicRigidBody);
    
    Obj.UpdateFromPhysics(CurrentScene);
    
    return Obj;
}

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetup, ERigidBodyType RigidBodyType) const
{
    GameObject* Obj = new GameObject();
    
    // RigidBodyType에 따라 다른 타입의 Actor 생성
    switch (RigidBodyType)
    {
    case ERigidBodyType::STATIC:
    {
        Obj->StaticRigidBody = CreateStaticRigidBody(Pos, Rot, BodyInstance, BodySetup);
        ApplyBodyInstanceSettings(Obj->StaticRigidBody, BodyInstance);
        break;
    }
    case ERigidBodyType::DYNAMIC:
    {
        Obj->DynamicRigidBody = CreateDynamicRigidBody(Pos, Rot, BodyInstance, BodySetup);
        ApplyBodyInstanceSettings(Obj->DynamicRigidBody, BodyInstance);
        break;
    }
    case ERigidBodyType::KINEMATIC:
    {
        Obj->DynamicRigidBody = CreateDynamicRigidBody(Pos, Rot, BodyInstance, BodySetup);
        Obj->DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        ApplyBodyInstanceSettings(Obj->DynamicRigidBody, BodyInstance);
        break;
    }
    }

    BodyInstance->BIGameObject = Obj;
    
    return Obj;
}

PxRigidDynamic* FPhysicsManager::CreateDynamicRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetup) const
{
    const PxTransform Pose(Pos, Rot);
    PxRigidDynamic* DynamicRigidBody = Physics->createRigidDynamic(Pose);
    
    // Shape 추가
    AttachShapesToActor(DynamicRigidBody, BodySetup);
    
    // 질량과 관성 설정
    ApplyMassAndInertiaSettings(DynamicRigidBody, BodyInstance);
    
    // Scene에 추가
    CurrentScene->addActor(*DynamicRigidBody);
    DynamicRigidBody->userData = (void*)BodyInstance;

    return DynamicRigidBody;
}

PxRigidStatic* FPhysicsManager::CreateStaticRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetup) const
{
    const PxTransform Pose(Pos, Rot);
    PxRigidStatic* StaticRigidBody = Physics->createRigidStatic(Pose);
    
    // Shape 추가
    AttachShapesToActor(StaticRigidBody, BodySetup);
    
    // Scene에 추가
    CurrentScene->addActor(*StaticRigidBody);
    StaticRigidBody->userData = (void*)BodyInstance;

    return StaticRigidBody;
}

// === Shape 추가 헬퍼 함수 ===
void FPhysicsManager::AttachShapesToActor(PxRigidActor* Actor, UBodySetup* BodySetup) const
{
    // Sphere 추가
    for (const auto& Sphere : BodySetup->AggGeom.SphereElems)
    {
    
        Actor->attachShape(*Sphere);
    }

    // Box 추가
    for (const auto& Box : BodySetup->AggGeom.BoxElems)
    {
        Actor->attachShape(*Box);
    }

    // Capsule 추가
    for (const auto& Capsule : BodySetup->AggGeom.CapsuleElems)
    {
        Actor->attachShape(*Capsule);
    }
}

// === 질량과 관성 설정 ===
void FPhysicsManager::ApplyMassAndInertiaSettings(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const
{
    // 기본 질량 설정
    if (BodyInstance->MassInKg > 0.0f)
    {
        PxRigidBodyExt::setMassAndUpdateInertia(*DynamicBody, BodyInstance->MassInKg);
    }
    else
    {
        // 기본 밀도로 질량 계산
        PxRigidBodyExt::updateMassAndInertia(*DynamicBody, 1000.0f); // 기본 밀도
    }
    
    // // 질량 중심 오프셋 적용
    // if (!BodyInstance->COMNudge.IsZero())
    // {
    //     // PxVec3 COMOffset(BodyInstance->COMNudge.X, BodyInstance->COMNudge.Y, BodyInstance->COMNudge.Z);
    //     // PxRigidBodyExt::setMassAndUpdateInertia(*DynamicBody, 1.0f);
    //     
    //     float newMass = 1.0f;
    //     DynamicBody->setMass(newMass);
    //
    //     // 🔸 문제 2: MassSpaceInertiaTensor: 0.000, 0.000, 0.000 해결
    //     // 구체의 경우 (반지름 1.0f 가정)
    //     float radius = 1.0f;
    //     float I = (2.0f / 5.0f) * newMass * radius * radius; // = 0.4f
    //     PxVec3 inertia(I, I, I);
    //     DynamicBody->setMassSpaceInertiaTensor(inertia);
    // }
    //
    // 관성 텐서 스케일 적용
    if (BodyInstance->InertiaTensorScale != FVector::OneVector)
    {
        PxVec3 Inertia = DynamicBody->getMassSpaceInertiaTensor();
        Inertia.x *= BodyInstance->InertiaTensorScale.X;
        Inertia.y *= BodyInstance->InertiaTensorScale.Y;
        Inertia.z *= BodyInstance->InertiaTensorScale.Z;
        DynamicBody->setMassSpaceInertiaTensor(Inertia);
    }
}

// === 모든 BodyInstance 설정 적용 ===
void FPhysicsManager::ApplyBodyInstanceSettings(PxRigidActor* Actor, const FBodyInstance* BodyInstance) const
{
    PxRigidDynamic* DynamicBody = Actor->is<PxRigidDynamic>();
    
    if (DynamicBody)
    {
        // === 시뮬레이션 설정 ===
        
        // 물리 시뮬레이션 활성화/비활성화
        if (!BodyInstance->bSimulatePhysics)
        {
            DynamicBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        }
        
        // 중력 설정
        DynamicBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !BodyInstance->bEnableGravity);
        
        // 시작 상태 설정
        if (!BodyInstance->bStartAwake)
        {
            DynamicBody->putToSleep();
        }
        
        // === 움직임 제한 설정 ===
        ApplyLockConstraints(DynamicBody, BodyInstance);
        
        // === 댐핑 설정 ===
        DynamicBody->setLinearDamping(BodyInstance->LinearDamping);
        DynamicBody->setAngularDamping(BodyInstance->AngularDamping);
        
        // === 고급 물리 설정 ===
        
        // 연속 충돌 검출 (CCD)
        DynamicBody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, BodyInstance->bUseCCD);
        
        // 최대 각속도 제한
        if (BodyInstance->MaxAngularVelocity > 0.0f)
        {
            DynamicBody->setMaxAngularVelocity(BodyInstance->MaxAngularVelocity);
        }
        
        // 솔버 반복 횟수 설정
        DynamicBody->setSolverIterationCounts(
            BodyInstance->PositionSolverIterationCount, 
            BodyInstance->VelocitySolverIterationCount
        );
    }
    
    // === 충돌 설정 (Static/Dynamic 공통) ===
    ApplyCollisionSettings(Actor, BodyInstance);
}

// === 움직임 제한 적용 ===
void FPhysicsManager::ApplyLockConstraints(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const
{
    PxRigidDynamicLockFlags LockFlags = PxRigidDynamicLockFlags(0);
    
    // 이동 제한
    if (BodyInstance->bLockXTranslation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
    if (BodyInstance->bLockYTranslation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
    if (BodyInstance->bLockZTranslation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
    
    // 회전 제한
    if (BodyInstance->bLockXRotation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
    if (BodyInstance->bLockYRotation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
    if (BodyInstance->bLockZRotation) LockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
    
    DynamicBody->setRigidDynamicLockFlags(LockFlags);
}

// === 충돌 설정 적용 ===
void FPhysicsManager::ApplyCollisionSettings(const PxRigidActor* Actor, const FBodyInstance* BodyInstance) const
{
    // 모든 Shape에 대해 충돌 설정 적용
    PxU32 ShapeCount = Actor->getNbShapes();
    TArray<PxShape*> Shapes;
    Shapes.SetNum(ShapeCount);
    Actor->getShapes(Shapes.GetData(), ShapeCount);
    
    for (PxShape* Shape : Shapes)
    {
        if (Shape)
        {
            ApplyShapeCollisionSettings(Shape, BodyInstance);
        }
    }
}

void FPhysicsManager::ApplyShapeCollisionSettings(PxShape* Shape, const FBodyInstance* BodyInstance) const
{
    PxShapeFlags ShapeFlags = Shape->getFlags();
    
    // 기존 플래그 초기화
    ShapeFlags &= ~(PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eTRIGGER_SHAPE);
    
    // CollisionEnabled에 따른 플래그 설정
    switch (BodyInstance->CollisionEnabled)
    {
    case ECollisionEnabled::NoCollision:
        // 모든 충돌 비활성화
        break;
        
    case ECollisionEnabled::QueryOnly:
        // 쿼리만 활성화 (트레이스, 오버랩)
        ShapeFlags |= PxShapeFlag::eSCENE_QUERY_SHAPE;
        break;
        
    case ECollisionEnabled::PhysicsOnly:
        // 물리 시뮬레이션만 활성화
        ShapeFlags |= PxShapeFlag::eSIMULATION_SHAPE;
        break;
        
    case ECollisionEnabled::QueryAndPhysics:
        // 둘 다 활성화
        ShapeFlags |= (PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE);
        break;
    }
    
    // 복잡한 충돌을 단순 충돌로 사용 설정
    if (BodyInstance->bUseComplexAsSimpleCollision)
    {
        // 복잡한 메시를 단순 충돌로 사용하는 로직
        // (구체적인 구현은 메시 충돌 시스템에 따라 다름)
    }
    
    Shape->setFlags(ShapeFlags);
}

// // === 런타임 설정 변경 함수들 === TODO : 필요하면 GameObject 안으로 옮기기
//
// void FPhysicsManager::SetSimulatePhysics(GameObject* Obj, bool bSimulate) const
// {
//     if (Obj && Obj->DynamicRigidBody)
//     {
//         Obj->DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, !bSimulate);
//     }
// }
//
// void FPhysicsManager::SetEnableGravity(GameObject* Obj, bool bEnableGravity) const
// {
//     if (Obj && Obj->DynamicRigidBody)
//     {
//         Obj->DynamicRigidBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !bEnableGravity);
//     }
// }
//
// void FPhysicsManager::SetMass(GameObject* Obj, float NewMass) const
// {
//     if (Obj && Obj->DynamicRigidBody && NewMass > 0.0f)
//     {
//         PxRigidBodyExt::setMassAndUpdateInertia(*Obj->DynamicRigidBody, NewMass);
//     }
// }
//
// void FPhysicsManager::SetLinearDamping(GameObject* Obj, float Damping) const
// {
//     if (Obj && Obj->DynamicRigidBody)
//     {
//         Obj->DynamicRigidBody->setLinearDamping(Damping);
//     }
// }
//
// void FPhysicsManager::SetAngularDamping(GameObject* Obj, float Damping) const
// {
//     if (Obj && Obj->DynamicRigidBody)
//     {
//         Obj->DynamicRigidBody->setAngularDamping(Damping);
//     }
// }

void FPhysicsManager::CreateJoint(const GameObject* Obj1, const GameObject* Obj2, FConstraintInstance* ConstraintInstance, const FConstraintSetup* ConstraintSetup) const
{
    PxTransform GlobalPose1 = Obj1->DynamicRigidBody->getGlobalPose();
    PxTransform GlobalPose2 = Obj2->DynamicRigidBody->getGlobalPose();

    PxQuat AxisCorrection = PxQuat(PxMat33(
        PxVec3(0.0f, 0.0f, 1.0f),
        PxVec3(1.0f, 0.0f, 0.0f),
        PxVec3(0.0f, 1.0f, 0.0f)
    ));
    AxisCorrection.normalize();

    PxTransform LocalFrameChild(PxVec3(0.0f), AxisCorrection);

    PxTransform ChildJointFrameInWorld = GlobalPose2 * LocalFrameChild;
    PxTransform LocalFrameParent = GlobalPose1.getInverse() * ChildJointFrameInWorld;

    // PhysX D6 Joint 생성
    PxD6Joint* Joint = PxD6JointCreate(*Physics,
        Obj1->DynamicRigidBody, LocalFrameParent,
        Obj2->DynamicRigidBody, LocalFrameChild);

    if (Joint && ConstraintSetup)
    {
        // === Linear Constraint 설정 ===
        
        // X축 선형 제약
        switch (ConstraintSetup->LinearLimit.XMotion)
        {
        case ELinearConstraintMotion::LCM_Free:
            Joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
            break;
        case ELinearConstraintMotion::LCM_Limited:
            Joint->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);
            break;
        case ELinearConstraintMotion::LCM_Locked:
            Joint->setMotion(PxD6Axis::eX, PxD6Motion::eLOCKED);
            break;
        }

        // Y축 선형 제약
        switch (ConstraintSetup->LinearLimit.YMotion)
        {
        case ELinearConstraintMotion::LCM_Free:
            Joint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
            break;
        case ELinearConstraintMotion::LCM_Limited:
            Joint->setMotion(PxD6Axis::eY, PxD6Motion::eLIMITED);
            break;
        case ELinearConstraintMotion::LCM_Locked:
            Joint->setMotion(PxD6Axis::eY, PxD6Motion::eLOCKED);
            break;
        }

        // Z축 선형 제약
        switch (ConstraintSetup->LinearLimit.ZMotion)
        {
        case ELinearConstraintMotion::LCM_Free:
            Joint->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);
            break;
        case ELinearConstraintMotion::LCM_Limited:
            Joint->setMotion(PxD6Axis::eZ, PxD6Motion::eLIMITED);
            break;
        case ELinearConstraintMotion::LCM_Locked:
            Joint->setMotion(PxD6Axis::eZ, PxD6Motion::eLOCKED);
            break;
        }

        // 선형 제한 값 설정 (Limited 모션이 하나라도 있을 때)
        if (ConstraintSetup->LinearLimit.XMotion == ELinearConstraintMotion::LCM_Limited ||
            ConstraintSetup->LinearLimit.YMotion == ELinearConstraintMotion::LCM_Limited ||
            ConstraintSetup->LinearLimit.ZMotion == ELinearConstraintMotion::LCM_Limited)
        {
            float LinearLimitValue = ConstraintSetup->LinearLimit.Limit;
            Joint->setLinearLimit(PxJointLinearLimit(LinearLimitValue, PxSpring(0, 0)));
        }

        // === Angular Constraint 설정 ===

        // Twist 제약 (X축 회전)
        switch (ConstraintSetup->TwistLimit.TwistMotion)
        {
        case EAngularConstraintMotion::EAC_Free:
            Joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
            break;
        case EAngularConstraintMotion::EAC_Limited:
            Joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
            break;
        case EAngularConstraintMotion::EAC_Locked:
            Joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
            break;
        }

        // Swing1 제약 (Y축 회전)
        switch (ConstraintSetup->ConeLimit.Swing1Motion)
        {
        case EAngularConstraintMotion::EAC_Free:
            Joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
            break;
        case EAngularConstraintMotion::EAC_Limited:
            Joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
            break;
        case EAngularConstraintMotion::EAC_Locked:
            Joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
            break;
        }

        // Swing2 제약 (Z축 회전)
        switch (ConstraintSetup->ConeLimit.Swing2Motion)
        {
        case EAngularConstraintMotion::EAC_Free:
            Joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
            break;
        case EAngularConstraintMotion::EAC_Limited:
            Joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
            break;
        case EAngularConstraintMotion::EAC_Locked:
            Joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);
            break;
        }

        // === Angular Limit 값 설정 ===

        // Twist 제한 값 설정 (도를 라디안으로 변환)
        if (ConstraintSetup->TwistLimit.TwistMotion == EAngularConstraintMotion::EAC_Limited)
        {
            float TwistLimitRad = FMath::DegreesToRadians(ConstraintSetup->TwistLimit.TwistLimitDegrees);
            Joint->setTwistLimit(PxJointAngularLimitPair(-TwistLimitRad, TwistLimitRad, PxSpring(0, 0)));
        }

        // Swing 제한 값 설정 (원뿔 제한)
        if (ConstraintSetup->ConeLimit.Swing1Motion == EAngularConstraintMotion::EAC_Limited ||
            ConstraintSetup->ConeLimit.Swing2Motion == EAngularConstraintMotion::EAC_Limited)
        {
            float Swing1LimitRad = FMath::DegreesToRadians(ConstraintSetup->ConeLimit.Swing1LimitDegrees);
            float Swing2LimitRad = FMath::DegreesToRadians(ConstraintSetup->ConeLimit.Swing2LimitDegrees);
            Joint->setSwingLimit(PxJointLimitCone(Swing1LimitRad, Swing2LimitRad, PxSpring(0, 0)));
        }

        // === Drive/Motor 설정 ===

        // Linear Position Drive
        if (ConstraintSetup->bLinearPositionDrive)
        {
            PxD6JointDrive LinearDrive;
            LinearDrive.stiffness = 1000.0f;     // 강성 (조정 가능)
            LinearDrive.damping = 100.0f;        // 댐핑 (조정 가능)
            LinearDrive.forceLimit = PX_MAX_F32;  // 힘 제한
            LinearDrive.flags = PxD6JointDriveFlag::eACCELERATION;

            Joint->setDrive(PxD6Drive::eX, LinearDrive);
            Joint->setDrive(PxD6Drive::eY, LinearDrive);
            Joint->setDrive(PxD6Drive::eZ, LinearDrive);
        }

        // Angular Position Drive (Slerp)
        if (ConstraintSetup->bAngularOrientationDrive)
        {
            PxD6JointDrive AngularDrive;
            AngularDrive.stiffness = 1000.0f;
            AngularDrive.damping = 100.0f;
            AngularDrive.forceLimit = PX_MAX_F32;
            AngularDrive.flags = PxD6JointDriveFlag::eACCELERATION;

            Joint->setDrive(PxD6Drive::eSLERP, AngularDrive);
        }

        // Angular Velocity Drive
        if (ConstraintSetup->bAngularVelocityDrive)
        {
            PxD6JointDrive AngularVelDrive;
            AngularVelDrive.stiffness = 0.0f;     // 속도 제어시 강성은 0
            AngularVelDrive.damping = 100.0f;
            AngularVelDrive.forceLimit = PX_MAX_F32;
            AngularVelDrive.flags = PxD6JointDriveFlag::eACCELERATION;

            Joint->setDrive(PxD6Drive::eTWIST, AngularVelDrive);
            Joint->setDrive(PxD6Drive::eSWING, AngularVelDrive);
        }

        // === 기본 조인트 설정 ===
        
        // 조인트 이름 설정 (디버깅용)
        if (!ConstraintSetup->JointName.IsEmpty())
        {
            Joint->setName(GetData(ConstraintSetup->JointName));
        }

        // 조인트 활성화
        Joint->setConstraintFlags(PxConstraintFlag::eVISUALIZATION);
        
        // 파괴 임계값 설정 (선택사항)
        Joint->setBreakForce(PX_MAX_F32, PX_MAX_F32);  // 무한대로 설정하여 파괴되지 않음

        Joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
        Joint->setProjectionLinearTolerance(0.01f);
        Joint->setProjectionAngularTolerance(0.017f);
    }

    ConstraintInstance->ConstraintData = Joint;
}

void FPhysicsManager::DestroyGameObject(GameObject* GameObject) const
{
    // TODO: StaticRigidBody 분기 처리 필요
    if (GameObject && GameObject->DynamicRigidBody)
    {
        CurrentScene->removeActor(*GameObject->DynamicRigidBody);
        GameObject->DynamicRigidBody->release();
        GameObject->DynamicRigidBody = nullptr;
    }
    delete GameObject;
}

PxShape* FPhysicsManager::CreateBoxShape(const PxVec3& Pos, const PxQuat& Quat, const PxVec3& HalfExtents) const
{
    // Box 모양 생성
    PxShape* Result = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    
    // 위치와 회전을 모두 적용한 Transform 생성
    PxTransform LocalTransform(Pos, Quat);
    Result->setLocalPose(LocalTransform);
    
    return Result;
}

PxShape* FPhysicsManager::CreateSphereShape(const PxVec3& Pos, const PxQuat& Quat, float Radius) const
{
    // Sphere 모양 생성 (구는 회전에 영향받지 않지만 일관성을 위해 적용)
    PxShape* Result = Physics->createShape(PxSphereGeometry(Radius), *Material);
    
    // 위치와 회전을 모두 적용한 Transform 생성
    PxTransform LocalTransform(Pos, Quat);
    Result->setLocalPose(LocalTransform);
    
    return Result;
}

PxShape* FPhysicsManager::CreateCapsuleShape(const PxVec3& Pos, const PxQuat& Quat, float Radius, float HalfHeight) const
{
    // Capsule 모양 생성
    PxShape* Result = Physics->createShape(PxCapsuleGeometry(Radius, HalfHeight), *Material);
    
    // 위치와 회전을 모두 적용한 Transform 생성
    PxTransform LocalTransform(Pos, Quat);
    Result->setLocalPose(LocalTransform);
    
    return Result;
}

void FPhysicsManager::Simulate(float DeltaTime)
{
    if (CurrentScene)
    {
        QUICK_SCOPE_CYCLE_COUNTER(SimulatePass_CPU)
        CurrentScene->simulate(DeltaTime);
        CurrentScene->fetchResults(true);
    }
}

void FPhysicsManager::ShutdownPhysX()
{
    // 모든 씬 정리 (PVD 포함)
    CleanupAllScenes();
    
    if(Dispatcher)
    {
        Dispatcher->release();
        Dispatcher = nullptr;
    }
    if(Material)
    {
        Material->release();
        Material = nullptr;
    }
    
    // PVD 정리
    CleanupPVD();
    
    if(Physics)
    {
        Physics->release();
        Physics = nullptr;
    }
    if(Foundation)
    {
        Foundation->release();
        Foundation = nullptr;
    }
}

void FPhysicsManager::CleanupPVD() {
    if (Pvd) {
        if (Pvd->isConnected()) {
            Pvd->disconnect();
        }
        if (Transport) {
            Transport->release();
            Transport = nullptr;
        }
        Pvd->release();
        Pvd = nullptr;
    }
}

void FPhysicsManager::CleanupScene()
{
    // CurrentScene만 정리 (SceneMap은 건드리지 않음)
    if (CurrentScene) {
        printf("현재 활성 씬 정리\n");
        CurrentScene = nullptr;
    }
}

// 새로운 PVD 정리 전용 함수
void FPhysicsManager::CleanupScenePVD(PxScene* Scene, UWorld* World)
{
    if (!Scene || !IsPVDConnected()) {
        return;
    }

    PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();
    if (pvdClient) {
        // PVD에서 씬 전송 중단
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, false);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, false);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, false);
        
        printf("씬 PVD 클라이언트 정리 완료: %s\n", World ? "Valid World" : "NULL World");
    }
}

// 모든 씬 정리 함수 (ShutdownPhysX에서 사용)
void FPhysicsManager::CleanupAllScenes()
{
    printf("모든 씬 정리 시작\n");
    
    // 모든 씬에 대해 PVD 정리 후 해제
    for (auto& ScenePair : SceneMap) {
        PxScene* Scene = ScenePair.Value;
        UWorld* World = ScenePair.Key;
        
        if (Scene) {
            CleanupScenePVD(Scene, World);
            Scene->release();
        }
    }
    
    // SceneMap 정리
    SceneMap.Empty();
    CurrentScene = nullptr;
    
    printf("모든 씬 정리 완료\n");
}

void FPhysicsManager::ConfigureSceneDesc(PxSceneDesc& SceneDesc)
{
    SceneDesc.gravity = PxVec3(0, 0, -9.81f);
    
    unsigned int hc = std::thread::hardware_concurrency();
    Dispatcher = PxDefaultCpuDispatcherCreate(hc-2);
    SceneDesc.cpuDispatcher = Dispatcher;
    
    SceneDesc.filterShader = PxDefaultSimulationFilterShader;
    
    SceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    SceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    SceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
}

// PVD 관련 함수들 구현
void FPhysicsManager::SetupPVDForScene(PxScene* Scene, UWorld* World)
{
    if (!Scene || !IsPVDConnected()) {
        return;
    }

    PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();
    if (!pvdClient) {
        printf("PVD 클라이언트 생성 실패: %s\n", World ? "Valid World" : "NULL World");
        return;
    }

    // 모든 전송 플래그 활성화
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    
    printf("씬 PVD 클라이언트 설정 완료: %s\n", World ? "Valid World" : "NULL World");
}

void FPhysicsManager::RefreshPVDClientSettings(PxScene* Scene)
{
    if (!Scene || !IsPVDConnected()) {
        return;
    }

    PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();
    if (pvdClient) {
        // 기존 설정 리프레시
        SetupPVDForScene(Scene, nullptr);
        printf("기존 씬 PVD 클라이언트 재설정 완료\n");
    }
}

void FPhysicsManager::ReconnectPVD()
{
    if (!Pvd) {
        printf("PVD가 초기화되지 않음\n");
        return;
    }

    // 기존 연결 정리
    if (Pvd->isConnected()) {
        Pvd->disconnect();
        printf("기존 PVD 연결 해제\n");
    }

    // 재연결 시도
    if (ConnectToPVD()) {
        // 모든 기존 씬에 대해 PVD 클라이언트 재설정
        for (auto& ScenePair : SceneMap) {
            SetupPVDForScene(ScenePair.Value, ScenePair.Key);
        }
        printf("PVD 재연결 및 씬 재설정 완료\n");
    }
}

void FPhysicsManager::ResetPVDSimulation()
{
    if (!IsPVDConnected()) {
        printf("PVD가 연결되지 않음 - 리셋 불가\n");
        return;
    }

    printf("PVD 시뮬레이션 리셋 시작\n");

    // 1. 모든 씬의 PVD 클라이언트 정리
    for (auto& ScenePair : SceneMap) {
        CleanupScenePVD(ScenePair.Value, ScenePair.Key);
    }

    // 2. PVD 연결 해제 및 재연결 (프레임 카운터 리셋을 위해)
    if (Pvd->isConnected()) {
        Pvd->disconnect();
        printf("PVD 연결 해제\n");
    }

    // 3. 재연결 (새로운 시뮬레이션 세션 시작)
    if (ConnectToPVD()) {
        printf("PVD 재연결 완료 - 새로운 시뮬레이션 세션 시작\n");
        
        // 4. 기존 씬들에 대해 PVD 클라이언트 재설정
        for (auto& ScenePair : SceneMap) {
            SetupPVDForScene(ScenePair.Value, ScenePair.Key);
        }
        
        printf("모든 씬의 PVD 클라이언트 재설정 완료\n");
    } else {
        printf("PVD 재연결 실패\n");
    }
}

void FPhysicsManager::RemoveScene(UWorld* World)
{
    if (!World || !SceneMap.Contains(World)) {
        return;
    }
    
    PxScene* Scene = SceneMap[World];
    if (!Scene) {
        SceneMap.Remove(World);
        return;
    }

    // PVD 클라이언트 정리
    CleanupScenePVD(Scene, World);
    
    // CurrentScene이 제거될 씬과 같다면 nullptr로 설정
    if (CurrentScene == Scene) {
        CurrentScene = nullptr;
    }
    
    // 씬 해제
    Scene->release();
    
    // SceneMap에서 제거
    SceneMap.Remove(World);
    
    printf("씬 제거 완료: %s\n", World ? "Valid World" : "NULL World");
}

// === 토크와 힘 적용 함수들 구현 ===

PxForceMode::Enum FPhysicsManager::ConvertForceMode(int ForceMode) const
{
    switch (ForceMode)
    {
    case 0: return PxForceMode::eFORCE;          // 연속적인 힘 (질량 고려)
    case 1: return PxForceMode::eIMPULSE;        // 즉시 충격 (질량 고려)
    case 2: return PxForceMode::eVELOCITY_CHANGE; // 속도 변화 (질량 무시)
    case 3: return PxForceMode::eACCELERATION;   // 가속도 (질량 무시)
    default: return PxForceMode::eFORCE;
    }
}

void FPhysicsManager::ApplyTorque(GameObject* Obj, const FVector& Torque, int ForceMode)
{
    if (!Obj || !Obj->DynamicRigidBody) return;
    
    PxVec3 PhysXTorque(Torque.X, -Torque.Y, Torque.Z); // Y축 반전 (언리얼->PhysX 좌표계)
    PxForceMode::Enum PhysXForceMode = ConvertForceMode(ForceMode);
    
    Obj->DynamicRigidBody->addTorque(PhysXTorque, PhysXForceMode);
}

void FPhysicsManager::ApplyTorqueToActor(AActor* Actor, const FVector& Torque, int ForceMode)
{
    GameObject* Obj = FindGameObjectByActor(Actor);
    if (Obj)
    {
        ApplyTorque(Obj, Torque, ForceMode);
    }
}

void FPhysicsManager::ApplyForce(GameObject* Obj, const FVector& Force, int ForceMode)
{
    if (!Obj || !Obj->DynamicRigidBody) return;
    
    PxVec3 PhysXForce(Force.X, -Force.Y, Force.Z); // Y축 반전 (언리얼->PhysX 좌표계)
    PxForceMode::Enum PhysXForceMode = ConvertForceMode(ForceMode);
    
    Obj->DynamicRigidBody->addForce(PhysXForce, PhysXForceMode);
}

void FPhysicsManager::ApplyForceToActor(AActor* Actor, const FVector& Force, int ForceMode)
{
    GameObject* Obj = FindGameObjectByActor(Actor);
    if (Obj)
    {
        ApplyForce(Obj, Force, ForceMode);
    }
}

void FPhysicsManager::ApplyForceAtPosition(GameObject* Obj, const FVector& Force, const FVector& Position, int ForceMode)
{
    if (!Obj || !Obj->DynamicRigidBody) return;
    
    PxVec3 PhysXForce(Force.X, -Force.Y, Force.Z);
    PxVec3 PhysXPosition(Position.X, -Position.Y, Position.Z);
    PxForceMode::Enum PhysXForceMode = ConvertForceMode(ForceMode);
    
    PxRigidBodyExt::addForceAtPos(*Obj->DynamicRigidBody, PhysXForce, PhysXPosition, PhysXForceMode);
}

void FPhysicsManager::ApplyForceAtPositionToActor(AActor* Actor, const FVector& Force, const FVector& Position, int ForceMode)
{
    GameObject* Obj = FindGameObjectByActor(Actor);
    if (Obj)
    {
        ApplyForceAtPosition(Obj, Force, Position, ForceMode);
    }
}

void FPhysicsManager::ApplyJumpImpulse(GameObject* Obj, float JumpForce)
{
    if (!Obj || !Obj->DynamicRigidBody) return;
    
    // 위쪽 방향으로 충격 적용
    PxVec3 JumpImpulse(0.0f, 0.0f, JumpForce);
    Obj->DynamicRigidBody->addForce(JumpImpulse, PxForceMode::eIMPULSE);
}

void FPhysicsManager::ApplyJumpImpulseToActor(AActor* Actor, float JumpForce)
{
    GameObject* Obj = FindGameObjectByActor(Actor);
    if (Obj)
    {
        ApplyJumpImpulse(Obj, JumpForce);
    }
}

GameObject* FPhysicsManager::FindGameObjectByActor(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    // Actor의 PrimitiveComponent를 찾아서 BodyInstance를 통해 GameObject 찾기
    for (auto* Component : Actor->GetComponents())
    {
        if (auto* PrimComp = Cast<class UPrimitiveComponent>(Component))
        {
            if (PrimComp->BodyInstance && PrimComp->BodyInstance->BIGameObject)
            {
                return PrimComp->BodyInstance->BIGameObject;
            }
        }
    }
    
    return nullptr;
}
