AnimFSM = {
    state = "Idle",
    blend = 0.4, -- 기본 블렌드 타임
    stateOrder = { "Idle", "Bray", "Walk", "Shake", "Preen", "Sit1", "Sleep"},
    
    -- 상태별 애니메이션 이름 매핑
    animMap = {
        Idle = "Contents/SkeletalPenguin/Armature|Armature|Idle",
        Bray = "Contents/SkeletalPenguin/Armature|Armature|Bray",
        Walk = "Contents/SkeletalPenguin/Armature|Armature|Walk",
        Shake = "Contents/SkeletalPenguin/Armature|Armature|Shake",
        Preen = "Contents/SkeletalPenguin/Armature|Armature|Preen",
        Sit1 = "Contents/SkeletalPenguin/Armature|Armature|Sit1",
        Sleep = "Contents/SkeletalPenguin/Armature|Armature|Sleep"
    },
    
    -- 상태별 블렌드 타임 매핑
    blendMap = {
        Idle = 0.4,
        Bray = 0.4,
        Walk = 0.2,  -- Walk는 더 빠른 전환
        Shake = 0.4,
        Preen = 0.4,
        Sit1 = 0.4,        
        Sleep = 0.6  -- Sleep은 더 부드러운 전환
    },

    -- Update 함수: 현재 상태에 맞는 애니메이션 반환
    Update = function(self, DeltaTime)
        --print("Current Animation: ", self.state)
        local animName = self.animMap[self.state] or self.animMap["Idle"]
        local blendTime = self.blendMap[self.state] or 0.4
        
        return { anim = animName, blend = blendTime }
    end
}

return AnimFSM
