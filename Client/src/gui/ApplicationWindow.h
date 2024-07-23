#pragma once

#include <imgui.h>

#include "Net/TCPClient.h"

void showApplicationWindow(net::TCPClient& client)
{
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
                                  | ImGuiWindowFlags_NoResize;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    ImGui::Begin("Discord Clone", NULL, flags);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", "Ctrl+W")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Left
    static int selected = 0;
    {
        ImGui::Text("Server List");

        ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        for (int i = 0; i < 100; i++)
        {
            // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
            char label[128];
            sprintf(label, "MyObject %d", i);
            if (ImGui::Selectable(label, selected == i))
                selected = i;
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::Text("Server Details");

        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text("MyObject: %d", selected);
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Description"))
            {
                ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Details"))
            {
                ImGui::Text("ID: 0123456789");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        
        if (ImGui::Button("Revert")) {}
        ImGui::SameLine();
        if (ImGui::Button("Save")) {}

        ImGui::EndGroup();
    }

    ImGui::End();
}
