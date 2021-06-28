workspace "DRMApproximation"
    configurations {"Debug", "Release"}
    platforms {"x64"}

if os.host() == "Windows" then
    filter {"platforms:x64"}
        system "Windows"
        architecture "x86_64"
        defines "WINDOWS"
end

if os.host() == "macosx" then
    filter {"platforms:x64"}
        system "macosx"
        defines "MAC"
end

if os.host() == "linux" then
    filter {"platforms:x64"}
        system "Linux"
        architecture "x86_64"
        defines "LINUX"
end

project "DRMApproximation"
    kind "ConsoleApp"
    language"C++"
    targetdir "Build/%{cfg.buildcfg}"

   files { "Headers/*.h", "Source/*.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"