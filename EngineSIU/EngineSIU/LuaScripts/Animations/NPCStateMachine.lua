return {
    state = "Idle",
    blend = 0.4, -- 기본 블렌드 타임
    stateOrder = { "Idle", "SlowRun", "FastRun", "NarutoRun" },

    -- 상태별 애니메이션 이름 매핑
    animMap = {
        Idle = "Contents/Human/Idle",
        SlowRun = "Contents/Human/SlowRun",
        FastRun = "Contents/Human/FastRun",
        NarutoRun = "Contents/Human/NarutoRun"
    },

    -- 상태별 블렌드 타임 매핑
    blendMap = {
        Idle = 0.4,
        SlowRun = 0.4,
        FastRun = 2.0,
        NarutoRun = 0.4 -- NarutoRun에서 다른 상태로 갈 때 기본값
    },

    -- Player가 Trigger에 들어오면 상태를 한 단계 올림
    OnOverlap = function(self, other)
        if other:IsA("APlayer") then
            local idx = 1
            for i, v in ipairs(self.stateOrder) do
                if v == self.state then
                    idx = i
                    break
                end
            end
            -- 다음 상태로 전환 (최대 NarutoRun)
            if idx < #self.stateOrder then
                self.state = self.stateOrder[idx + 1]
            end
            -- 상태 변경 시 블렌드 타임 갱신
            self.blend = self.blendMap[self.state] or 0.4
            print("NPC 상태 전환: " .. self.state .. ", 블렌드: " .. tostring(self.blend))
        end
    end,

      -- Player가 Trigger에서 나가면 상태를 한 단계 내림 (역순)
    OnEndOverlap = function(self, other)
        if other:IsA("APlayer") then
            local idx = #self.stateOrder
            for i, v in ipairs(self.stateOrder) do
                if v == self.state then
                    idx = i
                    break
                end
            end
            -- 이전 상태로 전환 (최소 Idle)
            if idx > 1 then
                self.state = self.stateOrder[idx - 1]
            end
            self.blend = self.blendMap[self.state] or 0.4
            print("NPC 상태 복귀: " .. self.state .. ", 블렌드: " .. tostring(self.blend))
        end
    end,

    -- 상태별 애니메이션 반환
    Update = function(self, DeltaTime)
        local animName = self.animMap[self.state] or "Idle"
        local blendTime = self.blend or 0.4
        return { anim = animName, blend = blendTime }
    end
}
