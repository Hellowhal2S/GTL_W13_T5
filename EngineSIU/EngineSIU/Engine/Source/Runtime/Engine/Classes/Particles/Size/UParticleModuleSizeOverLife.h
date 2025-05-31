#pragma once
#include "ParticleModuleSizeBase.h"
class UParticleModuleSizeOverLife : public UParticleModuleSizeBase
{
    DECLARE_CLASS(UParticleModuleSizeOverLife, UParticleModuleSizeBase);
public:
    UParticleModuleSizeOverLife();

    virtual void DisplayProperty() override;

    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, EndSize)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, ChangeScale)
    
    virtual void SerializeAsset(FArchive& Ar) override
    {
        Super::Serialize(Ar);
        Ar << EndSize;
    }
};

