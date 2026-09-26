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

// тайминги (мс)
const int PHASE_ERRORS_MS   = 13000;   // сколько идут ошибки
const int PHASE_BSOD_MS     = 2500;    // сколько висит BSOD
const int PHASE_NAPOLEON_MS = 15000;   // сколько летают клоны

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

// ---------- фаза ошибок ----------
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

// ---------- фаза BSOD ----------
void DrawBsod(Graphics& g, int W, int H) {
    // синий фон классического BSOD (0x0078D7)
    SolidBrush blue(Color(255, 0, 120, 215));
    g.FillRectangle(&blue, 0, 0, W, H);

    FontFamily ff(L"Segoe UI");
    Font fontBig(&ff, 96.0f, FontStyleRegular, UnitPixel);
    Font fontMed(&ff, 32.0f, FontStyleRegular, UnitPixel);
    Font fontSmall(&ff, 20.0f, FontStyleRegular, UnitPixel);
    SolidBrush white(Color(255, 255, 255, 255));

    float margin = W * 0.12f;
    float y = H * 0.28f;

    g.DrawString(L":(", -1, &fontBig, PointF(margin, y), &white);
    y += 130.0f;
    g.DrawString(L"Votre PC a rencontré un problème et doit redémarrer.",
                 -1, &fontMed, PointF(margin, y), &white);
    y += 70.0f;
    g.DrawString(L"Napoléon a piraté votre PC.", -1, &fontMed,
                 PointF(margin, y), &white);
    y += 70.0f;
    g.DrawString(L"Code d'arrêt: NAPOLEON_BONAPARTE_FAILURE", -1, &fontSmall,
                 PointF(margin, y), &white);
    y += 40.0f;
    g.DrawString(L"0x00001815 (0xB0NAP4RTE, 0xW4TERL00, 0x00000000, 0xFFFFFFFF)",
                 -1, &fontSmall, PointF(margin, y), &white);
    y += 60.0f;
    g.DrawString(L"0% terminé", -1, &fontSmall, PointF(margin, y), &white);
}

// ---------- фаза Наполеона ----------
void DrawNapoleonPhase(Graphics& g, int W, int H) {
    if ((int)g_clones.size() < 120) {
        for (int i = 0; i < 3; i++) SpawnClone(W, H);
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

    if (elapsedMs < PHASE_ERRORS_MS) {
        DrawErrors(g, W, H);
    } else if (elapsedMs < PHASE_ERRORS_MS + PHASE_BSOD_MS) {
        DrawBsod(g, W, H);
    } else if (elapsedMs < PHASE_ERRORS_MS + PHASE_BSOD_MS + PHASE_NAPOLEON_MS) {
        DrawNapoleonPhase(g, W, H);
    } else {
        // в самом конце цикла — просто чёрный (промежуток перед рестартом)
    }
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
    PlaySoundW(L"Bonaparte.wav", nullptr, SND_FILENAME | SND_ASYNC | SND_LOOP);

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

    const int CYCLE_MS = PHASE_ERRORS_MS + PHASE_BSOD_MS + PHASE_NAPOLEON_MS;
    auto cycleStart = std::chrono::steady_clock::now();

    MSG msg;
    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto done;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto now = std::chrono::steady_clock::now();
        int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - cycleStart).count();

        // рестарт цикла
        if (elapsed >= CYCLE_MS) {
            g_clones.clear();
            cycleStart = std::chrono::steady_clock::now();
            elapsed = 0;
        }

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
