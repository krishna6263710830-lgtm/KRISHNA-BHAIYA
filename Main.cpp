#include <Includes.h>
#include <fakedlfcn/fake_dlfcn.h>
#include <MemoryTool/ScanEngine.hpp>
#include <XorStr.hpp>



const char *LocalHacks_WallHack =
( "#version 310 es\n"
        "\n"
        "precision highp float;\n"
        "\n"
        "out vec4 fragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "fragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "gl_FragDepth = 1.0;\n"
        "}\n");


#include "imgui/imgui.h"
#include "imgui/imgui_additional.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/android_native_app_glue.h"
#include "imgui/fonts/GoogleSans.h"

#include "InfoDisplay.h"
#include "Godx/Godx.h"
#include "Godx/obfuscate.h"
#include "xhook/xhook.h"

#include "imgui/fonts/Iconcpp.h"
#include "imgui/fonts/Icon.h"
#include "imgui/fonts/AwesomeData.h"
#include "imgui/fonts/NotoSans.h"
#include "imgui/fonts/NotoSansBold.h"

#include <Logger.h>
#include <Utils.h>
#include <Macros.h>
#include <unordered_set>
#include <GLES/gl.h>
#include <dlfcn.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <chrono>
#include <sys/sendfile.h>


ImFont *SmallFont;
ImFont *BoldFont;

ImFont* GetSmallFont();
ImFont* GetBoldFont();

void SetSmallFont(ImFont* font);
void SetBoldFont(ImFont* font);

bool Bypass;
bool FPS;
bool KingsNoSmoke;
bool KingsNoWater;
bool KingsBlackSky;
bool KingsNoGrass;
bool KingsNoTree;
bool KingsLoot;
//bool KingsNoBuilding;
//bool KingsWeapons;

bool LocalHacksGfx_ColourBodyChecked = false;
bool LocalHacksGfx_ColourBody1Checked = false;
bool showWorkDialog = false;
          bool movePBChecked = false;
          bool copyPaksChecked = false;
          bool movePBStarted = false;
          bool copyPaksStarted = false;

uintptr_t g_UE4;

android_app *g_App = 0;

#include "UserLogin.cpp"
#include "Config.h"
#include "Tools.h"

bool initImGui = false;
int screenWidth = -1, glWidth, screenHeight = -1, glHeight;
float density = -1;
static float isRed = 0.0f, isGreen = 0.01f, isBlue = 0.0f;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//      
      #define GNativeAndroidApp_Offset 0xe497e98 // GL/KR/TW 64 BIT ( 4.0 )
    // #define GNativeAndroidApp_Offset 0xA4F3414 // GL/KR/TW 32 BIT ( 4.0 )
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//      
   //  #define GNativeAndroidApp_Offset 0xd8a6b80 // BGMI 64 BIT ( 3.9 )
   // #define GNativeAndroidApp_Offset 0x9a69a94 // BGMI 32 BIT ( 3.9 ) 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "shader.h"
#include "CreshFix.h"
#include "BlackScreenFix.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
std::unordered_set<GLuint> playerPrograms;
GLuint playerVertexShader = 0, playerMaskShader = 0;

std::mutex playerProgramsMutex;
std::mutex playerShaderMutex;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int OpenURL(const char* url)
{
    JavaVM* java_vm = g_App->activity->vm;
    JNIEnv* java_env = NULL;

    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "AndroidThunkJava_LaunchURL", "(Ljava/lang/String;)V");
    if (method_id == NULL)
        return -4;
        
    jstring retStr = java_env->NewStringUTF(url);
    java_env->CallVoidMethod(g_App->activity->clazz, method_id, retStr);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int LOCALHACKS() {
    char prop_value[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.sdk", prop_value);
    return atoi(prop_value);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void HueText(const char* text, ImVec4 color)
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() / 1000.0f; 
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 0.0f)); 
    for (int i = 0; i < strlen(text); i++)
    {
      ImGui::SameLine();
        float t = fmodf(elapsed_seconds + (float)i / (float)strlen(text), 1.0f); 
        ImVec4 currentColor = ImVec4(color.x * t, color.y * t, color.z * t, color.w); 
        ImGui::PushStyleColor(ImGuiCol_Text, currentColor); 
        ImGui::Text("%c", text[i]); 
        ImGui::PopStyleColor(); 
    }
    ImGui::PopStyleColor(); 
}

#define CREATE_COLOR(r, g, b, a) new float[4] {(float)r, (float)g, (float)b, (float)a};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
struct sRegion {
    uintptr_t start, end;
};

std::vector<sRegion> trapRegions;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void DrawText(std::string text) {
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(text.c_str()).x) / 2.f);
    ImGui::Text(text.c_str());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern ImGuiDemoMarkerCallback  GImGuiDemoMarkerCallback;
extern void* GImGuiDemoMarkerCallbackUserData;
ImGuiDemoMarkerCallback         GImGuiDemoMarkerCallback = NULL;
void* GImGuiDemoMarkerCallbackUserData = NULL;
#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void DrawESP(ImDrawList *draw) {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
/*
draw->AddText({((float) density / 14.0f), 150}, IM_COL32(255,255,0, 255),_enc((ICON_FA_KEY " @YouTubeBhanuOP")));
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
   if (FPS) {
        static float fps = 0.0f;
        fps = ImGui::GetIO().Framerate;
        char fpsText1[32];
        snprintf(fpsText1, sizeof(fpsText1), _enc("FPS :- %.2f"), fps);
        draw->AddText(NULL,30.0f,{((float) density / 10.0f), 40}, IM_COL32(0, 255, 0, 255),fpsText1);
        }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static float isAutoSlider = -0.20;
	   ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 13.0f),{(float) glWidth * isAutoSlider + glWidth / 40,650}, IM_COL32(100, 223, 0, 255), _enc("Owner :-: @SKYHORAA < We Are The First to Provide Hacks On Entire Telegram > "));
    if(isAutoSlider)
    {
        auto isFramese = ImGui::GetFrameCount();
        if(isFramese % 20 == 0)
        {
            if(isAutoSlider > 1)
            {
                isAutoSlider = -0.20;
            }
            isAutoSlider += 0.01;
        }
    }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

std::string credit = _enc(ICON_FA_TELEGRAM" Telegram Channel [ @GODXOPGOD ] [ SKYHORAA ]");
	
    auto textSize = ImGui::CalcTextSize2(credit.c_str(), 0, ((float) density / 13.0f));
    draw->AddText(NULL, ((float) density / 14.0f), {((float) glWidth / 2) - (textSize.x / 2), 6}, ImColor(IM_COL32(225, 0, 0, 255)), credit.c_str());
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~[TINE AND DAY]~~~~~~~~~~~~~~~~~~~~~~//


time_t lt;
struct tm *t_m;
lt = time(NULL);
t_m = localtime(&lt);

int time_h = t_m->tm_hour;
int time_m = t_m->tm_min;
int time_s = t_m->tm_sec;

int day = t_m->tm_mday;
int month = t_m->tm_mon + 1; // tm_mon ayın 0-based indeksi olduğu için +1 eklenir
int year = t_m->tm_year + 1900; // tm_year yılın 1900'den başlayan değeri olduğu için 1900 eklenir

std::string time;

if (time_h < 10)
    time += "0";

time += std::to_string(time_h) + ":";

if (time_m < 10)
    time += "0";

time += std::to_string(time_m) + ":";

if (time_s < 10)
    time += "0";

time += std::to_string(time_s);

std::string date = std::to_string(day) + "/" + std::to_string(month) + "/" + std::to_string(year);

std::string dayOfWeek;
switch (t_m->tm_wday) {
    case 0:
        dayOfWeek = "Sunday";
        break;
    case 1:
        dayOfWeek = "Monday";
        break;
    case 2:
        dayOfWeek = "Tuesday";
        break;
    case 3:
        dayOfWeek = "Wednesday";
        break;
    case 4:
        dayOfWeek = "Thursday";
        break;
    case 5:
        dayOfWeek = "Friday";
        break;
    case 6:
        dayOfWeek = "Saturday";
        break;
}

std::string dateTime = dayOfWeek + ", " + date + " " + time;
// ImGui::Text(dateTime.c_str());		

 textSize = ImGui::CalcTextSize(time.c_str(), 0, ((float) density / 20.0f));
draw->AddText(NULL, ((float) density / 17.0f), {((float) glWidth / 1.5) - (textSize.x / 2), 50}, IM_COL32(173, 255, 47, 255), dateTime.c_str());
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/*//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    time_t lt;
    struct tm *t_m;
    lt = time(NULL);
    t_m = localtime(&lt);
    int time_h = t_m->tm_hour;
    int time_m = t_m->tm_min;
    int time_s = t_m->tm_sec;
    std::string time;
    if (time_h < 10)
        time += "0";
    time += std::to_string(time_h) + " : ";
    if (time_m < 10)
        time += "0";
    time += std::to_string(time_m) + " : ";
    if (time_s < 10)
        time += "0";
    time += std::to_string(time_s) + " ";
