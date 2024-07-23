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
        for (int i = 0; i < 0; i++)
        {
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right

    static char createServer_serverName[64] = "";
    static char createServer_ownerId[64] = "";

    static char deleteServer_serverId[64] = "";

    static char joinServer_userId[64] = "";
    static char joinServer_serverId[64] = "";

    static char leaveServer_userId[64] = "";
    static char leaveServer_serverId[64] = "";

    static char createChannel_serverId[64] = "";
    static char createChannel_channelName[64] = "";

    static char deleteChannel_channelId[64] = "";

    static char sendMessage_authorId[64] = "";
    static char sendMessage_channelId[64] = "";
    static char sendMessage_content[64] = "";

    static char deleteMessage_channelId[64] = "";
    static char deleteMessage_messageId[64] = "";

    static char editMessage_messageId[64] = "";
    static char editMessage_content[64] = "";


    {
        ImGui::BeginGroup();
        ImGui::Text("Server Details");

        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text("MyObject: %d", selected);
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Logout"))
            {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Create Server"))
            {
                ImGui::InputText("User id", createServer_ownerId, IM_ARRAYSIZE(createServer_ownerId));
                ImGui::InputText("Server name", createServer_serverName, IM_ARRAYSIZE(createServer_serverName));

                if (ImGui::Button("Execute"))
                {
                    std::string ownerId = stripWhitespace(createServer_ownerId, 64);
                    std::string serverName = stripWhitespace(createServer_serverName, 64);
                    client.tryCreateServer(ownerId, serverName);
                }
                
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Delete Server"))
            {
                ImGui::InputText("Server id", deleteServer_serverId, IM_ARRAYSIZE(deleteServer_serverId));

                if (ImGui::Button("Execute"))
                {
                    std::string serverId = stripWhitespace(deleteServer_serverId, 64);
                    client.tryDeleteServer(serverId);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Create Channel"))
            {
                ImGui::InputText("Server id", createChannel_serverId, IM_ARRAYSIZE(createChannel_serverId));
                ImGui::InputText("Channel name", createChannel_channelName, IM_ARRAYSIZE(createChannel_channelName));

                if (ImGui::Button("Execute"))
                {
                    std::string serverId = stripWhitespace(createChannel_serverId, 64);
                    std::string channelName = stripWhitespace(createChannel_channelName, 64);
                    client.tryCreateChannel(serverId, channelName);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Delete Channel"))
            {
                ImGui::InputText("Channel id", deleteChannel_channelId, IM_ARRAYSIZE(deleteChannel_channelId));

                if (ImGui::Button("Execute"))
                {
                    std::string channelId = stripWhitespace(deleteChannel_channelId, 64);
                    client.tryDeleteChannel(channelId);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Join Server"))
            {
                ImGui::InputText("User id", joinServer_userId, IM_ARRAYSIZE(joinServer_userId));
                ImGui::InputText("Server id", joinServer_serverId, IM_ARRAYSIZE(joinServer_serverId));

                if (ImGui::Button("Execute"))
                {
                    std::string userId = stripWhitespace(joinServer_userId, 64);
                    std::string serverId = stripWhitespace(joinServer_serverId, 64);
                    client.tryJoinServer(userId, serverId);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Leave Server"))
            {
                ImGui::InputText("User id", leaveServer_userId, IM_ARRAYSIZE(leaveServer_userId));
                ImGui::InputText("Server id", leaveServer_serverId, IM_ARRAYSIZE(leaveServer_serverId));

                if (ImGui::Button("Execute"))
                {
                    std::string userId = stripWhitespace(leaveServer_userId, 64);
                    std::string serverId = stripWhitespace(leaveServer_serverId, 64);
                    client.tryLeaveServer(userId, serverId);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Send Message"))
            {
                ImGui::InputText("Author id", sendMessage_authorId, IM_ARRAYSIZE(sendMessage_authorId));
                ImGui::InputText("Channel id", sendMessage_channelId, IM_ARRAYSIZE(sendMessage_channelId));
                ImGui::InputText("Message content", sendMessage_content, IM_ARRAYSIZE(sendMessage_content));

                if (ImGui::Button("Execute"))
                {
                    std::string authorId = stripWhitespace(sendMessage_authorId, 64);
                    std::string channelId = stripWhitespace(sendMessage_channelId, 64);
                    std::string content = stripWhitespace(sendMessage_content, 64);
                    client.trySendMessage(authorId, channelId, content);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Delete Message"))
            {
                ImGui::InputText("Channel id", deleteMessage_channelId, IM_ARRAYSIZE(deleteMessage_channelId));
                ImGui::InputText("Message id", deleteMessage_messageId, IM_ARRAYSIZE(deleteMessage_messageId));

                if (ImGui::Button("Execute"))
                {
                    std::string channelId = stripWhitespace(deleteMessage_channelId, 64);
                    std::string messageId = stripWhitespace(deleteMessage_messageId, 64);
                    client.tryDeleteMessage(channelId, messageId);
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Edit Message"))
            {
                ImGui::InputText("Message id", editMessage_messageId, IM_ARRAYSIZE(editMessage_messageId));
                ImGui::InputText("Message content", editMessage_content, IM_ARRAYSIZE(editMessage_content));

                if (ImGui::Button("Execute"))
                {
                    std::string messageId = stripWhitespace(editMessage_messageId, 64);
                    std::string content = stripWhitespace(editMessage_content, 64);
                    client.tryEditMessage(messageId, content);
                }

                ImGui::EndTabItem();
            }
            //if (ImGui::BeginTabItem("Get Server Channels"))
            //{
            //    ImGui::EndTabItem();
            //}
            //if (ImGui::BeginTabItem("Get Server Members"))
            //{
            //    ImGui::EndTabItem();
            //}
            //if (ImGui::BeginTabItem("Get Channel Messages"))
            //{
            //    ImGui::EndTabItem();
            //}
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::EndGroup();
    }

    ImGui::End();
}
