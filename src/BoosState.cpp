/*
 * Copyright 2025, LiserverYang. All rights reserved.
 * MIT Licence
 * 这是一个基于modengine2的艾尔登法环mod，用于读取锁定的敌人信息，并显示在ImGui窗口上，灵感和实现方法启发于Hexinton团队制作，SilverCelty汉化的法环ct表，在此感谢！
 * 如果你想要添加敌人的更多信息，可以在ct表上查询相应的地址和偏移量，LastLockOnTarget的值即是g_raxValue的值，具体实现参考Run的内容。
 */

// ------IMPORTS_BEGIN--------

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include <Windows.h>
#include "MinHook.h"

// ------IMPORTS_END--------

// -----GLOBAL_VARIABLES_FOR_HOOK BEGIN------

// 全局变量保存 rax 的值
volatile uint64_t g_raxValue = 0;

HANDLE EldenRingHandle;

// 目标指令在游戏中的偏移
const uintptr_t TARGET_OFFSET = 0x71737E;

// 原始函数指针和钩子函数定义
typedef void (*TargetFunc)();
TargetFunc originalFunc = nullptr;

// -----GLOBAL_VARIABLES_FOR_HOOK END------

// 裸函数处理钩子，保存 rax 并跳转回原流程
__attribute__((naked)) void HookHandler()
{
	asm volatile(
		// 保存 rax 到全局变量
		"movq %%rax, %0\n\t"
		// 跳转回原始函数
		"jmp *%1\n\t"
		:
		: "m"(g_raxValue), "r"(originalFunc) // 操作数约束
		: "rax", "memory"					 // 破坏列表声明
	);
}

// ------GLOBAL_VARIABLES BEGIN--------

GLFWwindow *window;
ImGuiContext *context;

std::thread mod_thread;
bool shouldExit = false;

// ------GLOBAL_VARIABLES END--------

void InitStyle(ImGuiIO &io)
{
	io.Fonts->AddFontFromFileTTF("./BoosState/SourceCodePro-Regular.ttf", 18, NULL, io.Fonts->GetGlyphRangesChineseFull());

	ImGuiStyle &style = ImGui::GetStyle();

	style.WindowBorderSize = 0;
	style.ScrollbarSize = 15;

	style.FrameRounding = 6;
	style.GrabRounding = 6;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiViewportFlags_NoDecoration;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiCol_DockingEmptyBg;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	io.ConfigViewportsNoAutoMerge = true;
}

void InitUI()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(1000, 800, "TestImgui", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext(NULL);
	ImGuiIO &io = ImGui::GetIO();

	InitStyle(io);

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// 安全读取内存函数（避免崩溃）
// The function that can read memory safely (To not crash)
uintptr_t SafeReadMemory(uintptr_t address, SIZE_T size = sizeof(uintptr_t))
{
	uintptr_t value = 0;
	if (!ReadProcessMemory(
			EldenRingHandle,
			reinterpret_cast<LPCVOID>(address),
			&value,
			size,
			nullptr))
	{
		return 0; // 读取失败返回 0
	}
	return value;
}

float SafeReadFloatMemory(uintptr_t address) {
    float value = 0.0f;
    BOOL success = ReadProcessMemory(
        GetCurrentProcess(),               // 当前进程句柄
        reinterpret_cast<LPCVOID>(address), // 目标地址
        &value,                             // 接收缓冲区
        sizeof(float),                      // 读取大小
        nullptr                             // 不需要实际读取字节数
    );
    return (success) ? value : 0.0f;        // 失败返回0.0f
}

// 指针链解引用函数
uintptr_t ResolvePointerChain(
	uintptr_t base_address,
	const std::vector<uintptr_t> &offsets)
{
	uintptr_t current = base_address;

	for (size_t i = 0; i < offsets.size(); ++i)
	{
		current += offsets[i];

		// 非最后一个偏移量时需要解引用
		if (i != offsets.size() - 1)
		{
			current = SafeReadMemory(current);
			if (current == 0)
				return 0; // 提前终止
		}
	}

	return current;
}

void CreateSubWindow()
{
	GLFWwindow *backup_currect_context = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(backup_currect_context);
}

void Run()
{
	InitUI();

	while (!glfwWindowShouldClose(window) && !shouldExit)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();

		ImGuiWindowClass topmost;
		topmost.ClassId = ImHashStr("TopMost");
		topmost.ViewportFlagsOverrideSet = ImGuiViewportFlags_TopMost;
		ImGui::SetNextWindowClass(&topmost);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0, 0, 0, 0.5f});
		ImGui::Begin("BoosState");

		auto NpcId = SafeReadMemory(ResolvePointerChain(g_raxValue, {0x28, 0x124}), 4);
		auto NpcHealth = SafeReadMemory(ResolvePointerChain(g_raxValue, {0x190, 0x0, 0x138}), 4);
		auto NpcMaxHealth = SafeReadMemory(ResolvePointerChain(g_raxValue, {0x190, 0x0, 0x13C}), 4);
		auto NpcToughness = SafeReadFloatMemory(ResolvePointerChain(g_raxValue, {0x190, 0x40, 0x10}));
		auto NpcAnimation = SafeReadMemory(ResolvePointerChain(g_raxValue, {0x190, 0x18, 0x40}), 4);

		ImGui::Text(("NpcId:        " + std::to_string(NpcId)).c_str());
		ImGui::Text(("NpcHealth:    " + std::to_string(NpcHealth)).c_str());
		ImGui::Text(("NpcMaxHealth: " + std::to_string(NpcMaxHealth)).c_str());
		ImGui::Text(("NpcToughness: " + std::to_string(NpcToughness)).c_str());
		ImGui::Text(("NpcAnimation: " + std::to_string(NpcAnimation)).c_str());

		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		CreateSubWindow();

		glfwSwapBuffers(window);
		glfwPollEvents();

		Sleep(100);
	}
}

bool WINAPI DllMain(HINSTANCE dll_instance, unsigned int fdw_reason, void *lpv_reserved)
{
	if (fdw_reason == DLL_PROCESS_ATTACH)
	{
		// 初始化 MinHook
		// Init MinHook
		if (MH_Initialize() != MH_OK)
		{
			return FALSE;
		}

		// 计算目标地址（游戏基址 + 偏移）
		// Get the address of target
		uintptr_t baseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
		uintptr_t targetAddress = baseAddress + TARGET_OFFSET;

		EldenRingHandle = GetCurrentProcess();

		// 创建钩子
		if (MH_CreateHook(
				reinterpret_cast<LPVOID>(targetAddress),
				(void *)(&HookHandler),
				reinterpret_cast<LPVOID *>(&originalFunc)) != MH_OK)
		{
			std::cerr << "Hook faild";

			return FALSE;
		}

		// 启用钩子
		if (MH_EnableHook(reinterpret_cast<LPVOID>(targetAddress)) != MH_OK)
		{
			std::cerr << "Hool faild";

			return FALSE;
		}

		// 运行ImGui
		mod_thread = std::thread(&Run);
	}
	else if (fdw_reason == DLL_PROCESS_DETACH && lpv_reserved != nullptr)
	{
		shouldExit = true;
		mod_thread.join();
	}

	return TRUE;
}