// Lab11_29.cpp
// Лабораторная работа №11 - Вариант 29
// Чистый Win32 API, без C++/CLI, без pch.h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

// ====================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ====================================================================
HWND hMainWnd;
HWND hEditArray, hEditK, hEditM;
HWND hListOriginal, hListResult;
HWND hBtnSolve, hBtnClear, hBtnExit;
HWND hLblStatus;

std::vector<int> g_originalArray;
std::vector<int> g_primeArray;

// ====================================================================
// ЛОГИЧЕСКИЕ ФУНКЦИИ
// ====================================================================

bool IsPrime(int n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

bool AllInRangeArePrime(const std::vector<int>& arr, int k, int m) {
    if (k < 0 || m >= (int)arr.size() || k > m) return false;
    for (int i = k; i <= m; ++i) {
        if (!IsPrime(arr[i])) return false;
    }
    return true;
}

void ParseInput(const std::string& text, std::vector<int>& arr) {
    arr.clear();
    std::istringstream iss(text);
    int val;
    while (iss >> val) {
        arr.push_back(val);
    }
}

// ====================================================================
// ФУНКЦИИ РАБОТЫ С LISTBOX
// ====================================================================

void ClearListBox(HWND hList) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
}

void AddToListBox(HWND hList, const std::string& text) {
    SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
}

void DisplayArray(const std::vector<int>& arr, HWND hList) {
    ClearListBox(hList);
    if (arr.empty()) {
        AddToListBox(hList, "Пусто");
        return;
    }
    for (size_t i = 0; i < arr.size(); ++i) {
        char buffer[64];
        sprintf_s(buffer, "[%3zu] = %6d", i, arr[i]);
        AddToListBox(hList, buffer);
    }
}

void UpdateStatus(const std::string& msg, bool isError = false) {
    SetWindowTextA(hLblStatus, msg.c_str());
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

bool StringToInt(const std::string& str, int& value) {
    if (str.empty()) return false;
    char* endptr;
    long val = strtol(str.c_str(), &endptr, 10);
    if (*endptr != '\0') return false;
    value = (int)val;
    return true;
}

// ====================================================================
// ОБРАБОТЧИКИ
// ====================================================================

void OnSolve() {
    std::string arrayStr = GetEditText(hEditArray);
    ParseInput(arrayStr, g_originalArray);
    DisplayArray(g_originalArray, hListOriginal);

    int k, m;
    std::string kStr = GetEditText(hEditK);
    if (!StringToInt(kStr, k) || k < 0) {
        MessageBoxA(hMainWnd, "k должно быть >= 0!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditK);
        return;
    }
    std::string mStr = GetEditText(hEditM);
    if (!StringToInt(mStr, m) || m < 0) {
        MessageBoxA(hMainWnd, "m должно быть >= 0!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditM);
        return;
    }
    if (k > m) {
        MessageBoxA(hMainWnd, "k должно быть <= m!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditK);
        return;
    }

    if (g_originalArray.empty()) {
        ClearListBox(hListResult);
        AddToListBox(hListResult, "Массив пуст");
        UpdateStatus("Ошибка: массив пуст", true);
        return;
    }

    if (k < (int)g_originalArray.size() && m < (int)g_originalArray.size()) {
        if (AllInRangeArePrime(g_originalArray, k, m)) {
            g_primeArray.clear();
            for (int x : g_originalArray) {
                if (IsPrime(x)) g_primeArray.push_back(x);
            }
            DisplayArray(g_primeArray, hListResult);
            char status[256];
            sprintf_s(status, "Условие выполнено! Найдено %zu простых чисел.", g_primeArray.size());
            UpdateStatus(status);
        } else {
            ClearListBox(hListResult);
            AddToListBox(hListResult, "Условие не выполнено");
            AddToListBox(hListResult, "Не все элементы в диапазоне [k, m] простые");
            UpdateStatus("Условие не выполнено", true);
        }
    } else {
        ClearListBox(hListResult);
        AddToListBox(hListResult, "Ошибка:");
        char buffer[256];
        sprintf_s(buffer, "Индексы k=%d или m=%d выходят за границы (размер=%zu)", k, m, g_originalArray.size());
        AddToListBox(hListResult, buffer);
        UpdateStatus("Ошибка: индексы вне границ", true);
    }
}

void OnClear() {
    SetWindowTextA(hEditArray, "");
    SetWindowTextA(hEditK, "");
    SetWindowTextA(hEditM, "");
    ClearListBox(hListOriginal);
    ClearListBox(hListResult);
    g_originalArray.clear();
    g_primeArray.clear();
    UpdateStatus("Введите данные и нажмите 'Решение'");
    SetFocus(hEditArray);
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

        // Массив
        HWND hLblArray = CreateWindowA("STATIC", "Массив (числа через пробел):", WS_CHILD | WS_VISIBLE,
            20, 30, 250, 25, hWnd, NULL, NULL, NULL);
        hEditArray = CreateWindowA("EDIT", "2 3 5 7 11 13 17 19 23", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            270, 30, 450, 25, hWnd, NULL, NULL, NULL);

        // k
        HWND hLblK = CreateWindowA("STATIC", "k:", WS_CHILD | WS_VISIBLE,
            20, 65, 30, 25, hWnd, NULL, NULL, NULL);
        hEditK = CreateWindowA("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            60, 65, 80, 25, hWnd, NULL, NULL, NULL);

        // m
        HWND hLblM = CreateWindowA("STATIC", "m:", WS_CHILD | WS_VISIBLE,
            160, 65, 30, 25, hWnd, NULL, NULL, NULL);
        hEditM = CreateWindowA("EDIT", "3", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            200, 65, 80, 25, hWnd, NULL, NULL, NULL);

        // Кнопки
        hBtnSolve = CreateWindowA("BUTTON", "Решение", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 105, 100, 35, hWnd, (HMENU)1, NULL, NULL);
        hBtnClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, 105, 100, 35, hWnd, (HMENU)2, NULL, NULL);
        hBtnExit = CreateWindowA("BUTTON", "Выход", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            250, 105, 100, 35, hWnd, (HMENU)3, NULL, NULL);

        // Списки
        HWND hLblOrig = CreateWindowA("STATIC", "Исходный массив:", WS_CHILD | WS_VISIBLE,
            20, 155, 150, 25, hWnd, NULL, NULL, NULL);
        hListOriginal = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            20, 185, 400, 250, hWnd, NULL, NULL, NULL);

        HWND hLblResult = CreateWindowA("STATIC", "Простые числа:", WS_CHILD | WS_VISIBLE,
            440, 155, 150, 25, hWnd, NULL, NULL, NULL);
        hListResult = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            440, 185, 400, 250, hWnd, NULL, NULL, NULL);

        // Статус
        hLblStatus = CreateWindowA("STATIC", "Введите данные и нажмите 'Решение'",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 450, 820, 30, hWnd, NULL, NULL, NULL);

        SendMessage(hLblArray, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditArray, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblK, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditK, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblM, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditM, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblOrig, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblResult, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == 1) OnSolve();
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
    wc.lpszClassName = "Lab11Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Ошибка регистрации класса!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    hMainWnd = CreateWindowA("Lab11Class", "ЛР №11 - Вариант 29",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 890, 540,
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