draw->AddText(NULL, ((float) density / 15.0f), {(float) glWidth / 150 + glWidth / 2, 630}, IM_COL32(255, 255, 255, 255), time.c_str()); */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
if (LOCALHACKS() == 36) { // 安卓16支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(215, 0, 0, 255),
"\nANDROID: 16");
}
if (LOCALHACKS() == 35) { // 安卓15支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(215, 0, 0, 255),
"\nANDROID: 15");
}
if (LOCALHACKS() == 34) { // 安卓14支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(100, 0.2, 0.1, 255),
"\nANDROID: 14");
}
if (LOCALHACKS() == 33) { // 安卓13.1支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(100, 255, 225, 255),
"\nANDROID: 13.1");
}
if (LOCALHACKS() == 33) { // 安卓13支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(100, 255, 225, 255),
"\nANDROID: 13");
}
if (LOCALHACKS() == 32) { // 安卓12.1支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(255, 215, 215, 255),
"\nANDROID: 12.1");
}
if (LOCALHACKS() == 31) { // 安卓12支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(215, 255, 100, 255),
"\nANDROID: 12");
}
if (LOCALHACKS() == 30) { // 安卓11支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(255, 255, 255, 255),
"\nANDROID: 11");
}
if (LOCALHACKS() == 29) { // 安卓10支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(255,255,255, 255),
"\nANDROID: 10");
}
if (LOCALHACKS() == 28) { // 安卓9支持
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(25, 255, 255, 255),
"\nANDROID: 9");
}
if (LOCALHACKS() < 28) { // 低于安卓9提示
draw->AddText(NULL, ((float) density / 15.0f),{(float) glWidth / 150 + glWidth / 20,640},IM_COL32(200, 100, 255, 109),
"\nANDROID: 8");
}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

bool WriteAddr(void *addr, void *buffer, size_t length) {
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy(addr, buffer, length) != 0;
}

template<typename T>
void Write(uintptr_t addr, T value) {
    WriteAddr((void *) addr, &value, sizeof(T));
}

int WriteFloat(long int addr, float value) {
    Tools::WriteAddr((void *)(addr), (void *)&value, 4);
    return 0;
}

int WriteQword(long int addr, int64_t value) {
    Tools::WriteAddr((void *)(addr), (void *)&value, sizeof(value));
    return 0;
}

void *run_thread(void *arguments) {
	switch ((int)arguments) {
	}
	return 0;
}

int32_t (*orig_onInputEvent)(struct android_app *app, AInputEvent *inputEvent);
int32_t onInputEvent(struct android_app *app, AInputEvent *inputEvent) {
    if (initImGui) {
        ImGui_ImplAndroid_HandleInputEvent(inputEvent, {(float) screenWidth / (float) glWidth, (float) screenHeight / (float) glHeight});
    }
    return orig_onInputEvent(app, inputEvent);
}

//~~~~~~~~~~~~~~~~~~~[  🇮🇳 COPY SYSTEM 🇮🇳  ]~~~~~~~~~~~~~~~~~~~~//
// ADD DROW ESP
std::atomic<bool> isWorking(false);
std::atomic<float> workProgress(0.0f);
std::atomic<size_t> totalBytes(0);
std::atomic<size_t> copiedBytes(0);
std::atomic<bool> cancelRequested(false);
std::string workStatus = "";
float liveSpeedMBps = 0.0f;
bool showSuccessPopup = false;

// Toast system
struct Toast {
    std::string msg;
    std::chrono::steady_clock::time_point start;
    float duration;
};
std::vector<Toast> activeToasts;

void ShowToast(const std::string& msg, float duration = 3.0f) {
    activeToasts.push_back({msg, std::chrono::steady_clock::now(), duration});
}

//_____________________[ Developer @YouTubeBhanuOP ]____________________//

size_t getFolderSize(const std::string& path) {
    size_t size = 0;
    DIR* dir = opendir(path.c_str());
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        std::string fullPath = path + "/" + entry->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                size += getFolderSize(fullPath);
            } else {
                size += st.st_size;
            }
        }
    }
    closedir(dir);
    return size;
}

//_____________________[ Developer @YouTubeBhanuOP ]____________________//

void copyFile(const std::string& src, const std::string& dst) {
    int inFd = open(src.c_str(), O_RDONLY);
    if (inFd < 0) {
        perror(("open src failed: " + src).c_str());
        return;
    }

    struct stat st;
    if (fstat(inFd, &st) != 0) {
        close(inFd);
        return;
    }
    size_t fileSize = st.st_size;

    // ensure parent dirs
    size_t pos = dst.find_last_of('/');
    if (pos != std::string::npos) {
        std::string dirPath = dst.substr(0, pos);
        std::string cmd = "mkdir -p \"" + dirPath + "\"";
        system(cmd.c_str());
    }

    int outFd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outFd < 0) {
        perror(("open dst failed: " + dst).c_str());
        close(inFd);
        return;
    }

    size_t offset = 0;
    auto lastTime = std::chrono::steady_clock::now();
    size_t bytesThisInterval = 0;

    while (offset < fileSize && !cancelRequested) {
        size_t toCopy = std::min((size_t)16 * 1024 * 1024, fileSize - offset); // 16 MB chunks
        ssize_t sent = sendfile(outFd, inFd, (off_t*)&offset, toCopy);
        if (sent <= 0) break;

        copiedBytes += sent;
        bytesThisInterval += sent;

        workProgress = (float)copiedBytes / (float)totalBytes * 100.0f;

        // speed calc every 200ms
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() > 200) {
            liveSpeedMBps = (float)bytesThisInterval / (1024.0f * 1024.0f) /
                            (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() / 1000.0f);
            bytesThisInterval = 0;
            lastTime = now;
        }
    }

    close(inFd);
    close(outFd);
}

//_____________________[ Developer @YouTubeBhanuOP ]____________________//

void copyFolder(const std::string& src, const std::string& dst) {
    struct stat st;
    if (stat(dst.c_str(), &st) != 0) {
        mkdir(dst.c_str(), 0777);
    }
    DIR* dir = opendir(src.c_str());
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && !cancelRequested) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        std::string srcPath = src + "/" + entry->d_name;
        std::string dstPath = dst + "/" + entry->d_name;

        struct stat st2;
        if (stat(srcPath.c_str(), &st2) == 0) {
            if (S_ISDIR(st2.st_mode)) {
                copyFolder(srcPath, dstPath);
            } else {
                workStatus = "Copying: " + std::string(entry->d_name);
                copyFile(srcPath, dstPath);
            }
        }
    }
    closedir(dir);
}

//_____________________[ Developer @YouTubeBhanuOP ]____________________//

void StartCopy(const std::vector<std::pair<std::string, std::string>>& paths) {
    isWorking = true;
    workProgress = 0.0f;
    copiedBytes = 0;
    totalBytes = 0;
    liveSpeedMBps = 0.0f;
    cancelRequested = false;
    showSuccessPopup = false;
    workStatus = "Preparing...";

    std::thread([paths] {
        // calc total size
        totalBytes = 0;
        for (auto& p : paths) {
            struct stat st;
            if (stat(p.first.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode))
                    totalBytes += getFolderSize(p.first);
                else
                    totalBytes += st.st_size;
            }
        }
        workStatus = "Copying Files...";
        for (auto& p : paths) {
            if (cancelRequested) break;
            struct stat st;
            if (stat(p.first.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode))
                    copyFolder(p.first, p.second);
                else
                    copyFile(p.first, p.second);
            }
        }
        if (!cancelRequested) {
            workProgress = 100.0f;
            workStatus = "All Files Copied Successfully.";
            showSuccessPopup = true;
            ShowToast("Thanks @GODXOPGOD", 3.0f);
        } else {
            workStatus = " Copy Cancelled.";
        }

        isWorking = false;
    }).detach();
}

