-- 공 굴리기 컨트롤러
-- 각속도 직접 제어를 사용하여 질량에 무관한 일관된 굴리기 구현

setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.

-- 안전한 거듭제곱 함수 정의 (math.pow 대체)
local function safePow(base, exponent)
    -- Lua 내장 지수 연산자 사용 (가장 안전)
    return base ^ exponent
end

-- math 라이브러리 안전성 검사 및 대체 함수 정의
local mathPow = math.pow or safePow
local mathMin = math.min or function(a, b) return (a < b) and a or b end
local mathMax = math.max or function(a, b) return (a > b) and a or b end
local mathSqrt = math.sqrt or function(x) return x ^ 0.5 end

-- 설정값
local rollAngularSpeed = 50.0        -- 굴리기 각속도 (rad/s)
local jumpVelocity = 100.0          -- 기본 점프 속도
local airControlSpeed = 5.0         -- 공중에서의 제어 각속도
local jumpGravityMultiplier = 0.7   -- 점프 상승 시 중력 배율 (더 부드러운 상승)
local fallGravityMultiplier = 1.8   -- 점프 하강 시 중력 배율 (더 빠른 하강)
local coyoteTime = 0.15            -- 바닥에서 떨어진 후에도 점프 가능한 시간

-- 바닥 감지 변수
local isGrounded = false       -- 바닥에 닿아있는지 여부
local wantInAir = false -- 공중에 있는지 여부
local groundCheckDelay = 0.1   -- 바닥 감지 딜레이 (초)
local timeSinceLastContact = 0.0

-- 점프 제어 변수 (개선된 점프 시스템)
local jumpCooldown = 0.1           -- 점프 쿨다운 시간 (단축)
local timeSinceLastJump = 0.0      -- 마지막 점프 이후 경과 시간
local isJumpPressed = false        -- 점프 버튼이 눌려있는지 여부
local isJumping = false            -- 현재 점프 중인지 여부
local jumpPhase = "none"           -- 점프 단계: "none", "rising", "peak", "falling"
local timeSinceGrounded = 0.0      -- 바닥에서 떨어진 시간 (코요테 타임용)

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
    controller("SpaceBar", function(dt) self:OnPressSpace(dt) end) -- 점프 (누르기)
    
    -- 키 릴리즈 이벤트 바인딩 (점프 버튼을 뗐을 때)
    controllerRelease("SpaceBar", function() self:OnReleaseSpace() end)
    
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

-- 점프 - 단순한 고정 점프 시스템
function ReturnTable:OnPressSpace(dt)
    -- 코요테 타임 체크 (바닥에서 떨어진 직후에도 점프 가능)
    local canJump = isGrounded or (timeSinceGrounded <= coyoteTime)
    
    if canJump and timeSinceLastJump >= jumpCooldown then
        if not isJumpPressed then
            -- 점프 시작 - 고정된 점프력 적용
            isJumpPressed = true
            isJumping = true
            jumpPhase = "rising"
            
            -- 고정된 점프 속도 적용
            local jumpVector = FVector(0, 0, jumpVelocity)
            AddLinearVelocityToSnowBall(jumpVector)
            
            print("Jump started - Fixed height")
            
            -- 점프 상태 설정
            isGrounded = false
            wantInAir = true
            timeSinceLastJump = 0.0
            timeSinceGrounded = 0.0
        end
    end
end

-- 점프 버튼을 뗐을 때 처리하는 새로운 함수
function ReturnTable:OnReleaseSpace()
    if isJumpPressed then
        isJumpPressed = false
        jumpPhase = "falling"
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
        local moveDistance = mathSqrt(deltaX * deltaX + deltaY * deltaY)
        
        -- 누적 굴림 거리에 추가
        accumulatedRollDistance = accumulatedRollDistance + moveDistance
        
        -- 최소 굴림 거리에 도달했을 때 눈덩이 성장
        if accumulatedRollDistance >= minRollDistanceForGrow then
            -- 굴린 거리에 비례하여 성장 (더 많이 굴수록 더 많이 성장)
            local growthAmount = accumulatedRollDistance * dt * 0.5
            GrowSnowBall(growthAmount)
            PlayJumpSound();
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
    
    -- 코요테 타임 업데이트 (바닥에서 떨어진 시간 추적)
    if not isGrounded then
        timeSinceGrounded = timeSinceGrounded + dt
    else
        timeSinceGrounded = 0.0 -- 바닥에 닿으면 리셋
    end
    
    -- 점프 쿨다운 타이머 갱신
    if timeSinceLastJump < jumpCooldown then
        timeSinceLastJump = timeSinceLastJump + dt
    end
    
    -- 점프 상태별 중력 및 물리 처리
    if jumpPhase == "rising" and isJumpPressed then
        -- 상승 중: 약한 중력으로 더 부드러운 상승
        local reducedGravity = -98 * jumpGravityMultiplier * dt
        AddLinearVelocityToSnowBall(FVector(0, 0, reducedGravity))
    elseif jumpPhase == "falling" or (not isGrounded and not isJumping) then
        -- 하강 중이거나 공중에 있을 때: 강한 중력으로 빠른 하강
        local enhancedGravity = -98 * fallGravityMultiplier * dt
        AddLinearVelocityToSnowBall(FVector(0, 0, enhancedGravity))
    elseif not isGrounded and jumpPhase == "none" then
        -- 일반적인 공중 상태: 기본 중력
        local normalGravity = -98 * dt
        AddLinearVelocityToSnowBall(FVector(0, 0, normalGravity))
    end
    
    -- 점프 버튼이 떼어졌을 때 즉시 하강 단계로 전환
    if not isJumpPressed and jumpPhase == "rising" then
        jumpPhase = "falling"
    end
    
    -- 바닥에 착지했을 때 점프 상태 리셋
    if isGrounded and (jumpPhase == "falling" or jumpPhase == "peak") then
        jumpPhase = "none"
        isJumping = false
    end
end

function ReturnTable:BeginOverlap()
end

function ReturnTable:EndOverlap()
end

return ReturnTable
