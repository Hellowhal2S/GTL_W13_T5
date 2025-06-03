#include "EndUI.h"

#include "Engine/EditorEngine.h"
#include "GameFramework/GameMode.h"

EndUI::EndUI()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PIE);
}

void EndUI::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    
    if (!Engine->PIEWorld->GetGameMode()->bGameOver)
        return;
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = 300;
    float PanelHeight = 300;

    float PanelPosX = (Width) * 0.4f;
    float PanelPosY = (Height) * 0.5f;

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
    if (!ImGui::Begin("End UI", nullptr, PanelFlags))
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
    if (GEngine->ActiveWorld->GetGameMode())
    {
        ImGui::Text("Score %d", GEngine->ActiveWorld->GetGameMode()->Score);
    }
    if (ImGui::Button("ReStart", ImVec2(100, 50)))
    {
        Engine->EndPIE();
        Engine->NewLevel();
        Engine->LoadLevel("Saved/MainScene.Scene");
        Engine->StartPIE();
    }
    if (ImGui::Button("Exit", ImVec2(100, 50)))
    {
        Engine->EndPIE();
        Engine->NewLevel();
        Engine->LoadLevel("Saved/Start.Scene");
        Engine->StartPIE();
    }
    ImGui::PopStyleColor(3);  // 3개 푸시했으므로 3개 팝
    ImGui::End();
}

void EndUI::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