//~~~~~~~~~~~~~~~~~[  🌍 COPY SYSTEM 🌍  ]~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean _eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);
    
    if (glWidth <= 0 || glHeight <= 0)
        return orig_eglSwapBuffers(dpy, surface);
        
    if (!g_App)
        return orig_eglSwapBuffers(dpy, surface);
        
    if (g_App->onInputEvent != onInputEvent) {
        orig_onInputEvent = decltype(orig_onInputEvent)(g_App->onInputEvent);
        g_App->onInputEvent = onInputEvent;
    }
    
    screenWidth = ANativeWindow_getWidth(g_App->window);
    screenHeight = ANativeWindow_getHeight(g_App->window);
    density = AConfiguration_getDensity(g_App->config);
    
    if (!initImGui) {
		ImGui::CreateContext();
		InitializeInfoGraphics(); 
		ImGuiStyle* style = &ImGui::GetStyle();
		
        style->WindowRounding = 11.0f;
        style->FramePadding = ImVec2(4,3);
        style->WindowPadding = ImVec2(10,10);
        style->WindowRounding = 11.0f;
        style->FramePadding = ImVec2(4,3);
        style->FrameRounding = 11.0f;
        style->ItemSpacing = ImVec2(8, 8);
        style->ItemInnerSpacing = ImVec2(6, 6);
        style->CellPadding = ImVec2(5, 5);
        style->IndentSpacing = 23.0f;
        
        style->GrabMinSize = 5.0f;
        style->GrabRounding = 3.0f;
        style->PopupRounding = 2.f;
        style->Alpha = 1.0;
        style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style->TouchExtraPadding = ImVec2(0, 0);
        style->TabRounding = 7.0f;

        style->ChildRounding = 7.0f;
        style->ScrollbarSize = 30.0f;
        style->ScrollbarRounding = 9.0f;
        style->ButtonTextAlign = ImVec2(0.5,0.5);

        style->ChildBorderSize = 0.100f;

        style->DisplayWindowPadding = ImVec2(22, 22);
        style->DisplaySafeAreaPadding = ImVec2(4, 4);
        style->AntiAliasedLines = true;
        style->CurveTessellationTol = 1.f;
        style->TabBorderSize = 0.000f;
        ImVec4 *colors = style->Colors;
        
        style->Colors[ImGuiCol_Text]                  = ImColor(0, 255, 255, 255);
        style->Colors[ImGuiCol_WindowBg]              = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		ImGuiIO* io = &ImGui::GetIO();
		io->IniFilename = nullptr;

		ImGui_ImplAndroid_Init();
		ImGui_ImplOpenGL3_Init("#version 300 es");

        io->ConfigWindowsMoveFromTitleBarOnly = true;
        io->IniFilename = NULL;
        static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
        ImFontConfig font_config;
        ImFontConfig icons_config;
        ImFontConfig CustomFont;
        CustomFont.FontDataOwnedByAtlas = false;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.OversampleH = 2.5;
        icons_config.OversampleV = 2.5;
        //内存字体 load
        io->Fonts->AddFontFromMemoryTTF((void *)XT_data, XT_size, 25.0f, NULL, io->Fonts->GetGlyphRangesChineseFull());
        io->Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 25.0f, &icons_config, icons_ranges); 
   
                config_t::misc::color = CREATE_COLOR(1.0f, 0.0f, 0.0f, 1.0f);
                
        initImGui = true;
    }

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
    ImGui::NewFrame();
    RenderInfoDisplay();
    DrawESP(ImGui::GetBackgroundDrawList());
    
         auto isFrames = ImGui::GetFrameCount();
    if(isFrames % 1 == 0) {

        if(isGreen == 0.01f && isBlue == 0.0f){
            isRed += 0.01f;
             }

        if(isRed > 0.99f && isBlue == 0.0f){
            isRed = 1.0f;
            isGreen += 0.01f;
            }

        if(isGreen > 0.99f && isBlue == 0.0f){
            isGreen = 1.0f;
            isRed -= 0.01f;
        }

       if(isRed < 0.01f && isGreen == 1.0f){
            isRed = 0.0f;
            isBlue += 0.01f;
            }

        if(isBlue > 0.99f && isRed == 0.0f){
            isBlue = 1.0f;
            isGreen -= 0.01f;
            }
        if(isGreen < 0.01f && isBlue == 1.0f){
            isGreen = 0.0f;
            isRed += 0.01f;
            }
        if(isRed > 0.99f && isGreen == 0.0f){
            isRed = 1.0f;
            isBlue -= 0.01f;
            }
        if(isBlue < 0.01f && isGreen == 0.0f){
            isBlue = 0.0f;
            isRed -= 0.01f;
            if(isRed < 0.01f)
            isGreen = 0.01f;
        }
        ImGui::PushStyleColor( ImGuiCol_Border,ImColor(isRed, isBlue, isGreen, 8.0f).Value);
ImGui::PushStyleColor( ImGuiCol_Text,ImColor(isRed, isBlue, isGreen, 8.0f).Value);
    }
           ImVec2 center = ImGui::GetMainViewport()->GetCenter();
           ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
           ImGui::SetNextWindowSize(ImVec2((float) glWidth * 0.28f, (float) glHeight * 0.55f), ImGuiCond_Once);
    if (ImGui::Begin(_enc("GODXOPGOD [ 4.3 ] WALL HACK PBC"), 0, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {

        static bool isLogin = true;
        static std::string err;
        if (!isLogin) {
            ImGui::Text("USER AUTHENTICATION.");
            ImGui::PushItemWidth(-1);
            static char s[64];
            ImGui::InputText("##key", s, sizeof s);
            ImGui::PopItemWidth();

            if (ImGui::Button("Paste License", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
                auto key = getClipboardText();
                strncpy(s, key.c_str(), sizeof s);
            }

            if (ImGui::Button("Verify License", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
                err = Login(s);
                if (err == "OK") {
                    isLogin = bValid && g_Auth == g_Token;
                }
            }

            if (!err.empty() && err != "OK") {
                ImGui::TextWrapped("%s", err.c_str());
            }
        } else {
        
        ImGui::TextColored(ImVec4(2.55f, 0.00f, 1.00f, 1.00f), "GODXOPGOD     FPS %.1f", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::TextColored(ImVec4(2.55f, 1.00f, 0.00f, 1.00f), " Server Running %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                                         static float values[90]    = {};
                                         static int values_offset   = 0;
                                         static double refresh_time = 0.0;
                                         while( refresh_time < ImGui::GetTime() )
                          {
                          static float phase    = 0.0f;
                                         values[values_offset] = cosf( phase );
                                         values_offset = ( values_offset + 1 ) % IM_ARRAYSIZE( values );
                                         phase += 0.10f * values_offset;
                                         refresh_time += 1.0f / 60.0f;
                                         }
                    char overlay[32] = "";
                    ImGui::PlotLines("", values, IM_ARRAYSIZE( values ), values_offset, overlay, -1.0f, 1.0f, ImVec2( 0,20.0f ) );
          if (ImGui::Button("JOIN CHANNEL", ImVec2(180, 50)))
                   OpenURL(_enc("https://t.me/GODXOPGOD"));
                   ImGui::SameLine();
                   if (ImGui::Button("APPLY PROCESS", ImVec2(180, 50)))
                   OpenURL(_enc("https://t.me/GODXOPGOD/1935"));
                             ImGui::Separator();
          ImColor color(0, 255, 255);     
          ImColor GODXOP1(255, 0, 0);
          HueText("       GODXOPGOD", GODXOP1);
          ImGui::Separator();
         
             ImGui::Checkbox("NO GRASS: ", &KingsNoGrass);          
			 ImGui::SameLine();
			 ImGui::Checkbox("BLACK SKY ", &KingsBlackSky);  
			 ImGui::Separator();
			 ImGui::Checkbox("NO TREE : ", &KingsNoTree);  
			 ImGui::SameLine();
			 ImGui::Checkbox("NO WATER : ", &KingsNoWater);  
			 ImGui::Separator();
			 ImGui::Checkbox("NO SMOKE : ", &KingsNoSmoke);  
			 ImGui::SameLine();
             ImGui::Checkbox("LOOT COLOR : ", &KingsLoot);             
             ImGui::Separator();
          /*   ImGui::Checkbox("REMOVE BUILDINGS ", &KingsNoBuilding);
             ImGui::SameLine();
             ImGui::Checkbox("WEAPONS ", &KingsWeapons);       */             
             
//ADD UI BUTTONS KA JAGA PE
void ShowCopyDialog(); {
    // Welcome toast trigger (run once at start)
    static bool welcomeShown = false;
    if (!welcomeShown) {
        ShowToast(_enc("Welcome TG @GODXOPGOD 4.0f"));
        welcomeShown = true;
    }

    // --- Buttons to start copy ---
// --- Buttons ko Checkbox me convert kiya gaya ---
static bool copyPBC = false;

//~~~~~~~~~~~~~~~~~~~[  🇰🇷KR BUTTON 🇰🇷 ]~~~~~~~~~~~~~~~~~~~~//

if (!isWorking) {
    if (ImGui::Checkbox("Copy ProgramBinaryCache", &copyPBC)) {
        if (copyPBC) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.pubg.krmobile/files/ProgramBinaryCache")),
        std::string(_enc("/storage/emulated/0/Download/@GODXOPGOD/ProgramBinaryCache"))
    });
    StartCopy(paths);
    copyPBC = false; // reset
}
    }

    ImGui::Spacing();

static bool copyPaksObb = false;

if (ImGui::Checkbox("Copy Paks + OBB", &copyPaksObb)) {
    if (copyPaksObb) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.pubg.krmobile/files/UE4Game/ShadowTrackerExtra/ShadowTrackerExtra/Saved/Paks")),
        std::string(_enc("/storage/emulated/0/Download/@GODXOPGOD/Paks"))
    });
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/obb/com.pubg.krmobile")),
        std::string(_enc("/storage/emulated/0/Download/@GODXOPGOD/obb"))
    });

        StartCopy(paths);
        copyPaksObb = false; // reset checkbox
    }
}
}

//~~~~~~~~~~~~~~~~~~~[  🇮🇳 BGMI 🇮🇳  ]~~~~~~~~~~~~~~~~~~~~//
/*
if (!isWorking) {
    if (ImGui::Checkbox("Copy ProgramBinaryCache", &copyPBC)) {
        if (copyPBC) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.pubg.imobile/files/ProgramBinaryCache")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/ProgramBinaryCache"))
    });
    StartCopy(paths);
    copyPBC = false; // reset
}
    }

    ImGui::Spacing();

static bool copyPaksObb = false;

if (ImGui::Checkbox("Copy Paks + OBB", &copyPaksObb)) {
    if (copyPaksObb) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.pubg.imobile/files/UE4Game/ShadowTrackerExtra/ShadowTrackerExtra/Saved/Paks")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/Paks"))
    });
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/obb/com.pubg.imobile")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/obb"))
    });

        StartCopy(paths);
        copyPaksObb = false; // reset checkbox
    }
}
}
*/
//~~~~~~~~~~~~~~~~~~~[  🌍GL BUTTON 🌍 ]~~~~~~~~~~~~~~~~~~~~//
/*
if (!isWorking) {
    if (ImGui::Checkbox("Copy ProgramBinaryCache", &copyPBC)) {
        if (copyPBC) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.tencent.ig/files/ProgramBinaryCache")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/ProgramBinaryCache"))
    });
    StartCopy(paths);
    copyPBC = false; // reset
}
    }

    ImGui::Spacing();

static bool copyPaksObb = false;

if (ImGui::Checkbox("Copy Paks + OBB", &copyPaksObb)) {
    if (copyPaksObb) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.tencent.ig/files/UE4Game/ShadowTrackerExtra/ShadowTrackerExtra/Saved/Paks")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/Paks"))
    });
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/obb/com.tencent.ig")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/obb"))
    });

        StartCopy(paths);
        copyPaksObb = false; // reset checkbox
    }
}
}
*/
//~~~~~~~~~~~~~~~~~~~[  🇻🇳VIETNAM BUTTON🇻🇳 ]~~~~~~~~~~~~~~~~~~~~//
/*
if (!isWorking) {
    if (ImGui::Checkbox("Copy ProgramBinaryCache", &copyPBC)) {
        if (copyPBC) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.vng.pubgmobile/files/ProgramBinaryCache")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/ProgramBinaryCache"))
    });
    StartCopy(paths);
    copyPBC = false; // reset
}
    }

    ImGui::Spacing();

static bool copyPaksObb = false;

if (ImGui::Checkbox("Copy Paks + OBB", &copyPaksObb)) {
    if (copyPaksObb) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.vng.pubgmobile/files/UE4Game/ShadowTrackerExtra/ShadowTrackerExtra/Saved/Paks")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/Paks"))
    });
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/obb/com.vng.pubgmobile")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/obb"))
    });

        StartCopy(paths);
        copyPaksObb = false; // reset checkbox
    }
}
}
*/
//~~~~~~~~~~~~~~~~~~~[  🇹🇼TAIWAN BUTTON🇹🇼  ]~~~~~~~~~~~~~~~~~~~~//
/*
fi (!isWorking) {
    if (ImGui::Checkbox("Copy ProgramBinaryCache", &copyPBC)) {
        if (copyPBC) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.rekoo.pubgm/files/ProgramBinaryCache")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/ProgramBinaryCache"))
    });
    StartCopy(paths);
    copyPBC = false; // reset
}
    }

    ImGui::Spacing();

static bool copyPaksObb = false;

if (ImGui::Checkbox("Copy Paks + OBB", &copyPaksObb)) {
    if (copyPaksObb) {
    std::vector<std::pair<std::string, std::string>> paths;
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/data/com.rekoo.pubgm/files/UE4Game/ShadowTrackerExtra/ShadowTrackerExtra/Saved/Paks")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/Paks"))
    });
    paths.push_back({
        std::string(_enc("/storage/emulated/0/Android/obb/com.rekoo.pubgm")),
        std::string(_enc("/storage/emulated/0/Download/LocalHacksGfx_ColourBody/obb"))
    });

        StartCopy(paths);
        copyPaksObb = false; // reset checkbox
    }
}
}
*/
//~~~~~~~~~~~~~~~~~~~[  🇮🇳 TOST SYSTEM 🇮🇳  ]~~~~~~~~~~~~~~~~~~~~//


    // --- Progress Window ---
    if (isWorking) {
        ImGui::Begin("Copy Progress", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::Text("%s", workStatus.c_str());
        ImGui::Separator();

        float copiedMB = copiedBytes / (1024.0f * 1024.0f);
        float totalMBf = totalBytes / (1024.0f * 1024.0f);

        ImGui::Text("%.2f MB / %.2f MB   (%.0f%%)", copiedMB, totalMBf, workProgress.load());
        ImGui::ProgressBar(workProgress / 100.0f, ImVec2(-1, 20));

     //   ImGui::Text("Speed: %.2f MB/s", liveSpeedMBps);

     if (ImGui::Button("Cancel", ImVec2(300, 0))) {
    cancelRequested = true;
}


        ImGui::End();
    }

    // --- Success Popup ---
    if (showSuccessPopup) {
        ImGui::OpenPopup("Success");
    }

    if (ImGui::BeginPopupModal("Success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(" %s", workStatus.c_str());
        if (ImGui::Button("Done", ImVec2(300, 0))) {
            showSuccessPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // --- Toast Notifications ---
    int yOffset = 50;
    for (size_t i = 0; i < activeToasts.size();) {
        float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - activeToasts[i].start).count() / 1000.0f;

        if (elapsed < activeToasts[i].duration) {
            ImGui::SetNextWindowPos(ImVec2(50, yOffset), ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 220));

            ImGui::Begin(("Toast" + std::to_string(i)).c_str(), nullptr,
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoInputs);

            ImGui::Text("%s", activeToasts[i].msg.c_str());

            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            yOffset += 40;
            ++i;
        } else {
            activeToasts.erase(activeToasts.begin() + i);
        }
    }
}
//___________________
		
	/*    ImGui::Separator();
        DrawText(_enc("GLOW COLOUR ONLY WORKING"));
        DrawText(_enc(" HDR GRAPHICS")); */
        ImGui::Separator();
        DrawText(_enc("DON'T PLAY MOD APK ONLY MAKE"));
        DrawText(_enc("COLOUR DATA AND PLAY ORGINAL APK"));
        }
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return orig_eglSwapBuffers(dpy, surface);
}

