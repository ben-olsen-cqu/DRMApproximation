workspace "DRMApproximation"
    configurations {"Debug", "Release"}

project "DRMApproximation"
    kind "ConsoleApp"
    language"C++"
    targetdir "Build/%{cfg.buildcfg}"

   files { "**.h", "**.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"