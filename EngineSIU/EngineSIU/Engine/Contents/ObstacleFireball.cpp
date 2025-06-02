#include "ObstacleFireball.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/FObjLoader.h"

AObstacleFireball::AObstacleFireball()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Obstacle/FireBall.obj"));
}
