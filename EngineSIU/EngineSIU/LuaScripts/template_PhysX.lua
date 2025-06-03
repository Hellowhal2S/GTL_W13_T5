-- 공 굴리기 컨트롤러
-- 각속도 직접 제어를 사용하여 질량에 무관한 일관된 굴리기 구현

setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.

-- 설정값
local rollAngularSpeed = 20.0        -- 굴리기 각속도 (rad/s)
local jumpVelocity = 10.0          -- 점프 속도 (직접 설정)
local airControlSpeed = 5.0        -- 공중에서의 제어 각속도

-- 바닥 감지 변수
local isGrounded = false       -- 바닥에 닿아있는지 여부
local wantInAir = false -- 공중에 있는지 여부
local groundCheckDelay = 0.1   -- 바닥 감지 딜레이 (초)
local timeSinceLastContact = 0.0

-- GrowSnowBall 호출 제어 변수 (이동 기반으로 변경)
local minRollDistanceForGrow = 1.0  -- 성장을 위한 최소 굴림 거리
local accumulatedRollDistance = 0.0 -- 누적 굴림 거리
local lastPosition = FVector(0, 0, 0) -- 이전 프레임의 위치
local hasInitialPosition = false    -- 초기 위치 설정 여부

-- ForceMode 상수 정의 (각속도 설정에는 사용하지 않지만 호환성을 위해 유지)
local FORCE_MODE = {
    FORCE = 0,           -- 연속적인 힘 (질량 고려)
    IMPULSE = 1,         -- 즉시 충격 (질량 고려)
    VELOCITY_CHANGE = 2, -- 속도 변화 (질량 무시)
    ACCELERATION = 3     -- 가속도 (질량 무시)
}

function ReturnTable:BeginPlay()
    print("Ball Rolling Controller Started - Direct Velocity Control")
    self:InitializeLua()
end

function ReturnTable:EndPlay()
    print("Ball Rolling Controller Ended")
end

function ReturnTable:OnOverlap(OtherActor)
    -- 충돌 처리가 필요하면 여기에 구현
end

-- PhysX Contact 이벤트 처리
function ReturnTable:OnContactBegin(OtherActor, ContactPoint)
    -- 바닥과의 접촉 시작
    isGrounded = true
    wantInAir = false
    timeSinceLastContact = 0.0
end

function ReturnTable:OnContactEnd(OtherActor, ContactPoint)
    -- 바닥과의 접촉 종료
    -- 즉시 false로 설정하지 않고 약간의 딜레이를 둠
    wantInAir = true
end

-- 키 바인딩
function ReturnTable:InitializeLua()
    controller("W", function(dt) self:OnPressW(dt) end)    -- 앞으로 굴리기
    controller("S", function(dt) self:OnPressS(dt) end)    -- 뒤로 굴리기
    controller("A", function(dt) self:OnPressA(dt) end)    -- 왼쪽으로 굴리기
    controller("D", function(dt) self:OnPressD(dt) end)    -- 오른쪽으로 굴리기
    controller("SpaceBar", function(dt) self:OnPressSpace(dt) end) -- 점프
    
    -- PhysX Contact 이벤트 바인딩
    self:BindContactEvents()
end

-- Contact 이벤트 바인딩 함수
function ReturnTable:BindContactEvents()    
    -- Contact 콜백 등록
    RegisterSnowBallContactCallback(
        function(otherActor, contactPoint) 
            self:OnContactBegin(otherActor, contactPoint) 
        end,
        function(otherActor, contactPoint) 
            self:OnContactEnd(otherActor, contactPoint) 
        end
    )
    
    print("SnowBall contact events bound successfully")
end

-- 앞으로 굴리기 (액터의 Forward 방향) - 선형속도만 제어
function ReturnTable:OnPressW(dt)
    -- 액터의 Forward 벡터로 선형 이동 (각속도 제거)
    local forwardVector = GetActorForwardVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    
    -- 선형속도만 적용
    local linearVelocity = FVector(
        forwardVector.X * currentSpeed * dt,
        forwardVector.Y * currentSpeed * dt,
        0  -- Z축 속도는 0으로 유지하여 수평 이동만
    )
    AddLinearVelocityToSnowBall(linearVelocity)
