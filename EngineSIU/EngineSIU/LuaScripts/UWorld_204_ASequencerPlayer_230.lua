-- 공 굴리기 컨트롤러
-- 토크를 사용하여 물리적으로 자연스러운 굴리기 구현

-- 설정값
local rollTorque = 5.0        -- 굴리기 토크 강도
local jumpForce = 50.0         -- 점프 힘
local airControlForce = 0.0   -- 공중에서의 제어력

-- 바닥 감지 변수
local isGrounded = false       -- 바닥에 닿아있는지 여부
local groundCheckDelay = 0.1   -- 바닥 감지 딜레이 (초)
local timeSinceLastContact = 0.0

-- ForceMode 상수 정의
local FORCE_MODE = {
    FORCE = 0,           -- 연속적인 힘 (질량 고려)
    IMPULSE = 1,         -- 즉시 충격 (질량 고려)
    VELOCITY_CHANGE = 2, -- 속도 변화 (질량 무시)
    ACCELERATION = 3     -- 가속도 (질량 무시)
}

function BeginPlay()
    print("Ball Rolling Controller Started")
    isGrounded = false
end

function EndPlay()
    print("Ball Rolling Controller Ended")
end

function OnOverlap(OtherActor)
    -- 충돌 처리가 필요하면 여기에 구현
end

-- PhysX Contact 이벤트 처리
function OnContactBegin(OtherActor, ContactPoint)
    -- 바닥과의 접촉 시작
    isGrounded = true
    timeSinceLastContact = 0.0
end

function OnContactEnd(OtherActor, ContactPoint)
    -- 바닥과의 접촉 종료
    -- 즉시 false로 설정하지 않고 약간의 딜레이를 둠
end

function InitializeLua()
    -- 키 바인딩
    controller("W", OnPressW)    -- 앞으로 굴리기
    controller("S", OnPressS)    -- 뒤로 굴리기
    controller("A", OnPressA)    -- 왼쪽으로 굴리기
    controller("D", OnPressD)    -- 오른쪽으로 굴리기
    controller("SpaceBar", OnPressSpace) -- 점프
    
    -- PhysX Contact 이벤트 바인딩
    BindContactEvents()
end

-- 앞으로 굴리기 (X축 토크)
function OnPressW(dt)
    -- ms 보정
    local torque = FVector(-rollTorque * dt * 1000, 0, 0)
    ApplyTorque(torque, FORCE_MODE.FORCE)
end

-- 뒤로 굴리기 (-X축 토크)
function OnPressS(dt)
    local torque = FVector(rollTorque * dt * 1000, 0, 0)
    ApplyTorque(torque, FORCE_MODE.FORCE)
end

-- 왼쪽으로 굴리기 (Y축 토크)
function OnPressA(dt)
    local torque = FVector(0, rollTorque * dt * 1000, 0)
    ApplyTorque(torque, FORCE_MODE.FORCE)
end

-- 오른쪽으로 굴리기 (-Y축 토크)
function OnPressD(dt)
    local torque = FVector(0, -rollTorque * dt * 1000, 0)
    ApplyTorque(torque, FORCE_MODE.FORCE)
end

-- 점프 (바닥에 닿아있을 때만)
function OnPressSpace(dt)
    if isGrounded then
        ApplyJumpImpulse(jumpForce)
        -- 점프 후 즉시 grounded를 false로 설정하여 연속 점프 방지
        isGrounded = false
    end
end

function Tick(dt)
    -- 매 프레임마다 실행되는 로직
    -- 바닥 감지 딜레이 처리
    if not isGrounded then
        timeSinceLastContact = timeSinceLastContact + dt
        
        -- 일정 시간 후에도 접촉이 없으면 완전히 공중 상태로 간주
        if timeSinceLastContact > groundCheckDelay then
            isGrounded = false
        end
    end
end

function BeginOverlap()
end

function EndOverlap()
end