ImFont *GetSmallFont() {
    return SmallFont;
}

ImFont *GetBoldFont() {
    return BoldFont;
}

void SetSmallFont(ImFont * font) {
    SmallFont = font;
}

void SetBoldFont(ImFont * font) {
    BoldFont = font;
}

#define DefineHook(RET, NAME, ARGS) \
    RET(*Orig_##NAME) \
    ARGS; \
    RET Hook_##NAME ARGS

typedef long long __int64;
typedef long long _QWORD;

void (*oglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void _glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    if (config_t::misc::wallhack) {
        GLuint currentProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &currentProgram);
        {
            std::lock_guard<std::mutex> lock(playerProgramsMutex);
            if (playerPrograms.find(currentProgram) == playerPrograms.end()) {
                return oglDrawElements(mode, count, type, indices);
            }
        }
        GLfloat depthRange[2];
        glGetFloatv(GL_DEPTH_RANGE, depthRange);
        GLboolean colorWriteMask[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, colorWriteMask);
        GLboolean blend;
        glGetBooleanv(GL_BLEND, &blend);
        if (!blend) {
            glEnable(GL_BLEND);
        }
        glDepthRangef(1.f, 0.0f);
        glColorMask(1, 1, 1, 1);
        if (config_t::misc::enabled) {
            GLint blendSrc, blendDst;
            glGetIntegerv(GL_BLEND_SRC, &blendSrc);
            glGetIntegerv(GL_BLEND_DST, &blendDst);
            GLfloat blendColor[4];
            glGetFloatv(GL_BLEND_COLOR, blendColor);
            glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
            if (config_t::misc::rainbow) {
                static float cnt = 0.0f;

                float r = cos(cnt) * .5f + .5f;
                float g = cos(cnt - 2.f * 3.14 / 3.f) * .5f + .5f;
                float b = cos(cnt - 4.f * 3.14 / 3.f) * .5f + .5f;

                if (cnt >= FLT_MAX) {
                    cnt = 0.0f;
                } else {
                    cnt += 0.01f;
                }

                glBlendColor(r, g, b, 1.0f);
            } else {
              glBlendColor(config_t::misc::color[1], config_t::misc::color[0], config_t::misc::color[1], 1.0f);
            }
            oglDrawElements(mode, count, type, indices);

            glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);
            glBlendFunc(blendSrc, blendDst);
        } else {
            oglDrawElements(mode, count, type, indices);
        }
        if (config_t::misc::colorfull) {
            static float cnt = 0.0f;
            float r = cos(cnt) * .5f + .5f;
            float g = cos(cnt - 2.f * 3.14 / 3.f) * .5f + .5f;
            float b = cos(cnt - 4.f * 3.14 / 3.f) * .5f + .5f;
            if (cnt >= FLT_MAX) {
                    cnt = 0.0f;
            } else {
                    cnt += 0.01f;
            }
            glDepthRangef(1, 0);
            glEnable(GL_BLEND);
            glColorMask(1, 1, 1, 1);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glBlendColor(0, 0, 0, 1);
            oglDrawElements(GL_TRIANGLES, count, type, indices);
            glDepthRangef(0.5, 1);
            glBlendColor(r, g, b, 1.0f);
            oglDrawElements(GL_LINES, count, type, indices);
        } else {
           glBlendColor(config_t::misc::color[1], config_t::misc::color[0], config_t::misc::color[1], 1.0f);
        }
        if (config_t::misc::glow) {
            glEnable(GL_BLEND);
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glLineWidth(1);
            glDepthRangef(0.5, 1);
            oglDrawElements(GL_LINES, count, type, indices);
            glBlendColor(1, 1, 1, 1);
            glDepthRangef(1, 0.5);
            oglDrawElements(GL_TRIANGLES, count, type, indices);
        } else {
         glBlendColor(config_t::misc::color[1], config_t::misc::color[0], config_t::misc::color[1], 1.0f);
        }
        if (config_t::misc::wireframe) {
            glDepthRangef(1, 0);
            glLineWidth(1);
            glEnable(GL_BLEND);
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glBlendColor(0, 0, 0, 1);
            oglDrawElements(GL_TRIANGLES, count, type, indices);
            glDepthRangef(1, 0.5);
            glBlendColor(1, 2, 2, 1);
            oglDrawElements(GL_LINES, count, type, indices);
        } else {
           glBlendColor(config_t::misc::color[1], config_t::misc::color[0], config_t::misc::color[1], 1.0f);
        }
        
        glDepthRangef(depthRange[0], depthRange[1]);
        glColorMask(colorWriteMask[0], colorWriteMask[1], colorWriteMask[2], colorWriteMask[3]);
        if (!blend) {
            glDisable(GL_BLEND);
        }
        return;
    }
    return oglDrawElements(mode, count, type, indices);
}

