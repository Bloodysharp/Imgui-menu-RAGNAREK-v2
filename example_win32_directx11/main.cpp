#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "imgui_edited.hpp"
#include "imgui_freetype.h"

#include <d3d11.h>
#include <tchar.h>

#include "font.h"
#include "image.h"

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void TextColoredPos(const char* label, ImU32 col, const ImVec2& pos)
{
    ImGui::SetCursorPos(ImGui::GetCursorPos() + pos);
    ImGui::TextColored(ImColor(col), label);
}

namespace image
{
    inline ID3D11ShaderResourceView* background_preview = nullptr;
    inline ID3D11ShaderResourceView* preview_model = nullptr;
    inline ID3D11ShaderResourceView* logo = nullptr;
}

namespace font
{
    inline ImFont* icomoon = nullptr;
    inline ImFont* icomoon_tabs = nullptr;
    inline ImFont* icomoon_widget = nullptr;

    inline ImFont* inter_child = nullptr;
    inline ImFont* inter_element = nullptr;
}

namespace esp_preview
{
    bool money = true;
    bool nickname = true;
    bool weapon = true;
    bool zoom = true;

    bool c4 = true;
    bool HP_line = true;
    bool hit = true;
    bool box = true;
    bool bomb = true;

    static float box_color[4] = { 37 / 255.f, 37 / 255.f, 47 / 255.f, 1.f };
    static float nick_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float money_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float zoom_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float c4_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float bomb_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float hp_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float hp_line_color[4] = { 112 / 255.f, 109 / 255.f, 214 / 255.f, 1.f };
    static float weapon_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
    static float hit_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };

    int hp = 85;
}

bool info_bar = true;

const char* cheat_name = "RAGNAREK";
const char* game_status = "Counter-Strike: 2";
const char* developer = "Bloodysharp";

const char* ping = "80ms";
const char* world_time = "4:30am";

DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayHex;
static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

