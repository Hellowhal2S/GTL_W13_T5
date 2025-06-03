#include "StartUI.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"

StartUI::StartUI()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PIE);
}

void StartUI::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    int32 tmp = Engine->CurSceneName.Find("Start");
    if ( tmp == -1 )
        return; 
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = 200;
    float PanelHeight = 100;

    float PanelPosX = (Width) * 0.4f;
    float PanelPosY = (Height) * 0.5f;

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
    if (!ImGui::Begin("Start UI", nullptr, PanelFlags))
    {
        ImGui::End();
        return;
    }
    ImVec4 defaultColor     = ImGui::GetStyleColorVec4(ImGuiCol_Button);
    ImVec4 hoveredColor     = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);  // 원하는 회색
    ImVec4 activeColor      = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);  // 클릭된 상태 색상

    ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
    if (ImGui::Button("Start", ImVec2(100, 50)))
    {
        Engine->EndPIE();
        Engine->NewLevel();
        Engine->LoadLevel("Saved/MainScene.Scene");
        Engine->StartPIE();
    }
    ImGui::PopStyleColor(3);  // 3개 푸시했으므로 3개 팝
    ImGui::End();
}

void StartUI::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
