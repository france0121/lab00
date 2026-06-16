 // Lab13_29.cpp
// Лабораторная работа №13 - Вариант 29
// Проверить, начинается ли текст с заглавной буквы.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

HWND hMainWnd;
HWND hEditText;
HWND hLblResult;
HWND hLblStatus;
HWND hBtnCheck, hBtnClear, hBtnExit;

// ====================================================================
// ЛОГИКА
// ====================================================================

bool StartsWithUppercase(const std::string& text) {
    if (text.empty()) return false;
    char first = text[0];
    return (first >= 'A' && first <= 'Z');
}

std::string GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthA(hEdit);
    if (len == 0) return "";
    char* buffer = new char[len + 1];
    GetWindowTextA(hEdit, buffer, len + 1);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

void UpdateStatus(const std::string& msg, bool isError = false) {
    SetWindowTextA(hLblStatus, msg.c_str());
}

// ====================================================================
// ОБРАБОТЧИКИ
// ====================================================================

void OnCheck() {
    std::string text = GetEditText(hEditText);
    if (text.empty()) {
        MessageBoxA(hMainWnd, "Введите текст!", "Ошибка", MB_OK | MB_ICONERROR);
        SetWindowTextA(hLblResult, "Текст не введен");
        UpdateStatus("Ошибка: текст пуст", true);
        SetFocus(hEditText);
        return;
    }

    bool result = StartsWithUppercase(text);
    if (result) {
        SetWindowTextA(hLblResult, "Текст начинается с заглавной буквы!");
        UpdateStatus("Текст начинается с заглавной буквы");
    } else {
        char buffer[256];
        sprintf_s(buffer, "Текст начинается со строчной буквы ('%c')", text[0]);
        SetWindowTextA(hLblResult, buffer);
        UpdateStatus("Текст начинается со строчной буквы", true);
    }
}

void OnClear() {
    SetWindowTextA(hEditText, "");
    SetWindowTextA(hLblResult, "Нажмите 'Проверить'");
    UpdateStatus("Введите текст и нажмите 'Проверить'");
    SetFocus(hEditText);
}

void OnExit() {
    PostQuitMessage(0);
}

// ====================================================================
// ОКНО ПРОЦЕДУРЫ
// ====================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hMainWnd = hWnd;
        HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        // Текстовое поле
        HWND hLblText = CreateWindowA("STATIC", "Введите текст:", WS_CHILD | WS_VISIBLE,
            20, 20, 150, 25, hWnd, NULL, NULL, NULL);
        hEditText = CreateWindowA("EDIT", "Пример текста для проверки",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            20, 50, 500, 100, hWnd, NULL, NULL, NULL);
        SendMessage(hEditText, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Результат
        HWND hLblRes = CreateWindowA("STATIC", "Результат:", WS_CHILD | WS_VISIBLE,
            20, 170, 150, 25, hWnd, NULL, NULL, NULL);
        hLblResult = CreateWindowA("STATIC", "Нажмите 'Проверить'",
            WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
            20, 200, 500, 60, hWnd, NULL, NULL, NULL);
        SendMessage(hLblResult, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Статус
        hLblStatus = CreateWindowA("STATIC", "Введите текст и нажмите 'Проверить'",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 280, 500, 30, hWnd, NULL, NULL, NULL);

        // Кнопки
        hBtnCheck = CreateWindowA("BUTTON", "Проверить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 330, 130, 35, hWnd, (HMENU)1, NULL, NULL);
        hBtnClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            170, 330, 130, 35, hWnd, (HMENU)2, NULL, NULL);
        hBtnExit = CreateWindowA("BUTTON", "Выход", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            320, 330, 130, 35, hWnd, (HMENU)3, NULL, NULL);

        SendMessage(hLblText, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblRes, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == 1) OnCheck();
        else if (id == 2) OnClear();
        else if (id == 3) OnExit();
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "Lab13Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Ошибка регистрации класса!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    hMainWnd = CreateWindowA("Lab13Class", "ЛР №13 - Вариант 29",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 560, 430,
        NULL, NULL, hInstance, NULL);

    if (!hMainWnd) {
        MessageBoxA(NULL, "Ошибка создания окна!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}