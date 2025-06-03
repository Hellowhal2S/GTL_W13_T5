#pragma once
#include "Engine/Contents/Actor/ATarget.h"
#include "UnrealEd/EditorPanel.h"

class StartUI : public UEditorPanel
{
public:
    StartUI();
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    float Width = 300, Height = 100;
};