int main(int, char**)
{

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, 0, 0, 1920, 1080, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;     

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;

    font::inter_element = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::inter_child = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    font::icomoon = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 19.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icomoon_tabs = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 22.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icomoon_widget = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
    if (image::background_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background, sizeof(background), &info, pump, &image::background_preview, 0);
    if (image::preview_model == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, preview_model, sizeof(preview_model), &info, pump, &image::preview_model, 0);
    if (image::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &image::logo, 0);

  //  ImGui::StyleColorsLight();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(26, 27, 31);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGuiStyle* style = &ImGui::GetStyle();

            static float color[4] = { 112 / 255.f, 109 / 255.f, 214 / 255.f, 1.f };
            c::accent_color = { color[0], color[1], color[2], 1.f };

            static float background[4] = { 21 / 255.f, 21 / 255.f, 21 / 255.f, 1.f };
            c::bg::background = { background[0], background[1], background[2], background[3] };

            static float border[4] = { 23 / 255.f, 24 / 255.f, 25 / 255.f, 1.f };
            c::bg::border = { border[0], border[1], border[2], border[3] };

            static float child[4] = { 23 / 255.f, 24 / 255.f, 25 / 255.f, 1.f };
            c::child::background = { child[0], child[1], child[2], child[3] };

            static float widget[4] = { 28 / 255.f, 28 / 255.f, 35 / 255.f, 1.f };
            c::widget::background = { widget[0], widget[1], widget[2], widget[3] };

            static float selectable[4] = { 37 / 255.f, 37 / 255.f, 47 / 255.f, 1.f };
            c::widget::selectable = { selectable[0], selectable[1], selectable[2], selectable[3] };

            static float popup[4] = { 21 / 255.f, 21 / 255.f, 22 / 255.f, 1.f };
            c::widget::popup = { popup[0], popup[1], popup[2], popup[3] };

            static float text_active[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
            c::text::text_active = { text_active[0], text_active[1], text_active[2], text_active[3] };

            static float text_hovered[4] = { 89 / 255.f, 95 / 255.f, 105 / 255.f, 1.f };
            c::text::text_hov = { text_hovered[0], text_hovered[1], text_hovered[2], text_hovered[3] };

            static float text_default[4] = { 50 / 255.f, 54 / 255.f, 59 / 255.f, 1.f };
            c::text::text = { text_default[0], text_default[1], text_default[2], text_default[3] };

            style->WindowPadding = ImVec2(0, 0);
            style->ItemSpacing = ImVec2(20, 0);
            style->WindowBorderSize = 0;
            style->ScrollbarSize = 9.f;

           // ImGui::GetBackgroundDrawList()->AddImage(image::background_preview, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));

            ImGui::SetNextWindowSize(c::bg::size);

            ImGui::Begin("RAGNAREK", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
            {
                const ImVec2& pos = ImGui::GetWindowPos();
                const ImVec2& region = ImGui::GetContentRegionMax();
                const ImVec2& spacing = style->ItemSpacing;

                ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(c::bg::size), ImGui::GetColorU32(c::bg::background), c::bg::rounding + 1);
                ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(100, c::bg::size.y), ImGui::GetColorU32(c::bg::border), c::bg::rounding, ImDrawFlags_RoundCornersLeft);

                ImGui::GetBackgroundDrawList()->AddImage(image::logo, pos + (ImVec2(100, 100) - ImVec2(38, 43)) / 2, pos + (ImVec2(100, 100) + ImVec2(38, 43)) / 2, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent_color));

                ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(0, 100), pos + ImVec2(100, 100), ImGui::GetColorU32(c::widget::background), 1.f);

                static int page = 0;

                ImGui::SetCursorPos(ImVec2((100 - 47) / 2, 100 + (47 / 2) ));
                ImGui::BeginGroup();
                {
                    if (edited::Tab(0 == page, 1, "c", ImVec2(47, 47))) page = 0;

                    if (edited::Tab(1 == page, 2, "a", ImVec2(47, 47))) page = 1;

                    if (edited::Tab(2 == page, 3, "b", ImVec2(47, 47))) page = 2;

                    if (edited::Tab(3 == page, 4, "o", ImVec2(47, 47))) page = 3;

                    if (edited::Tab(4 == page, 5, "v", ImVec2(47, 47))) page = 4;

                    if (edited::Tab(5 == page, 6, "f", ImVec2(47, 47))) page = 5;

                    if (edited::Tab(6 == page, 7, "e", ImVec2(47, 47))) page = 6;
                }
                ImGui::EndGroup();

                ImGui::SetCursorPos(ImVec2(100 + spacing.x, 0));

                tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (page == active_tab ? 1.f : -1.f)), 0.f, 1.f);
                if (tab_alpha == 0.f && tab_add == 0.f) active_tab = page;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);

                ImGui::BeginChild("CONTAINER", ImVec2(c::bg::size) - ImVec2(100 + spacing.x, 0));
                {
                    if (active_tab == 0)
                    {
                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("Weapons", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static int select = 0;
                                const char* items[3]{ "AWP", "AK47", "M4A1" };
                                edited::Combo("Select Weapon", &select, items, IM_ARRAYSIZE(items), 3);

                                static bool enable_cfg = true;
                                edited::Checkbox("Enable Config", &enable_cfg);
                            }
                            edited::EndChild();

                            edited::BeginChild("Additions", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static int select = 0;
                                const char* items[3]{ "Low", "Normal", "High" };
                                edited::Combo("History", &select, items, IM_ARRAYSIZE(items), 3);

                                static bool delay_shot = true;
                                edited::Checkbox("Delay Shot", &delay_shot);

                                static bool duck_peek = false;
                                edited::Checkbox("Duck Peek Assist", &duck_peek);

                                static bool peek_assist = false;
                                edited::CheckboxClicked("Quick Peek Assist", &peek_assist);

                                static bool speed_fire = true;
                                edited::Checkbox("Speed Up Fire Rate", &speed_fire);

                                static bool Magic_bullet = false;
                                edited::CheckboxClicked("Magic Bullet", &Magic_bullet);
                            }
                            edited::EndChild();

                            edited::BeginChild("Anti Aim", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static bool Enabled = true;
                                edited::Checkbox("Enabled", &Enabled);

                                static int select0 = 0;
                                const char* items0[2]{ "Disabled", "Enabled" };
                                edited::ComboClicked("Pitch", &select0, items0, IM_ARRAYSIZE(items0), 2);

                                static int select1 = 0;
                                const char* items1[2]{ "Disabled", "Enabled" };
                                edited::ComboClicked("Yaw", &select1, items1, IM_ARRAYSIZE(items1), 2);

                                static bool slow_walk = false;
                                edited::Checkbox("Slow Walk", &slow_walk);

                                static bool freestanding = false;
                                static float color[4] = { 124 / 255.f, 103 / 255.f, 255 / 255.f, 0.5f };
                                edited::CheckboxPicker("Freestanding", &freestanding, color, picker_flags);
                            }
                            edited::EndChild();
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("General", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static bool enabled = true;
                                edited::CheckboxClicked("Enabled", &enabled);

                                static bool silent = false;
                                edited::CheckboxClicked("Silent Aimbot", &silent);

                                static bool auto_fire = true;
                                edited::Checkbox("Automatic Fire", &auto_fire);

                                static bool penetrate_walls = true;
                                edited::Checkbox("Penetrate Walls", &penetrate_walls);

                                static int field = 90;
                                edited::SliderInt("Field Of View", &field, -180, 180);

                                static float r0 = -100, r1 = 100;
                                edited::RangeSliderFloat("Hit Chance", &r0, &r1, -100, 100, "%.1f, %.1f");

                                static float r2 = 0, r3 = 10;
                                edited::RangeSliderFloat("Damage", &r2, &r3, 0, 10, "%.1f, %.1f");
                            }
                            edited::EndChild();

                            edited::BeginChild("Selection", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static int select0 = 0;
                                const char* items0[2]{ "Hit Chance", "Default" };
                                edited::Combo("Target", &select0, items0, IM_ARRAYSIZE(items0), 2);

                                static bool multi_num1[5] = { false, true, true, true, false };
                                const char* multi_items1[5] = { "Head", "Chest", "Stomatch", "Body", "Legs" };
                                edited::MultiComboClicked("Hitboxes", multi_num1, multi_items1, 5);

                                static int select1 = 0;
                                const char* items1[2]{ "Select", "Defect" };
                                edited::ComboClicked("Multipint", &select1, items1, IM_ARRAYSIZE(items1), 2);

                                static bool auto_stop = true;
                                edited::CheckboxClicked("Auto Stop", &auto_stop);

                                static bool auto_scope = true;
                                edited::Checkbox("Auto Scope", &auto_scope);
                            }
                            edited::EndChild();

                            edited::BeginChild("Extrended", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static int select1 = 0;
                                const char* items1[2]{ "Automatic", "Yourself" };
                                edited::Combo("Mode", &select1, items1, IM_ARRAYSIZE(items1), 2);

                                static int key = 0;
                                edited::Keybind("Click on me to bind", &key);

                            }
                            edited::EndChild();
                        }
                        ImGui::EndGroup();

                    }
                    else if (active_tab == 2)
                    {
                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("Players", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static bool enabled = true;
                                edited::Checkbox("Enabled", &enabled);

                                static bool teammates = false;
                                edited::Checkbox("Teammates", &teammates);

                                static bool behind = false;
                                edited::Checkbox("Behind Walls", &behind);

                                static bool tracers = true;
                                edited::CheckboxClicked("Bullet Tracers", &tracers);

                                static bool offscreen = false;
                                edited::CheckboxClicked("Offscreen ESP", &offscreen);

                                static bool sounds = false;
                                static float color_sound[4] = { 124 / 255.f, 103 / 255.f, 255 / 255.f, 0.5f };
                                edited::CheckboxPicker("Sounds", &sounds, color_sound, picker_flags);

                                static bool radar = false;
                                static float color_radar1[4] = { 124 / 255.f, 103 / 255.f, 255 / 255.f, 1.0f };
                                static float color_radar2[4] = { 124 / 255.f, 103 / 255.f, 255 / 255.f, 0.5f };
                                edited::CheckboxDoublePicker("Radar", &radar, color_radar1, color_radar2, picker_flags);

                                static char input[45] = { "" };

                                ImGui::InputTextEx("v", "Enter your text here", input, 45, ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 35), NULL);

                            }
                            edited::EndChild();

                            edited::BeginChild("Models", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static int enemies = 0;
                                edited::Keybind("Enemies", &enemies);

                                static int teammates = 0;
                                edited::Keybind("Teammates", &teammates);

                                static int players = 0;
                                edited::Keybind("Local Player", &players);

                                static int ragdolls = 0;
                                edited::Keybind("Ragdolls", &ragdolls);

                            }
                            edited::EndChild();

                            edited::BeginChild("World", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                static bool bomb = true;
                                edited::CheckboxClicked("Bomb", &bomb);

                                static bool weapons = false;
                                edited::CheckboxClicked("Weapons", &weapons);

                                static float r0 = -9000, r1 = 9000;
                                edited::RangeSliderFloat("The Radius Of Vision", &r0, &r1, -10000, 10000, "%.1f, %.1f");
                            }
                            edited::EndChild();
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("ESP PREVIEW", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                edited::esp_preview(image::preview_model,
                                &esp_preview::nickname, esp_preview::nick_color,
                                &esp_preview::weapon, esp_preview::weapon_color,
                                &esp_preview::hp, esp_preview::hp_color,
                                &esp_preview::zoom, esp_preview::zoom_color,
                                &esp_preview::bomb, esp_preview::bomb_color,
                                &esp_preview::c4, esp_preview::c4_color,
                                &esp_preview::money, esp_preview::money_color,
                                &esp_preview::hit, esp_preview::hit_color,
                                &esp_preview::box, esp_preview::box_color,
                                &esp_preview::HP_line, esp_preview::hp_line_color);
                            }
                            edited::EndChild();

                            edited::BeginChild("ESP MANAGE ELEMENTS", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {

                                edited::CheckboxPicker("Show Nickname", &esp_preview::nickname, esp_preview::nick_color, picker_flags);

                                edited::CheckboxPicker("Show Zoomed", &esp_preview::zoom, esp_preview::zoom_color, picker_flags);

                                edited::CheckboxPicker("Show Weapon", &esp_preview::weapon, esp_preview::weapon_color, picker_flags);

                                edited::CheckboxPicker("Show Money", &esp_preview::money, esp_preview::money_color, picker_flags);

                                edited::CheckboxPicker("Show Bomb", &esp_preview::bomb, esp_preview::bomb_color, picker_flags);

                                edited::CheckboxPicker("Show Box", &esp_preview::box, esp_preview::box_color, picker_flags);

                                edited::CheckboxPicker("Show Hit", &esp_preview::hit, esp_preview::hit_color, picker_flags);

                                edited::CheckboxDoublePicker("Show HP", &esp_preview::HP_line, esp_preview::hp_color, esp_preview::hp_line_color, picker_flags);

                                edited::CheckboxPicker("Show C4", &esp_preview::c4, esp_preview::c4_color, picker_flags);

                            }
                            edited::EndChild();
                        }
                        ImGui::EndGroup();
                    }
                    else if (active_tab == 6)
                    {
                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("GUI", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                edited::ColorEdit4("Accent Color", color, picker_flags | ImGuiColorEditFlags_NoAlpha);
                            }
                            edited::EndChild();

                            edited::BeginChild("Styles", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                edited::ColorEdit4("Background", background, picker_flags);

                                edited::ColorEdit4("Border", border, picker_flags);

                                edited::ColorEdit4("Child", child, picker_flags);

                            }
                            edited::EndChild();

                            edited::BeginChild("Others", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                edited::ColorEdit4("Color Element's", widget, picker_flags);

                                edited::ColorEdit4("Color Selectable", selectable, picker_flags);

                                edited::ColorEdit4("Color Popup's", popup, picker_flags);
                            }
                            edited::EndChild();
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();

                        ImGui::BeginGroup();
                        {
                            edited::BeginChild("Miscellaneous", ImVec2(c::bg::size.x - (100 + spacing.x * 3), 0) / 2);
                            {
                                edited::ColorEdit4("Text Active", text_active, picker_flags);

                                edited::ColorEdit4("Text Hovered", text_hovered, picker_flags);

                                edited::ColorEdit4("Text Default", text_default, picker_flags);
                            }
                            edited::EndChild();

                        }
                        ImGui::EndGroup();
                    }

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing.x);
                }
                ImGui::EndChild();

                ImGui::PopStyleVar();

                static float ibar_size = ImGui::CalcTextSize(cheat_name).x + ImGui::CalcTextSize("|").x + ImGui::CalcTextSize(developer).x + ImGui::CalcTextSize("|").x + ImGui::CalcTextSize(ping).x + ImGui::CalcTextSize("|").x + ImGui::CalcTextSize(world_time).x + (style->ItemSpacing.x * 8);
                static float position = (GetSystemMetrics(SM_CXSCREEN) - ibar_size) / 2;
                position = ImLerp(position, info_bar ? position : GetSystemMetrics(SM_CXSCREEN), ImGui::GetIO().DeltaTime * 8.f);

                if (position <= (GetSystemMetrics(SM_CXSCREEN) - 2)) {

                    ImGui::SetNextWindowPos(ImVec2(position, 5));
                    ImGui::SetNextWindowSize(ImVec2(ibar_size, 45));

                    ImGui::Begin("info-bar", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
                    {
                        const ImVec2& pos = ImGui::GetWindowPos(), spacing = style->ItemSpacing, region = ImGui::GetContentRegionMax();

                        ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(ibar_size, 45), ImGui::GetColorU32(c::bg::background), c::child::rounding);
                        ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(0, 10), pos + ImVec2(4, 35), ImGui::GetColorU32(c::accent_color), c::bg::rounding, ImDrawFlags_RoundCornersRight);
                        ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(region.x - 4, 10), pos + ImVec2(region.x, 35), ImGui::GetColorU32(c::accent_color), c::bg::rounding, ImDrawFlags_RoundCornersLeft);

                        const char* info_set[4] = { cheat_name, developer, ping, world_time };
                        static int info_bar_size = 0;

                        ImGui::SetCursorPos(ImVec2(spacing.x, (45 - ImGui::CalcTextSize(developer).y) / 2));
                        ImGui::BeginGroup();
                        {

                            for (int i = 0; i < sizeof(info_set) / sizeof(info_set[0]); i++) {
                                ImGui::TextColored(i < 1 ? ImColor(ImGui::GetColorU32(c::accent_color)) : ImColor(ImGui::GetColorU32(c::text::text)), info_set[i]);
                                ImGui::SameLine();

                                if (i < 3) {
                                    ImGui::TextColored(ImColor(ImGui::GetColorU32(c::text::text)), "|");
                                    ImGui::SameLine();
                                }
                            }
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::End();
                }

            }
            ImGui::End();
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
