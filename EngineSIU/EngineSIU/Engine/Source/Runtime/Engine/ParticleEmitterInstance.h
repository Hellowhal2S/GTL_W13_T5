#pragma once
#include "HAL/PlatformType.h"
#include "ParticleHelper.h"
#include "Particles/ParticleModule.h"

class UParticleLODLevel;
class UParticleSystemComponent;
class UParticleEmitter;
struct DistributionFloat;

struct FParticleEmitterInstance
{
public:
    UParticleEmitter* SpriteTemplate;

    UParticleSystemComponent* Component;

    int32 CurrentLODLevelIndex;
    UParticleLODLevel* CurrentLODLevel;

    /** Pointer to the particle data array.                             */
    uint8* ParticleData;
    /** Pointer to the particle index array.                            */
    uint16* ParticleIndices;
    /** Pointer to the instance data array.                             */
    uint8* InstanceData;
    /** The size of the Instance data array.                            */
    int32 InstancePayloadSize;
    /** The offset to the particle data.                                */
    int32 PayloadOffset;
    /** The total size of a particle (in bytes).                        */
    int32 ParticleSize;
    /** The stride between particles in the ParticleData array.         */
    int32 ParticleStride;
    /** The number of particles currently active in the emitter.        */
    int32 ActiveParticles;
    /** Monotonically increasing counter. */
    uint32 ParticleCounter;
    /** The maximum number of active particles that can be held in the particle data array.*/
    int32 MaxActiveParticles;
    /** The fraction of time left over from spawning.                   */
    /** The offset to the SubUV payload in the particle data.			*/
    int32 SubUVDataOffset;

    /** Component can disable Tick and Rendering of this emitter. */
    uint8 bEnabled : 1;

    float AccumulatedTime = 0;
    float CurrentTimeForBurst;
    DistributionFloat* SpawnRateDistribution;
    float SpawnFraction = 0;

    bool bOnceBurstOnBegin = false;

public:
    virtual ~FParticleEmitterInstance();

    void Initialize();
    void Release();

    void Tick(float DeltaTime);
    void SpawnParticles(int32 Count, float StartTime, float Increment, 
                        const FVector& InitialLocation, const FVector& InitialVelocity);
    void KillParticle(int32 Index);

    int32 CalculateSpawnCount(float DeltaTime);

    void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);
    void PostSpawn(FBaseParticle* Particle, float Interp, float SpawnTime);
    void UpdateParticles(float DeltaTime);
    void UpdateModules(float DeltaTime);

    void AllKillParticles();
    void BuildMemoryLayout();

    virtual bool IsDynamicDataRequired();
    virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) { return nullptr; }

    virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData );
    uint32 GetModuleDataOffset(UParticleModule* Module); // TODO: 구현하기
};

struct FParticleSpriteEmitterInstance : public FParticleEmitterInstance
{
    virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;
    virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData ) override;
};

struct FParticleMeshEmitterInstance : public FParticleEmitterInstance
{
    virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;
    virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData) override;
};
