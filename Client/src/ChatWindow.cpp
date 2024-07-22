#include <iostream>

#include "ChatWindow.h"
#include "Global.h"

void showChatWindow(ImGuiIO& io, ImVec4 clear_color)
{
    ImGui::Begin("Discord Clone");

    static char username[64] = "username123";
    static char password[64] = "password123";
    
    ImGui::InputText("Username", username, IM_ARRAYSIZE(password));
    ImGui::InputText("password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

    
    if (ImGui::Button("Login"))
    {
        std::string susername;
        std::string spassword;

        susername = stripWhitespace(username, 64);
        spassword = stripWhitespace(password, 64);

        client.tryLogin(susername, spassword);
    }

    if (ImGui::Button("Register"))
    {
        std::string susername;
        std::string spassword;

        susername = stripWhitespace(username, 64);
        spassword = stripWhitespace(password, 64);

        client.tryRegister(susername, spassword);
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
};

std::string stripWhitespace(char* data, int size)
{
    std::string ret;
    for (int i = 0; i < size; i++)
        if (data[i] != ' ' && data[i] != '\0' && data[i] != '\n' && data[i] != '\t')
            ret += data[i];

    return ret;
}