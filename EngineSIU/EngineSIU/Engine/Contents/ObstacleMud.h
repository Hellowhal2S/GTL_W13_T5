#pragma once
#include "ObstacleBase.h"

class AObstacleMud : public AObstacleBase
{
    DECLARE_CLASS(AObstacleMud, AObstacleBase);

public:
    AObstacleMud();
    virtual ~AObstacleMud() override = default;
};

