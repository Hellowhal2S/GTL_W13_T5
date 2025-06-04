AnimFSM = {
    state = "Sleep",
    blend = 0.4, -- 기본 블렌드 타임
    --stateOrder = { "Idle", "SlowRun", "FastRun", "NarutoRun", "Runaway"},
    stateOrder = { "Idle", "Bray", "Walk", "Shake", "Preen", "Sit1", "Sleep"},
    -- 상태별 애니메이션 이름 매핑
    animMap = {
        --Idle = "Contents/Human/Idle",
        --SlowRun = "Contents/Human/SlowRun",
        --FastRun = "Contents/Human/FastRun",
        --NarutoRun = "Contents/Human/NarutoRun",
        --Runaway = "Contents/Xbot|Runaway.001"
        Idle = "Contents/SkeletalPenguin/Armature|Armature|Idle",
        Bray = "Contents/SkeletalPenguin/Armature|Armature|Bray",
        Walk = "Contents/SkeletalPenguin/Armature|Armature|Walk",
        Shake = "Contents/SkeletalPenguin/Armature|Armature|Shake",
        --Penguin swim = "Contents/SkeletalPenguin/Armature|Armature|Penguin swim",
        Preen = "Contents/SkeletalPenguin/Armature|Armature|Preen",
        Sit1 = "Contents/SkeletalPenguin/Armature|Armature|Sit1",
        Sleep = "Contents/SkeletalPenguin/Armature|Armature|Sleep"
    },    -- 상태별 블렌드 타임 매핑
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
        Walk = 0.4,
        Shake = 0.4,
        Preen = 0.4,
        Sit1 = 0.4,        
        Sleep = 0.4
    },

    -- 트리거 상태 관련 변수
    isTriggerOn = false,    -- OnOverlap: 무조건 Walk 상태로 변경
    OnOverlap = function(self, other)
        print("========================================================")
        print("========================================================")
        print("self : ", tostring(self))
        print("OnOverlap 호출 - 이전 상태: " .. self.state)
        self.state = "Walk"
        self.blend = self.blendMap[self.state] or 0.4
        self.isTriggerOn = true
        print("OnOverlap 완료 - 새로운 상태: " .. self.state)
    end,

    -- OnEndOverlap: 무조건 Idle 상태로 변경
    OnEndOverlap = function(self, other)
        print("========================================================")
        print("========================================================")
        print("self : ", tostring(self))
        print("OnEndOverlap 호출 - 이전 상태: " .. self.state)
        self.state = "Idle"
        self.blend = self.blendMap[self.state] or 0.4
        self.isTriggerOn = false
        print("OnEndOverlap 완료 - 새로운 상태: " .. self.state)
    end,
    
    Update = function(self, DeltaTime)
        print("self : ", tostring(self))
        --print("Update 함수 호출")
        local animName = self.animMap[self.state] or self.animMap["Idle"]
        local blendTime = self.blend or 0.4
        
        -- 디버깅을 위한 출력
        --print("Update - Current State: " .. self.state)
        --print("Update - Animation Name: " .. animName)
        --print("Update - Blend Time: " .. blendTime)
        
        return { anim = animName, blend = blendTime }
    end
}

return AnimFSM
