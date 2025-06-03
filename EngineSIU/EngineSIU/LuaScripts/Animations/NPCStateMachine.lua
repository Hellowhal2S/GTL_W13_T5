AnimFSM = {
    state = "Idle",
    blend = 0.4, -- 기본 블렌드 타임
    --stateOrder = { "Idle", "SlowRun", "FastRun", "NarutoRun", "Runaway"},
    stateOrder = { "Idle", "Bray", "Walk"},
    -- 상태별 애니메이션 이름 매핑
    animMap = {
        --Idle = "Contents/Human/Idle",
        --SlowRun = "Contents/Human/SlowRun",
        --FastRun = "Contents/Human/FastRun",
        --NarutoRun = "Contents/Human/NarutoRun",
        --Runaway = "Contents/Xbot|Runaway.001"
        Idle = "Contents/SkeletalPenguin/Armature|Armature|Idle",
        Bray = "Contents/SkeletalPenguin/Armature|Armature|Bray",
        Walk = "Contents/SkeletalPenguin/Armature|Armature|Walk"
    },

    -- 상태별 블렌드 타임 매핑
    --blendMap = {
    --    Idle = 1.0,
    --    SlowRun = 1.0,
    --    FastRun = 1.0,
    --    NarutoRun = 1.0, -- NarutoRun에서 다른 상태로 갈 때 기본값
    --    Runaway = 1.0
    --},
    blendMap = {
        Idle = 0.4,
        Bray = 0.4,
        Walk = 0.4
    },

   -- 상태 전이 관련 변수
    isTriggerOn = false,
    stateTimer = 0,
    stateInterval = 2.0, -- 상태 전이 간격(초)
    targetStateIdx = 1,  -- 목표 상태 인덱스

    OnOverlap = function(self, other)
        print("OnOverlap 호출")
        if other:IsA("APlayer") then
            print("플레이어와 겹침")
            self.isTriggerOn = true
            -- Runaway 상태로 전환
            self.state = "Walk"
            self.blend = self.blendMap[self.state] or 0.4
            --self.targetStateIdx = #self.stateOrder -- NarutoRun까지 진행
            self.stateTimer = 0
        end
    end,

    OnEndOverlap = function(self, other)
        print("OnEndOverlap 호출")
        if other:IsA("APlayer") then
            self.isTriggerOn = false
            --self.targetStateIdx = 1 -- Idle까지 역순 진행
            self.state = "Bray"
            self.blend = self.blendMap[self.state] or 0.4
            self.stateTimer = 0
        end
    end,

    Update = function(self, DeltaTime)
        local animName = self.animMap[self.state] or "Idle"
        local blendTime = self.blend or 0.4
        return { anim = animName, blend = blendTime }
    end
}

return AnimFSM