end

-- 뒤로 굴리기 (액터의 Forward 반대 방향) - 선형속도만 제어
function ReturnTable:OnPressS(dt)
    -- 액터의 Forward 벡터 반대로 선형 이동 (각속도 제거)
    local forwardVector = GetActorForwardVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    
    -- 선형속도만 적용
    local linearVelocity = FVector(
        -forwardVector.X * currentSpeed * dt,
        -forwardVector.Y * currentSpeed * dt,
        0  -- Z축 속도는 0으로 유지하여 수평 이동만
    )
    AddLinearVelocityToSnowBall(linearVelocity)
end

-- 왼쪽으로 굴리기 (액터의 Right 반대 방향) - 선형속도만 제어
function ReturnTable:OnPressA(dt)
    -- 액터의 Right 벡터 반대로 선형 이동 (각속도 제거)
    local rightVector = GetActorRightVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    
    -- 선형속도만 적용
    local linearVelocity = FVector(
        -rightVector.X * currentSpeed * dt,
        -rightVector.Y * currentSpeed * dt,
        0  -- Z축 속도는 0으로 유지하여 수평 이동만
    )
    AddLinearVelocityToSnowBall(linearVelocity)
end

-- 오른쪽으로 굴리기 (액터의 Right 방향) - 선형속도만 제어
function ReturnTable:OnPressD(dt)
    -- 액터의 Right 벡터로 선형 이동 (각속도 제거)
    local rightVector = GetActorRightVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    
    -- 선형속도만 적용
    local linearVelocity = FVector(
        rightVector.X * currentSpeed * dt,
        rightVector.Y * currentSpeed * dt,
        0  -- Z축 속도는 0으로 유지하여 수평 이동만
    )
    AddLinearVelocityToSnowBall(linearVelocity)
end

-- 점프 - 선형속도 직접 설정
function ReturnTable:OnPressSpace(dt)
    if isGrounded then
        -- Z축(위쪽) 방향으로 직접 속도 설정
        local jumpVector = FVector(0, 0, jumpVelocity)
        AddLinearVelocityToSnowBall(jumpVector)
        -- 점프 후 즉시 grounded를 false로 설정하여 연속 점프 방지
        isGrounded = false
        wantInAir = true
    end
end

function ReturnTable:Tick(dt)
    -- 매 프레임마다 실행되는 로직
    
    -- 현재 위치 가져오기
    local currentPosition = GetActorLocation(self.this)
    
    -- 초기 위치 설정
    if not hasInitialPosition then
        lastPosition = currentPosition
        hasInitialPosition = true
        return
    end
    
    -- 바닥에 닿아있을 때만 이동 거리 계산
    if isGrounded then
        -- 이전 위치와 현재 위치 간의 거리 계산
        local deltaX = currentPosition.X - lastPosition.X
        local deltaY = currentPosition.Y - lastPosition.Y
        -- Z축 변화는 제외 (점프나 높이 변화는 굴림에 포함하지 않음)
        local moveDistance = math.sqrt(deltaX * deltaX + deltaY * deltaY)
        
        -- 누적 굴림 거리에 추가
        accumulatedRollDistance = accumulatedRollDistance + moveDistance
        
        -- 최소 굴림 거리에 도달했을 때 눈덩이 성장
        if accumulatedRollDistance >= minRollDistanceForGrow then
            -- 굴린 거리에 비례하여 성장 (더 많이 굴수록 더 많이 성장)
            local growthAmount = accumulatedRollDistance * dt * 0.5
            GrowSnowBall(growthAmount)
            accumulatedRollDistance = 0.0 -- 누적 거리 리셋
        end
    end
    
    -- 현재 위치를 다음 프레임을 위해 저장
    lastPosition = currentPosition
    
    if wantInAir then
        timeSinceLastContact = timeSinceLastContact + dt
        
        -- 일정 시간 후에도 접촉이 없으면 완전히 공중 상태로 간주
        if timeSinceLastContact > groundCheckDelay then
            isGrounded = false
            wantInAir = false  -- 공중 상태 확정 후 플래그 리셋
        end
    end
end

function ReturnTable:BeginOverlap()
end

function ReturnTable:EndOverlap()
end

return ReturnTable
