g++ .\src\imgui_demo.cpp .\src\imgui_draw.cpp .\src\imgui_impl_glfw.cpp .\src\imgui_impl_opengl3.cpp .\src\imgui_tables.cpp .\src\imgui_widgets.cpp .\src\imgui.cpp  .\src\BossState.cpp -I .\src\API\GLEW\include\ -I .\src\API\GLFW\include\ -L .\src\API\GLEW\bin\Release\x64 -L .\src\API\GLFW\lib-mingw-w64 -L .\src\  -l glew32 -l glfw3 -l MinHook.x64 -l OpenGL32 -shared -fPIC -o BossState.dll -finput-charset=UTF-8 -fexec-charset=UTF-8