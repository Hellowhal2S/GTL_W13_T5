#include "SphereTriggerComponent.h"
#include "Classes/GameFramework/Actor.h"

USphereTriggerComponent::USphereTriggerComponent()
{
    bGenerateOverlapEvents = true;

    OnComponentBeginOverlap.AddLambda(
        [](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
        {
            UE_LOG(ELogLevel::Display, TEXT("Trigger BeginOverlap: %s"), *OtherActor->GetName());
        }
    );

    OnComponentEndOverlap.AddLambda(
        [](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex)
        {
            UE_LOG(ELogLevel::Display, TEXT("Trigger EndOverlap: %s"), *OtherActor->GetName());
        }
    );
}
    