void (*oglShaderSource)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
void _glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) {
    if (!string || !*string) {
        return oglShaderSource(shader, count, string, length);
    }
    if (!count) {
        return oglShaderSource(shader, count, string, length);
    }
    
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        
        // SMOOTH / Balanced / HD / HDR / PLAYER 👤
    if (strstr(*string,_enc( "Master_Mask")) || strstr(*string, _enc("Master_Mask_Base")) || strstr(*string, _enc("CH_Base_SK_Skeleton")) || strstr(*string,_enc( "M_Body_01_A")) || strstr(*string, _enc("M_Head_01_A")) || strstr(*string, _enc("M_AT_Mask_05")) || strstr(*string, _enc("T_Mask_03_A_D")) || strstr(*string, _enc("T_Mask_04_N")) ||  strstr(*string, _enc("T_Mask_04_RMA")) || strstr(*string, _enc("Base_Skeleton")) || strstr(*string, _enc("M_Shoes_28_A")) ||  strstr(*string, _enc("F_Leg_Bare")) ||  strstr(*string, _enc("Body_lod3_D")) || strstr(*string, _enc("F_Body_01_RMA")) || strstr(*string, _enc("F_Body_03_A_D")) || strstr(*string, _enc("F_Leg_02_White_A_D")) || strstr(*string, _enc("F_Hair_16_int")) || strstr(*string, _enc("Hair_Skeleton")) || strstr(*string, _enc("Master_Mask_Base9")) || strstr(*string, _enc("F_Head_03_A")) || strstr(*string, _enc("T_F_Head_10_int_RMA")) || strstr(*string, _enc("Body_lod3")) || strstr(*string, _enc("F_Body_03_A")) || strstr(*string, _enc("F_Body_03_A_Bra")) || strstr(*string, _enc("F_Leg_03_White_A")) || strstr(*string, _enc("CH_Base_SK_PhysicsAsset")) || strstr(*string, _enc("F_Body_01_black_A_D_Bra_ZD")) || strstr(*string, _enc("M_Mask_06_Pickup")) || strstr(*string, _enc("T_Mask_06_A_D")) || strstr(*string, _enc("M_Jacket_687_int_A")) || strstr(*string, _enc("AT_F_Jacket_687_int_LOD")) || strstr(*string, _enc("M_Jacket_687_int_C")) || strstr(*string, _enc("AT_F_Shirt_01")) || strstr(*string, _enc("AT_F_Shirt_01_LOD")) || strstr(*string, _enc("M_Shirt_01_A")) || strstr(*string,_enc( "Master_Mask_Base_Elves")) || strstr(*string, _enc("Master_Mask_Base_Elves_Player")) || strstr(*string,_enc( "Master_Mask_Base_FireMove")) || strstr(*string, _enc("Master_Mask_Base_MetalCube")) || strstr(*string,_enc( "Master_Mask_Base_Slime")) || strstr(*string, _enc("Master_Mask_Base_Xiyou")) || strstr(*string,_enc( "Master_Mask_Base_Emisive")) || strstr(*string, _enc("Master_Mask_Base_FlowParal")) || strstr(*string,_enc( "Master_Mask_Base_Emoji_Vari")) || strstr(*string, _enc("Master_Mask_Base_Trans")) || strstr(*string,_enc( "Master_Mask_Base_Fly")) || strstr(*string, _enc("Master_Mask_Base_MaskedEmiss"))) {
    GLenum shaderTypes = 0;
    glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = LocalHacks_WallHack;
        }
    }

    // SMOOTH / Balanced / HDR / HD / OUTFIT 2💃🏻🕺🏻 
    if (strstr(*string,_enc( "Custom_Master_Mask_NewMask")) || strstr(*string, _enc("Master_CH_Ice")) || strstr(*string,_enc( "Master_Frost_Base_Player_Opague")) || strstr(*string,_enc( "Master_MirrorCloth")) || strstr(*string, _enc("Master_Planer_Base_shine_Player")) || strstr(*string,_enc( "Master_WaterCooling")) || strstr(*string, _enc("Master_Monster_Base1")) || strstr(*string,_enc( "Master_Opacity_NightVision")) || strstr(*string, _enc("Master_JellyFish_Base")) || strstr(*string,_enc( "Master_Iridescent")) || strstr(*string, _enc("Master_Mask_Base_Player")) || strstr(*string,_enc( "Master_Mask_Base_Player_Mask")) || strstr(*string, _enc("Master_Skin_SSS")) || strstr(*string,_enc( "MM_SoulGirl")) || strstr(*string, _enc("MM_GoldMan")) || strstr(*string,_enc( "MM_Jacket_2035_int")) || strstr(*string, _enc("MM_Jacket_2247_int")) || strstr(*string,_enc( "MM_Jacket_2247_int_2")) || strstr(*string, _enc("MM_Snake")) || strstr(*string,_enc( "M_Nomask")) || strstr(*string, _enc("M_ToonBody")) || strstr(*string,_enc( "M_RedMan")) || strstr(*string, _enc("T_Eye")) || strstr(*string,_enc( "M_MaskEffect")) || strstr(*string,_enc( "M_Jacket_2000_Int")) || strstr(*string, _enc("M_JellyFish_01")) || strstr(*string,_enc( "M_Jacket_2312")) || strstr(*string, _enc("M_Jacket_2593_New")) || strstr(*string,_enc( "M_Jacket_2176_1")) || strstr(*string, _enc("M_Jacket_2427")) || strstr(*string,_enc( "M_Jacket_2422")) || strstr(*string, _enc("M_Jacket_2479")) || strstr(*string,_enc( "M_Jacket_2508")) || strstr(*string, _enc("M_Jacket_2542")) || strstr(*string,_enc( "M_Jacket_2480_Fire_2")) || strstr(*string, _enc("M_Jacket_2372")) || strstr(*string,_enc( "M_Jacket_2660_DT_Tree")) || strstr(*string, _enc("M_Jacket_2630")) || strstr(*string,_enc( "M_Jacket_2605")) || strstr(*string, _enc("M_Jacket_2626_int_A")) || strstr(*string,_enc( "M_Jacket_2626_int_B")) || strstr(*string, _enc("M_Jacket_2487")) || strstr(*string,_enc( "M_Jacket_2593_Translusent")) || strstr(*string, _enc("M_Jacket_2419_int")) || strstr(*string,_enc( "M_Jacket_2542_int_JN_Wave04")) || strstr(*string, _enc("M_Jacket_2414_A")) || strstr(*string,_enc( "M_Jacket_2414_B_DT")) || strstr(*string, _enc("M_Jacket_2414_Cloak")) || strstr(*string,_enc( "M_Jacket_2641")) || strstr(*string, _enc("M_Jacket_2636")) || strstr(*string,_enc( "M_Jacket_2422_int_E")) || strstr(*string, _enc("M_Jacket_2427_Dissolve")) || strstr(*string,_enc( "M_JokerSuit_Intergrate")) || strstr(*string, _enc("M_Suit_JYX")) || strstr(*string,_enc( "M_GoldSuitLevelup_270")) || strstr(*string, _enc("M_GoldSuit_Door")) || strstr(*string,_enc( "M_Suit_WSJ")) || strstr(*string, _enc("M_SuitChangeAnimation")) || strstr(*string,_enc( "M_GoldSuit_Spirit")) || strstr(*string, _enc("M_GoldSuit_Fearless")) || strstr(*string,_enc( "M_GoldSuit_Fearless_Box")) || strstr(*string, _enc("M_ShortFur")) || strstr(*string, _enc("Master_Pet")) || strstr(*string, _enc("M_Pet_int_065")) || strstr(*string,_enc( "Master_Pet_Fur")) || strstr(*string, _enc("M_Snake_Fresnel")) || strstr(*string,_enc( "M_Birdfly")) || strstr(*string, _enc("M_Pet_Cloth_int_054_Outter")) || strstr(*string,_enc( "M_Butterfly_01"))) {
    GLenum shaderTypes = 0;
    glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = LocalHacks_WallHack;
        }
    }

  
       
    
    
      // SMOOTH / Balanced / HD / HDR /  HAIR 🧓🧔🧔‍♀️
     if (strstr(*string,_enc( "M_Character_Dissolve_Broken")) || strstr(*string,_enc("M_GeneralCharacterTransform")) || strstr(*string, _enc("M_Hair_Emi")) || strstr(*string, _enc("M_Hair_TwoPass")) || strstr(*string, _enc("M_Hair_TwoPass_Dissolve")) || strstr(*string, _enc("M_Hat_1708_B")) || strstr(*string, _enc("M_Hat_int_1881")) || strstr(*string,_enc("M_ToonHair")) || strstr(*string,_enc("M_PET_int_065_Outline")) || strstr(*string,_enc("Mat_Hair_V2_Player")) || strstr(*string,_enc("Mat_Hair_V2")) || strstr(*string,_enc("Mat_Hair_V2_Irradiance")) || strstr(*string,_enc( "Mat_Hair_V2_2372")) || strstr(*string,_enc("Mat_Frequency_low")) || strstr(*string,_enc("M_MaskEffect"))) { 
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = LocalHacks_WallHack;
        }
    }
    
    
  
    
    
  
 
  	if (KingsBlackSky) {
        // SMOOTH / Balanced / HDR / HD / SKY ☁️
     if (strstr(*string,_enc("_BaseMat_Sky_SphereTex")) || strstr(*string,_enc("_BaseMat_Sky_lowdevice")) || strstr(*string,_enc("_BaseMat_SkyMoved_SphereTex")) || strstr(*string,_enc("_BaseMat_Sky_DayNight")) || strstr(*string,_enc("_BaseMat_SkySunnyToFoggy_SphereTex")) || strstr(*string,_enc("_BaseMat_Sky_Night")) || strstr(*string,_enc("M_Sky_Panning_Clouds2")) || strstr(*string,_enc("M_NewSky")) || strstr(*string,_enc("M_Cloud")) || strstr(*string,_enc("M_NewSky_02")) || strstr(*string,_enc("M_SkyTimeOfDay_By_Yoohaozhang")) || strstr(*string,_enc("M_Cloud2D")) || strstr(*string,_enc("M_FlowMap_Sphere_01")) || strstr(*string,_enc("SM_skydome_mat")) || strstr(*string,_enc("Lobby_sky_mat")) || strstr(*string,_enc("SkySphere_SphereMap"))) {
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = newMaskBlack;
        }
    }
	}
	
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    
    
    if (KingsNoSmoke) {
        // SMOOTH / Balanced / HDR / HD / NO SMOKE || NO FOG 🌫️
      if (strstr(*string,_enc("M_fog")) || strstr(*string,_enc("M_smoke_Lobby")) || strstr(*string,_enc("M_New_PublicMain_Int")) || strstr(*string,_enc("M_Song3CloudsFog")) || strstr(*string,_enc("M_Smoke")) || strstr(*string,_enc("M_NN_PublicMain_Int")) || strstr(*string,_enc("M_Main_Smoke_01_Int")) || strstr(*string,_enc("M_SmokeV2_Main_Int")) || strstr(*string,_enc("M_Main_Smoke_01_zx")) || strstr(*string,_enc("M_New_PublicMain_Int_02")) || strstr(*string,_enc("M_smoketrail_add")) || strstr(*string,_enc("M_Main_Smoke_Grenade_01_zx")) || strstr(*string,_enc("M_GlideSmoke")) || strstr(*string,_enc("M_Main_Smoke_Add_Int")) || strstr(*string,_enc("M_Main_Smoke_02")) || strstr(*string,_enc("M_Hologram_Particle")) || strstr(*string,_enc("M_particle_shan_01_1")) || strstr(*string,_enc("M_Main_Smoke_pixel")) || strstr(*string,_enc("M_GrenadeSmoke_Int")) || strstr(*string,_enc("M_BlackBodySmoke")) || strstr(*string,_enc("M_Main_Smoke_Grenade_01_q")) || strstr(*string,_enc("M_smoketrail_Int_Alpha")) || strstr(*string,_enc("M_Main_Smoke_01_HDR")) || strstr(*string,_enc("M_Main_Smoke_Grenade_01_q03")) || strstr(*string,_enc("M_NNN_PublicMain_Int")) || strstr(*string,_enc("M_smoketrail_Int")) || strstr(*string,_enc("M_LitSmoke_Int_03")) || strstr(*string,_enc("M_Particle_DYG_101")) || strstr(*string,_enc("M_smoketrail_Int_Add"))) {
      GLenum shaderTypes = 0;
      glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
      if (shaderTypes == GL_FRAGMENT_SHADER) {
          std::lock_guard<std::mutex> lock{playerShaderMutex};
          playerMaskShader = shader;
          *string = NOGRASS;
        }
    }
  }
  
    
    if (KingsNoGrass) {
        // SMOOTH / Balanced / HDR / HD / GRASS 🏞️🏜️🍀🍃☘️
      if (strstr(*string,_enc( "_BaseMat_Grass_EarlyZ")) || strstr(*string, _enc("_BaseMat_Grass_Billboard_OneSide")) || strstr(*string,_enc( "_BaseMat_Grass_SimpleWind")) || strstr(*string,_enc( "Master_Grass_EarlyZ")) || strstr(*string,_enc( "Master_Grass_Billboard")) || strstr(*string,_enc( "Lobby_Grass_mat_switch")) || strstr(*string,_enc("PhysicalMaterial_Grass")) || strstr(*string, _enc("Grass_D")) || strstr(*string, _enc("Livik_frame_128")) || strstr(*string, _enc("Livik_frame_256")) || strstr(*string,_enc("LandscapeGrass")) || strstr(*string,_enc("LayerMask_Grass")) || strstr(*string,_enc("C_wet_2nd_Grass")) || strstr(*string,_enc("Baltic_Grass06")) || strstr(*string,_enc("Baltic_GrassType01")) || strstr(*string,_enc( "Baltic_GrassType02")) || strstr(*string,_enc("Baltic_Grass06_FoliageType")) || strstr(*string,_enc("PhysicalMaterial_Grass")) || strstr(*string,_enc( "M_Grass_2x2")) || strstr(*string,_enc( "M_Grass")) || strstr(*string,_enc( "Grass")) || strstr(*string,_enc( "M_GrassFootStepInst01"))) {
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = NOGRASS;
        }
    }
    
    
    }
	if (KingsNoWater) {
        // SMOOTH / Balanced / HDR / HD / NO WATER 💦🌊
       if (strstr(*string,_enc( "M_NewOcean_LDR_Rough")) || strstr(*string, _enc("M_NewOcean_Refl")) || strstr(*string,_enc( "M_Ocean_Iteration")) || strstr(*string,_enc( "M_Ocean_New_V2_Low")) || strstr(*string,_enc( "M_Ocean_WaterForm")) || strstr(*string,_enc( "M_NewOcean_LDRNoFog")) || strstr(*string,_enc( "M_Ocean_2x2_LOD")) || strstr(*string,_enc( "M_Ocean_2x2")) || strstr(*string,_enc( "M_Waterfall_yh03")) || strstr(*string,_enc( "M_Ocean_New_V2")) || strstr(*string,_enc( "M_WaterCloth_Caustic")) || strstr(*string,_enc( "M_FallWater_xq_01")) || strstr(*string,_enc( "M_WaterWall_Ball")) || strstr(*string,_enc( "M_Water_Main")) || strstr(*string,_enc( "MM_FallWater_xq_01")) || strstr(*string,_enc( "_Ocean_Mat"))) {
      GLenum shaderTypes = 0;
      glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
      if (shaderTypes == GL_FRAGMENT_SHADER) {
          std::lock_guard<std::mutex> lock{playerShaderMutex};
          playerMaskShader = shader;
            *string = NOGRASS;
        }
    }
}

	

	
	/*
    
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        
//==================[ BUILDING 🛖🏠🏫]
/*
 if (KingsNoBuilding) {
   if (strstr(*string,_enc( "_BaseMat_Buildings_AO_LowLOD_Karakin")) || strstr(*string,_enc( "_BaseMat_Buildings_AO_LowLOD")) || strstr(*string, _enc("_BaseMat_Buildings_AO_RGB")) || strstr(*string,_enc( "_BaseMat_Buildings_AO_4x8")) || strstr(*string,_enc( "M_Building_2x2_New")) || strstr(*string,_enc( "M_Building_Metro2033")) || strstr(*string,_enc( "M_Building_Metro2033_Dynamic")) || strstr(*string,_enc( "M_Building_MultiLight")) || strstr(*string,_enc( "M_Building_Moss")) || strstr(*string,_enc( "M_Building_Flowmap"))) {
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = NOGRASS;
        }
    }
    }        */
        
    
