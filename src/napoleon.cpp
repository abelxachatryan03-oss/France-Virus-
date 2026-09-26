// language: C++, file: src/napoleon.cpp
// build MinGW: g++ src/napoleon.cpp -o France.exe -lgdiplus -lwinmm -mwindows
// build MSVC:  cl src/napoleon.cpp /EHsc /O2 /link gdiplus.lib winmm.lib user32.lib gdi32.lib /OUT:France.exe
// put Bonaparte.wav and Napoleon.png next to France.exe

#include <windows.h>
#include <gdiplus.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <random>
#include <chrono>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

using namespace Gdiplus;

HWND g_hwnd = nullptr;
Image* g_napoleon = nullptr;
std::mt19937 g_rng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());

std::vector<std::wstring> g_errorTexts = {
    L"Napoléon a piraté votre PC",
    L"ERREUR: Napoléon a piraté votre PC",
    L"Napoléon a piraté votre PC !!!",
    L"ALERTE: Napoléon a piraté votre PC",
    L"Napoléon contrôle votre ordinateur",
    L"Votre PC appartient à Napoléon",
    L"ERREUR SYSTÈME: Napoléon est partout",
    L"Napoléon a pris le contrôle",
    L"VIVE L'EMPEREUR — votre PC est à lui",
    L"Napoléon a piraté votre PC",
    L"ERREUR FATALE: Napoléon a piraté votre PC",
    L"Votre disque dur est sous contrôle de Napoléon",
    L"Napoléon lit vos fichiers",
    L"Napoléon a piraté votre PC",
    L"SYSTÈME COMPROMIS PAR NAPOLÉON",
    L"Napoléon regarde votre webcam",
    L"ERREUR 1815: Waterloo inversé",
    L"Napoléon a piraté votre PC",
    L"Votre souris obéit à Napoléon",
    L"N'essayez pas de fuir — Napoléon est partout"
};

struct Clone {
    float x, y;
    int size;
    float vx, vy;
};

std::vector<Clone> g_clones;

ULONG_PTR g_gdiToken = 0;

const int NAPOLEON_AT_MS = 13000; // с этой секунды появляется Наполеон

void InitGdiPlus() {
    GdiplusStartupInput gsi;
    GdiplusStartup(&g_gdiToken, &gsi, nullptr);
}

void ShutdownGdiPlus() {
    GdiplusShutdown(g_gdiToken);
}

Image* LoadPng(const wchar_t* path) {
    Image* img = new Image(path);
    if (img->GetLastStatus() != Ok) {
        delete img;
        return nullptr;
    }
    return img;
}

DWORD GetWavLengthMs(const wchar_t* path) {
    MCI_OPEN_PARMS mciOpen = {};
    mciOpen.lpstrDeviceType = L"waveaudio";
    mciOpen.lpstrElementName = path;
    if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&mciOpen) != 0)
        return 0;
    DWORD deviceId = mciOpen.wDeviceID;

    MCI_STATUS_PARMS mciStatus = {};
    mciStatus.dwItem = MCI_STATUS_LENGTH;
    mciSendCommand(deviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

    mciSendCommand(deviceId, MCI_CLOSE, 0, 0);
    return (DWORD)mciStatus.dwReturn;
}

void SpawnClone(int screenW, int screenH) {
    std::uniform_real_distribution<float> xd(0, (float)screenW);
    std::uniform_real_distribution<float> yd(0, (float)screenH);
    std::uniform_int_distribution<int> sd(40, 220);
    std::uniform_real_distribution<float> vd(-2.0f, 2.0f);

    Clone c;
    c.x = xd(g_rng);
    c.y = yd(g_rng);
    c.size = sd(g_rng);
    c.vx = vd(g_rng);
    c.vy = vd(g_rng);
    g_clones.push_back(c);
}

void DrawErrors(Graphics& g, int W, int H) {
    std::uniform_int_distribution<int> xd(0, W - 500);
    std::uniform_int_distribution<int> yd(0, H - 60);
    std::uniform_int_distribution<int> fd(14, 32);
    std::uniform_int_distribution<int> cd(200, 255);
    std::uniform_int_distribution<int> count(10, 22);

    FontFamily ff(L"Consolas");
    int n = count(g_rng);
    for (int i = 0; i < n; i++) {
        int idx = std::uniform_int_distribution<int>(0, (int)g_errorTexts.size() - 1)(g_rng);
        int fs = fd(g_rng);
        Font font(&ff, (REAL)fs, FontStyleRegular, UnitPixel);
        SolidBrush brush(Color(255, cd(g_rng), 30, 30));
        g.DrawString(g_errorTexts[idx].c_str(), -1, &font,
                     PointF((REAL)xd(g_rng), (REAL)yd(g_rng)), &brush);
    }
}

void DrawNapoleon(Graphics& g, int W, int H) {
    if ((int)g_clones.size() < 80) {
        for (int i = 0; i < 2; i++) SpawnClone(W, H);
    }

    for (auto& c : g_clones) {
        c.x += c.vx;
        c.y += c.vy;
        if (c.x < -c.size || c.x > W) c.vx = -c.vx;
        if (c.y < -c.size || c.y > H) c.vy = -c.vy;
    }

    for (auto& c : g_clones) {
        if (g_napoleon) {
            g.DrawImage(g_napoleon, (INT)c.x, (INT)c.y, c.size, c.size);
        }
    }

    if (g_napoleon) {
        int bigSize = min(W, H) / 3;
        g.DrawImage(g_napoleon, (W - bigSize) / 2, (H - bigSize) / 2, bigSize, bigSize);
    }
}

void DrawScene(HDC hdc, int W, int H, int elapsedMs) {
    Graphics g(hdc);

    SolidBrush black(Color(255, 0, 0, 0));
    g.FillRectangle(&black, 0, 0, W, H);

    // с 13-й секунды — Наполеон под ошибками
    if (elapsedMs >= NAPOLEON_AT_MS) {
        DrawNapoleon(g, W, H);
    }

    // ошибки всегда поверх
    DrawErrors(g, W, H);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    DWORD wavMs = GetWavLengthMs(L"Bonaparte.wav");
    if (wavMs == 0) wavMs = 30000;

    PlaySoundW(L"Bonaparte.wav", nullptr, SND_FILENAME | SND_ASYNC);

    InitGdiPlus();
    g_napoleon = LoadPng(L"Napoleon.png");

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"NapoleonPrank";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    int W = GetSystemMetrics(SM_CXSCREEN);
    int H = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"NapoleonPrank", L"SYSTEM ERROR",
        WS_POPUP,
        0, 0, W, H,
        nullptr, nullptr, hInst, nullptr
    );
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    auto start = std::chrono::steady_clock::now();
    MSG msg;

    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto done;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto now = std::chrono::steady_clock::now();
        int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if ((DWORD)elapsed >= wavMs) break;

        HDC hdc = GetDC(g_hwnd);
        DrawScene(hdc, W, H, elapsed);
        ReleaseDC(g_hwnd, hdc);

        Sleep(30);
    }

done:
    PlaySoundW(nullptr, nullptr, 0);
    if (g_napoleon) delete g_napoleon;
    ShutdownGdiPlus();
    return 0;
}
