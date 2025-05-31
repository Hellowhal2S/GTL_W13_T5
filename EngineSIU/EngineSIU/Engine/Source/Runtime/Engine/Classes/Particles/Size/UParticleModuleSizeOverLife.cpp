#include "UParticleModuleSizeOverLife.h"
#include "Engine/ParticleHelper.h"
#include "Engine/ParticleEmitterInstance.h"
#include "Particles/ParticleSystemComponent.h"

UParticleModuleSizeOverLife::UParticleModuleSizeOverLife()
{
    bUpdateModule = true;

    EndSize = FVector::One() * 0.1f;
    ChangeScale = 1.0f;

    ModuleName = "SizeOverLife";
}

void UParticleModuleSizeOverLife::DisplayProperty()
{
    Super::DisplayProperty();

    for (const auto& Property : StaticClass()->GetProperties())
    {
        ImGui::PushID(Property);
        Property->DisplayInImGui(this);
        ImGui::PopID();
    }
}

void UParticleModuleSizeOverLife::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    BEGIN_UPDATE_LOOP

    float t = FMath::Clamp(Particle.RelativeTime, 0.0f, 1.0f);
    float curveAlpha = 1.0f - FMath::Pow(1.0f - t, ChangeScale);
    FVector NewSize = FMath::Lerp(Particle.BaseSize, EndSize, curveAlpha);

    Particle.BaseSize = NewSize;

    END_UPDATE_LOOP
}
