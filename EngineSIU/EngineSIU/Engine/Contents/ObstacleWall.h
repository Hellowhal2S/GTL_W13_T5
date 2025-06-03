#pragma once
#include "ObstacleBase.h"

class AObstacleWall : public AObstacleBase
{
    DECLARE_CLASS(AObstacleWall, AObstacleBase);

public:
    AObstacleWall();
    virtual ~AObstacleWall() override = default ;


};