/*    if (strstr(*string,_enc("Master_VH_IBL/TGPUSkinVertexFactoryfalse")) || strstr(*string,_enc("Master_VH_Base/TGPUSkinVertexFactoryfalse"))) {
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = newMaskRed;
        }
    }
    if (KingsNoGrass) {
        // SMOOTH / Balanced / HDR / HD / GRASS 🏞️🏜️🍀🍃☘️
      if (strstr(*string,_enc( "DihorOtok_GrassType03")) || strstr(*string, _enc("Forest_GrassType01")) || strstr(*string,_enc( "Grass_LayerInfo")) || strstr(*string, _enc("Grass_Tiling")) || strstr(*string,_enc( "LandscapeGrassType")) || strstr(*string, _enc("M_Crabgrass")) || strstr(*string,_enc( "M_Crabgrass2")) || strstr(*string, _enc("Lobby_Grass_mat_switch")) || strstr(*string,_enc( "MI_Landscape_ForestDesert_NewGrass")) || strstr(*string, _enc("bOverrideGrassTypes")) || strstr(*string,_enc( "GrassTypes"))|| strstr(*string, _enc("MinGrassWeightThreshold"))|| strstr(*string,_enc( "DihorOtok_Grass01")) || strstr(*string, _enc("M_Foliage_Grass_004_Billboard_Inst")) || strstr(*string,_enc( "M_Foliage_Grass_005_Billboard_Inst")) || strstr(*string, _enc("T_Survive_grass01")) || strstr(*string,_enc( "M_GrassBillboard_2x2")) || strstr(*string, _enc("M_Foliage_Grass_001_Billboard_Inst")) || strstr(*string,_enc( "Highting_SnowGrass_002_Billboard_Inst")) || strstr(*string, _enc("bHasLandscapeGrass")) || strstr(*string,_enc( "_BaseMat_Grass_EarlyZ")) || strstr(*string, _enc("_BaseMat_Grass_Billboard_OneSide")) || strstr(*string,_enc( "_BaseMat_Grass_SimpleWind")) || strstr(*string, _enc("Master_Grass_EarlyZ")) || strstr(*string,_enc( "Master_Grass_Billboard")) || strstr(*string, _enc("Lobby_Grass_mat_switch")) || strstr(*string,_enc( "PhysicalMaterial_Grass")) || strstr(*string, _enc("Grass_D")) || strstr(*string,_enc( "Baltic_Dogwood_Bush_wind")) || strstr(*string, _enc("Baltic_Dogwood_Bush_wind_Type")) || strstr(*string,_enc( "Livik_frame_128")) || strstr(*string, _enc("Livik_frame_256")) || strstr(*string,_enc( "LandscapeGrass")) || strstr(*string, _enc("LayerMask_Grass")) || strstr(*string,_enc( "C_wet_2nd_Grass")) || strstr(*string, _enc("Baltic_Grass06")) || strstr(*string,_enc( "Baltic_GrassType01")) || strstr(*string, _enc("Baltic_GrassType02")) || strstr(*string,_enc( "Baltic_Grass06_FoliageType")) || strstr(*string, _enc("PhysicalMaterial_Grass")) || strstr(*string,_enc( "M_Grass_2x2")) || strstr(*string, _enc("M_Grass")) || strstr(*string,_enc( "Grass")) || strstr(*string, _enc("M_GrassFootStepInst01")) ){
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = NOGRASS;
        }
    }*/
    
    
