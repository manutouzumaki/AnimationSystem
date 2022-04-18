@ECHO OFF

SET compilerFLags= -Od -nologo -Gm- -GR- -Oi -WX -W3 -MDd -wd4530 -wd4201 -wd4100 -wd4189 -wd4505 -wd4101 -Zi
SET linkerFlags= -incremental:no SDL2.lib SDL2main.lib opengl32.lib shell32.lib assimp-vc143-mt.lib
SET includeSDLPath="D:\Libs\SDL2\include" 
SET includeGLADPath="D:\Libs\glad\include"
SET includeASSIMPPath="D:\Libs\assimp\include"
SET includeGLMPath="D:\Libs\glm"
SET linkerSDLPath="D:\Libs\SDL2\lib\x64"
SET linkerASSIMPPath="D:\Libs\assimp\lib\Release"
SET defines=-D_DEBUG
SET src=..\src\main.cpp ..\src\stb_image.cpp ..\src\glad.c ..\imgui\backends\imgui_impl_sdl.cpp ..\imgui\backends\imgui_impl_opengl3.cpp ..\imgui\imgui*cpp
SET includes=..\imgui /I..\imgui\backends

PUSHD ..\build
cl %compilerFLags% %src% -Fesdl_platform %defines% /I%includes% /I%includeSDLPath% /I%includeGLADPath% /I%includeASSIMPPath% /I%includeGLMPath% /link /LIBPATH:%linkerSDLPath% /LIBPATH:%linkerASSIMPPath% /SUBSYSTEM:CONSOLE %linkerFlags%
POPD
