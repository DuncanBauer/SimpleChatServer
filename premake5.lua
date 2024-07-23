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

vcpkgdir = "C:/repos/vcpkg/packages"

-- Include dirs relative to root folder
IncludeDir = {}
IncludeDir["Core"]				= "%{os.getcwd()}/Core/src"
IncludeDir["ASIO"]				= "%{os.getcwd()}/Core/Vendor/asio/include"

IncludeDir["Cryptopp"]			= "%{vcpkgdir}/cryptopp_x64-windows/include"

IncludeDir["ImGui"]				= "%{os.getcwd()}/Client/Vendor/imgui"
IncludeDir["ImGuiBackends"]		= "%{os.getcwd()}/Client/Vendor/imgui/backends"

IncludeDir["MongoC"]			= "%{vcpkgdir}/mongo-c-driver_x64-windows/include"
IncludeDir["MongoCXX"]			= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/include/mongocxx/v_noabi"
IncludeDir["Bson"]				= "%{vcpkgdir}/libbson_x64-windows/include"
IncludeDir["BsonCXX"]			= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/include/bsoncxx/v_noabi/"

IncludeDir["SDL"]				= "%{os.getcwd()}/Client/Vendor/SDL/include"
IncludeDir["spdlog"]			= "%{os.getcwd()}/Core/Vendor/spdlog/include"

LinkDir = {}
LinkDir["Cryptopp"]				= "%{vcpkgdir}/cryptopp_x64-windows/lib/cryptopp"
LinkDir["Cryptoppd"]			= "%{vcpkgdir}/cryptopp_x64-windows/debug/lib/cryptopp"

LinkDir["MongoC"]				= "%{vcpkgdir}/mongo-c-driver_x64-windows/lib/mongoc-1.0"
LinkDir["MongoCXX"]				= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/lib/mongocxx-v_noabi-rhs-md"
LinkDir["Bson"]					= "%{vcpkgdir}/libbson_x64-windows/lib/bson-1.0"
LinkDir["BsonCXX"]				= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/lib/bsoncxx-v_noabi-rhs-md"

LinkDir["MongoCd"]				= "%{vcpkgdir}/mongo-c-driver_x64-windows/debug/lib/mongoc-1.0"
LinkDir["MongoCXXd"]			= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/debug/lib/mongocxx-v_noabi-dhs-mdd"
LinkDir["Bsond"]				= "%{vcpkgdir}/libbson_x64-windows/debug/lib/bson-1.0"
LinkDir["BsonCXXd"]				= "%{vcpkgdir}/mongo-cxx-driver_x64-windows/debug/lib/bsoncxx-v_noabi-dhs-mdd"

LinkDir["SDL"]					= "%{os.getcwd()}/Client/Vendor/SDL/build/%{cfg.buildcfg}/SDL3"

include "Client"
include "Server"