/*      // SMOOTH / Balanced / HD / HDR /  LANDSCAPE 🌆
     if (strstr(*string,_enc("PUBG_Baltic_Landscape")) || strstr(*string,_enc("MaskLayerA_FromAlpha")) || strstr(*string,_enc("NoGroudLayer")) || strstr(*string,_enc("BLEND_Opaque")) || strstr(*string,_enc("MSM_DefaultLit")) || strstr(*string,_enc("__LANDSCAPE_VISIBILITY__")) || strstr(*string,_enc("FLandscapeFlattenTeturesParameters")) || strstr(*string,_enc("FMobileDirectionalLightShaderParameters")) || strstr(*string,_enc("T_Default_BaseColor")) || strstr(*string,_enc("Default__Material")) || strstr(*string,_enc("LowLevelMaterialLOD")) || strstr(*string,_enc("Default__SkeletalMesh")) || strstr(*string,_enc("TutorialTPP_Mat")) || strstr(*string,_enc("TutorialTPP_PhysicsAsset")) || strstr(*string,_enc("ELandscapeCullingPrecision")) || strstr(*string,_enc("LandscapeMaterial")) || strstr(*string,_enc("LayerMask_Ground")) || strstr(*string,_enc("Baltic_x0_y0_Landscape")) || strstr(*string,_enc("Baltic_x0_y7_Landscape")) || strstr(*string,_enc("Baltic_x2_y0_Landscape")) || strstr(*string,_enc("Baltic_x2_y2_Landscape")) || strstr(*string,_enc("Baltic_x2_y5_Landscape")) || strstr(*string,_enc("Baltic_x2_y7_Landscape")) || strstr(*string,_enc("Baltic_x3_y0_Landscape")) || strstr(*string,_enc("Baltic_x3_y1_Landscape")) || strstr(*string,_enc("Baltic_x3_y5_Landscape")) || strstr(*string,_enc("Baltic_x3_y7_Landscape")) || strstr(*string,_enc("Baltic_x4_y0_Landscape")) || strstr(*string,_enc("Baltic_x4_y1_Landscape")) || strstr(*string,_enc("Baltic_x4_y2_Landscape")) || strstr(*string,_enc("Baltic_x4_y3_Landscape")) ||  strstr(*string,_enc( "Baltic_x4_y4_Landscape")) || strstr(*string,_enc("Baltic_x4_y5_Landscape")) || strstr(*string,_enc("Baltic_x4_y6_Landscape")) || strstr(*string,_enc("Baltic_x4_y7_Landscape")) || strstr(*string,_enc("Baltic_x5_y0_Landscape")) || strstr(*string,_enc("Baltic_x5_y1_Landscape")) || strstr(*string,_enc("Baltic_x5_y2_Landscape")) || strstr(*string,_enc("Baltic_x5_y3_Landscape")) || strstr(*string,_enc("Baltic_x5_y4_Landscape")) || strstr(*string,_enc("Baltic_x5_y5_Landscape")) || strstr(*string,_enc("Baltic_x5_y6_Landscape")) || strstr(*string,_enc("Baltic_x5_y7_Landscape")) || strstr(*string,_enc("Baltic_x6_y0_Landscape")) || strstr(*string,_enc("Baltic_x6_y6_Landscape")) || strstr(*string,_enc("Baltic_x7_y0_Landscape")) || strstr(*string,_enc("FLandscapeVTPS")) || strstr(*string,_enc("PUBG_Baltic_LandscapeLow")) || strstr(*string,_enc("Master_Baltic_Landscape")) || strstr(*string,_enc("Master_Baltic_Landscape_LOW")) || strstr(*string,_enc("Master_Baltic_Landscape_Hole_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_LOW_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_Sand_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_Low_FarmLand_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_Low_Sand_Inst")) || strstr(*string,_enc("Master_Baltic_Landscape_FarmLand_Inst")) || strstr(*string,_enc("M_Master_TPlanD_Landscape")) || strstr(*string,_enc("M_Master_TPlanD_Landscape_Hole")) || strstr(*string,_enc("M_Ground_Moss")) || strstr(*string,_enc("M_WSZLandscpae")) || strstr(*string,_enc("DihorOtok_Landscape_underwater")) || strstr(*string,_enc("MI_SD_Ground_Moss")) || strstr(*string,_enc("_Landscape_shooting_materials1"))) {
        GLenum shaderTypes = 0;
        glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
        if (shaderTypes == GL_FRAGMENT_SHADER) {
            std::lock_guard<std::mutex> lock{playerShaderMutex};
            playerMaskShader = shader;
            *string = newMaskSILVER;
        }
    }*/
    
    
