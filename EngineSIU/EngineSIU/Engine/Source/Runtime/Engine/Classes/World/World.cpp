void UWorld::Tick(float DeltaTime)
{
    // Release 빌드에서 World Tick 상태 확인
    #ifdef NDEBUG
    static bool bFirstWorldTick = true;
    if (bFirstWorldTick)
    {
        UE_LOG(ELogLevel::Display, TEXT("Release Build: World::Tick started for %s"), *GetName());
        bFirstWorldTick = false;
    }
    #endif
    
    WorldTime += DeltaTime;
    
    // ...existing code...
}