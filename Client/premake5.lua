project "Client"
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	targetdir ("../bin/" .. outputdir)
	objdir ("../bin-int/" .. outputdir)

	files
	{
		"src/**.cpp",
		"src/**.h",
		
		-- Core
		"../Core/src/**.cpp",
		"../Core/src/**.h",

		-- Include required imgui classes for SDL3 and OpenGL
		"Vendor/imgui/*.cpp",
		"Vendor/imgui/*.h",
		"Vendor/imgui/backends/imgui_impl_sdl3.cpp",
		"Vendor/imgui/backends/imgui_impl_sdl3.h",
		"Vendor/imgui/backends/imgui_impl_opengl3.cpp",
		"Vendor/imgui/backends/imgui_impl_opengl3.h",
		"Vendor/imgui/backends/imgui_impl_opengl3_loader.h",
	}

	includedirs
	{
		"%{IncludeDir.Core}",
		"%{IncludeDir.ASIO}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.SDL}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuiBackends}"
	}

	links
	{
		"%{LinkDir.SDL}"
	}

	filter "system:windows"
		architecture "x86_64"
		defines 
		{
			"_WIN32_WINNT=0x0601"
		}
		links
		{
			"OpenGL32"
		}

	filter "system:linux"
		architecture "x86_64"
		links
		{
			"GL"
		}

	filter "configurations:Debug"
		defines 
		{
			"DEBUG"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"NDEBUG"
		}
		runtime "Release"
		optimize "on"