/*         // SMOOTH / Balanced / HDR / HD / SCOPE⭕
      if (strstr(*string, _enc("Master_Opacity_Sight/TGPUSkinVertexFactorytrue")) || strstr(*string, _enc("Master_Opacity_Sight/TGPUSkinVertexFactoryfalse"))) {
      GLenum shaderTypes = 0;
      glGetShaderiv(shader, GL_SHADER_TYPE, reinterpret_cast<GLint *>(&shaderTypes));
      if (shaderTypes == GL_FRAGMENT_SHADER) {
          std::lock_guard<std::mutex> lock{playerShaderMutex};
          playerMaskShader = shader;
          *string = scopeRingCod;
        }
    }*/
    
        
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  
    return oglShaderSource(shader, count, string, length);
}

void (*oglAttachShader)(GLuint program, GLuint shader);
void _glAttachShader(GLuint program, GLuint shader) {
    std::unique_lock<std::mutex> ulock{playerShaderMutex};
    if (shader == playerVertexShader || shader == playerMaskShader) {
        ulock.unlock();
        {
            std::lock_guard<std::mutex> lock{playerProgramsMutex};
            playerPrograms.insert(program);
        }
    }
    return oglAttachShader(program, shader);
}


#define SLEEP_TIME 1000LL / 60LL

[[noreturn]] void *maps_thread(void *) {
    while (true) {
        auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        std::vector<sRegion> tmp;
        char line[512];
        FILE *f = fopen("/proc/self/maps", "r");
        if (f) {
            while (fgets(line, sizeof line, f)) {
                uintptr_t start, end;
                char tmpProt[16];
                if (sscanf(line, "%" PRIXPTR "-%" PRIXPTR " %16s %*s %*s %*s %*s", &start, &end, tmpProt) > 0) {
                    if (tmpProt[0] != 'r') {
                        tmp.push_back({start, end});
                    }
                }
            }
            fclose(f);
        }
        trapRegions = tmp;

        auto td = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - t1;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::max(std::min(0LL, SLEEP_TIME - td), SLEEP_TIME)));
    }
}

void *main_thread(void *) {
     g_UE4 = Tools::GetBaseAddress("libUE4.so");
    while (!g_UE4) {
        g_UE4 = Tools::GetBaseAddress("libUE4.so");
        sleep(1);
    } 

    while (!g_App) {
        g_App = *(android_app **) (g_UE4 + GNativeAndroidApp_Offset);
        sleep(1);
    }

    while (!g_App->onInputEvent)
        sleep(1);

    orig_onInputEvent = decltype(orig_onInputEvent)(g_App->onInputEvent);
    g_App->onInputEvent = onInputEvent;

    xhook_register(".*\\.so$", "eglSwapBuffers", (void*)_eglSwapBuffers, (void **)&orig_eglSwapBuffers);
    xhook_register(".*\\.so$", "glDrawElements", (void*)_glDrawElements, (void **)&oglDrawElements);
    xhook_register(".*\\.so$", "glShaderSource", (void*)_glShaderSource, (void **)&oglShaderSource);
    xhook_register(".*\\.so$", "glAttachShader", (void*)_glAttachShader, (void **)&oglAttachShader);
    xhook_refresh(0);

    pthread_t t;
    pthread_create(&t, 0, maps_thread, 0);
    return 0;
}

__attribute__((constructor)) void _init() {
    pthread_t t;
    pthread_create(&t, 0, main_thread, 0);
}
bool LOCALxHACKS = true;
extern "C" {
JNIEXPORT void JNICALL Java_androidx_core_MultDex_LOCALxHACKS(JNIEnv *, jobject, jint code,
    jboolean jboolean1) {
        switch ((int)code) {
            case 390:
            LOCALxHACKS = jboolean1;
        }
    }
}
