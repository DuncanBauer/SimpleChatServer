project "Server"
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
		"../Core/src/**.h"
	}

	includedirs
	{
		"src",
		"%{IncludeDir.Core}",
		"%{IncludeDir.ASIO}",
		"%{IncludeDir.Cryptopp}",
		"%{IncludeDir.MongoC}",
		"%{IncludeDir.MongoCXX}",
		"%{IncludeDir.Bson}",
		"%{IncludeDir.BsonCXX}",
		"%{IncludeDir.spdlog}"
	}

	links
	{
	}

	filter "system:windows"
		architecture "x86_64"
		defines 
		{
			"_WIN32_WINNT=0x0601"
		}

	filter "system:linux"
		architecture "x86_64"

	filter "configurations:Debug"
		defines 
		{
			"DEBUG",
			"STATIC_CONCPP"
		}
		links
		{
			"%{LinkDir.Cryptoppd}",
			
			"%{LinkDir.MongoCd}",
			"%{LinkDir.MongoCXXd}",
			"%{LinkDir.Bsond}",
			"%{LinkDir.BsonCXXd}"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"NDEBUG",
			"STATIC_CONCPP"
		}
		links
		{
			"%{LinkDir.Cryptopp}",
			
			"%{LinkDir.MongoC}",
			"%{LinkDir.MongoCXX}",
			"%{LinkDir.Bson}",
			"%{LinkDir.BsonCXX}"
		}
		runtime "Release"
		optimize "on"