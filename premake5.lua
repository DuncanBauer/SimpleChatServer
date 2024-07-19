workspace "ChatServer"
	configurations
	{
		"Debug",
		"Release"
	}
	platforms
	{
		"Windows",
		"Unix"
	}

outputdir = "%{cfg.buildcfg}/%{cfg.system}"

-- Include dirs relative to root folder
IncludeDir = {}
IncludeDir["Core"]  = "%{os.getcwd()}/Core/src"
IncludeDir["ASIO"] = "%{os.getcwd()}/Core/Vendor/asio/include"
IncludeDir["spdlog"] = "%{os.getcwd()}/Core/Vendor/spdlog/include"

IncludeDir["ImGui"]  = "%{os.getcwd()}/Client/Vendor/imgui"
IncludeDir["ImGuiBackends"]  = "%{os.getcwd()}/Client/Vendor/imgui/backends"
IncludeDir["SDL"] = "%{os.getcwd()}/Client/Vendor/SDL/include"

LinkDir = {}
LinkDir["SDL"] = "%{os.getcwd()}/Client/Vendor/SDL/build/%{cfg.buildcfg}/SDL3"

include "Client"
include "Server"