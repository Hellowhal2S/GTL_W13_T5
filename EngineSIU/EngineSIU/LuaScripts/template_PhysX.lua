-- 공 굴리기 컨트롤러
-- 각속도 직접 제어를 사용하여 질량에 무관한 일관된 굴리기 구현

setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.

-- 설정값
local rollAngularSpeed = 10.0        -- 굴리기 각속도 (rad/s)
local jumpVelocity = 10.0          -- 점프 속도 (직접 설정)
local airControlSpeed = 5.0        -- 공중에서의 제어 각속도

-- 바닥 감지 변수
local isGrounded = false       -- 바닥에 닿아있는지 여부
local wantInAir = false -- 공중에 있는지 여부
local groundCheckDelay = 0.1   -- 바닥 감지 딜레이 (초)
local timeSinceLastContact = 0.0

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

-- 앞으로 굴리기 (액터의 Forward 방향) - 각속도 직접 제어
function ReturnTable:OnPressW(dt)
    -- 액터의 Right 벡터를 회전축으로 사용 (Forward 방향으로 굴리기 위해)
    local rightVector = GetActorRightVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    currentSpeed = currentSpeed * dt
    local angularVelocity = FVector(
        rightVector.X * currentSpeed,
        rightVector.Y * currentSpeed,
        rightVector.Z * currentSpeed
    )
    -- 새로운 함수 사용 (구현 필요)
    AddAngularVelocityToSnowBall(angularVelocity)
end

-- 뒤로 굴리기 (액터의 Forward 반대 방향) - 각속도 직접 제어
function ReturnTable:OnPressS(dt)
    -- 액터의 Right 벡터 반대를 회전축으로 사용 (Backward 방향으로 굴리기 위해)
    local rightVector = GetActorRightVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    currentSpeed = currentSpeed * dt
    local angularVelocity = FVector(
        -rightVector.X * currentSpeed,
        -rightVector.Y * currentSpeed,
        -rightVector.Z * currentSpeed
    )
    AddAngularVelocityToSnowBall(angularVelocity)
end

-- 왼쪽으로 굴리기 (액터의 Right 반대 방향) - 각속도 직접 제어
function ReturnTable:OnPressA(dt)
    -- 액터의 Forward 벡터를 회전축으로 사용 (Left 방향으로 굴리기 위해)
    local forwardVector = GetActorForwardVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    currentSpeed = currentSpeed * dt
    local angularVelocity = FVector(
        forwardVector.X * currentSpeed,
        forwardVector.Y * currentSpeed,
        forwardVector.Z * currentSpeed
    )
    AddAngularVelocityToSnowBall(angularVelocity)
end

-- 오른쪽으로 굴리기 (액터의 Right 방향) - 각속도 직접 제어
function ReturnTable:OnPressD(dt)
    -- 액터의 Forward 벡터 반대를 회전축으로 사용 (Right 방향으로 굴리기 위해)
    local forwardVector = GetActorForwardVector(self.this)
    local currentSpeed = isGrounded and rollAngularSpeed or airControlSpeed
    currentSpeed = currentSpeed * dt
    local angularVelocity = FVector(
        -forwardVector.X * currentSpeed,
        -forwardVector.Y * currentSpeed,
        -forwardVector.Z * currentSpeed
    )
    AddAngularVelocityToSnowBall(angularVelocity)
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
    -- 필요하면 공중에서의 추가 제어나 댐핑 등을 구현
    if isGrounded then
        GrowSnowBall(dt) -- 바닥에 닿아있을 때 눈덩이 성장
    end
    if wantInAir then
        timeSinceLastContact = timeSinceLastContact + dt
        
        -- 일정 시간 후에도 접촉이 없으면 완전히 공중 상태로 간주
        if timeSinceLastContact > groundCheckDelay then
            isGrounded = false
        end
    end
end

function ReturnTable:BeginOverlap()
end

function ReturnTable:EndOverlap()
end

return ReturnTable
