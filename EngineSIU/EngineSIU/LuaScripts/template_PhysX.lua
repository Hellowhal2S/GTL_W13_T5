-- 공 굴리기 컨트롤러
-- 토크를 사용하여 물리적으로 자연스러운 굴리기 구현

-- 설정값
local rollTorque = 10.0        -- 굴리기 토크 강도
local jumpForce = 10.0         -- 점프 힘
local airControlForce = 0.0   -- 공중에서의 제어력

-- ForceMode 상수 정의
local FORCE_MODE = {
    FORCE = 0,           -- 연속적인 힘 (질량 고려)
    IMPULSE = 1,         -- 즉시 충격 (질량 고려)
    VELOCITY_CHANGE = 2, -- 속도 변화 (질량 무시)
    ACCELERATION = 3     -- 가속도 (질량 무시)
}

function BeginPlay()
    print("Ball Rolling Controller Started")
end

function EndPlay()
    print("Ball Rolling Controller Ended")
end

function OnOverlap(OtherActor)
    -- 충돌 처리가 필요하면 여기에 구현
end

function InitializeLua()
    -- 키 바인딩
    controller("W", OnPressW)    -- 앞으로 굴리기
    controller("S", OnPressS)    -- 뒤로 굴리기
    controller("A", OnPressA)    -- 왼쪽으로 굴리기
    controller("D", OnPressD)    -- 오른쪽으로 굴리기
    controller("SpaceBar", OnPressSpace) -- 점프
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

-- 점프
function OnPressSpace(dt)
    print("Jumping")
    ApplyJumpImpulse(jumpForce)
end

function Tick(dt)
    -- 매 프레임마다 실행되는 로직
    -- 필요하면 공중에서의 추가 제어나 댐핑 등을 구현
end

function BeginOverlap()
end

function EndOverlap()
end
