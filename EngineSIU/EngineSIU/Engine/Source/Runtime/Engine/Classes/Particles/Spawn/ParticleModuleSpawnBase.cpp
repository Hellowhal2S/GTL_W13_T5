#include "ParticleModuleSpawnBase.h"

void UParticleModuleSpawnBase::DisplayProperty()
{
    Super::DisplayProperty();
    for (const auto& Property : StaticClass()->GetProperties())
    {
        Property->DisplayInImGui(this);
    }
}

void UParticleModuleSpawnBase::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);

    Ar << bProcessSpawnRate << bProcessBurstList;
}
