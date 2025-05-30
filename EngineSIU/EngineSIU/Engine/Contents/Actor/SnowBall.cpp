#include "SnowBall.h"

#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASnowBall::ASnowBall()
{
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SetRootComponent(SnowBallComponent);

    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    
    SphereCollision = AddComponent<USphereComponent>("Collision");
    SphereCollision->SetupAttachment(SnowBallComponent);

    SpringArmComponent = AddComponent<USpringArmComponent>("SpringArm");
    SpringArmComponent->SetupAttachment(SnowBallComponent);
    CameraComponent = AddComponent<UCameraComponent>("Camera");
    CameraComponent->SetupAttachment(SpringArmComponent);
}
