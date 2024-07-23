#include <string>

#include <imgui.h>

#include "Net/TCPClient.h"
#include "Util.h"

void showLoginWindow(net::TCPClient& client)
{
    ImGui::Begin("Discord Clone");

    static char username[64] = "duncan";
    static char password[64] = "password";

    ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
    ImGui::InputText("password", password, IM_ARRAYSIZE(password));

    if (ImGui::Button("Login"))
    {
        std::string susername = stripWhitespace(username, 64);
        std::string spassword = stripWhitespace(password, 64);
        client.tryLogin(susername, spassword);
    }

    if (ImGui::Button("Register"))
    {
        std::string susername = stripWhitespace(username, 64);
        std::string spassword = stripWhitespace(password, 64);
        client.tryRegister(susername, spassword);
    }

    ImGui::End();
};
