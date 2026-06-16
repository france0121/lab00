 // Lab12_29.cpp
// Лабораторная работа №12 - Вариант 29
// Чистый Win32 API, без C++/CLI, без pch.h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <random>
#include <string>

#pragma comment(lib, "comctl32.lib")

HWND hMainWnd;
HWND hEditRowsA, hEditColsA, hEditRowsB, hEditColsB, hEditMin, hEditMax;
HWND hListA, hListB, hListR;
HWND hLblStatus;

std::vector<std::vector<double>> g_A, g_B, g_R;

// ====================================================================
// ЛОГИЧЕСКИЕ ФУНКЦИИ
// ====================================================================

void GenerateMatrix(int rows, int cols, double minVal, double maxVal,
                    std::vector<std::vector<double>>& mat) {
    mat.clear();
    mat.resize(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(minVal, maxVal);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mat[i][j] = dist(gen);
        }
    }
}

double MaxInRow(const std::vector<double>& row) {
    if (row.empty()) return 0;
    double mx = row[0];
    for (double val : row) {
        if (val > mx) mx = val;
    }
    return mx;
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

void DisplayMatrix(const std::vector<std::vector<double>>& mat, HWND hList) {
    ClearListBox(hList);
    if (mat.empty()) {
        AddToListBox(hList, "Пусто");
        return;
    }
    for (size_t i = 0; i < mat.size(); ++i) {
        std::string line;
        for (size_t j = 0; j < mat[i].size(); ++j) {
            char buffer[32];
            sprintf_s(buffer, "%8.2f", mat[i][j]);
            line += buffer;
        }
        AddToListBox(hList, line);
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

bool StringToDouble(const std::string& str, double& value) {
    if (str.empty()) return false;
    char* endptr;
    double val = strtod(str.c_str(), &endptr);
    if (*endptr != '\0') return false;
    value = val;
    return true;
}

// ====================================================================
// ОБРАБОТЧИКИ
// ====================================================================

void OnSolve() {
    int rowsA, colsA, rowsB, colsB;
    double minVal, maxVal;

    if (!StringToInt(GetEditText(hEditRowsA), rowsA) || rowsA < 1) {
        MessageBoxA(hMainWnd, "Строки A должны быть >= 1!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditRowsA);
        return;
    }
    if (!StringToInt(GetEditText(hEditColsA), colsA) || colsA < 1) {
        MessageBoxA(hMainWnd, "Столбцы A должны быть >= 1!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditColsA);
        return;
    }
    if (!StringToInt(GetEditText(hEditRowsB), rowsB) || rowsB < 1) {
        MessageBoxA(hMainWnd, "Строки B должны быть >= 1!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditRowsB);
        return;
    }
    if (!StringToInt(GetEditText(hEditColsB), colsB) || colsB < 1) {
        MessageBoxA(hMainWnd, "Столбцы B должны быть >= 1!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditColsB);
        return;
    }

    if (rowsA != rowsB) {
        MessageBoxA(hMainWnd, "Количество строк A и B должно совпадать!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditRowsA);
        return;
    }

    if (!StringToDouble(GetEditText(hEditMin), minVal)) {
        MessageBoxA(hMainWnd, "Введите корректное минимальное значение!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMin);
        return;
    }
    if (!StringToDouble(GetEditText(hEditMax), maxVal)) {
        MessageBoxA(hMainWnd, "Введите корректное максимальное значение!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMax);
        return;
    }
    if (minVal >= maxVal) {
        MessageBoxA(hMainWnd, "min должен быть < max!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMin);
        return;
    }

    GenerateMatrix(rowsA, colsA, minVal, maxVal, g_A);
    GenerateMatrix(rowsB, colsB, minVal, maxVal, g_B);

    DisplayMatrix(g_A, hListA);
    DisplayMatrix(g_B, hListB);

    g_R.resize(rowsA, std::vector<double>(colsA));
    for (int i = 0; i < rowsA; ++i) {
        double maxB = MaxInRow(g_B[i]);
        for (int j = 0; j < colsA; ++j) {
            g_R[i][j] = g_A[i][j] * maxB;
        }
    }

    DisplayMatrix(g_R, hListR);
    char status[256];
    sprintf_s(status, "Готово! Размер R: %dx%d", rowsA, colsA);
    UpdateStatus(status);
}

void OnClear() {
    SetWindowTextA(hEditRowsA, "");
    SetWindowTextA(hEditColsA, "");
    SetWindowTextA(hEditRowsB, "");
    SetWindowTextA(hEditColsB, "");
    SetWindowTextA(hEditMin, "");
    SetWindowTextA(hEditMax, "");
    ClearListBox(hListA);
    ClearListBox(hListB);
    ClearListBox(hListR);
    g_A.clear();
    g_B.clear();
    g_R.clear();
    UpdateStatus("Введите параметры и нажмите 'Решение'");
    SetFocus(hEditRowsA);
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

        // Строка 0: A
        HWND hLblRowsA = CreateWindowA("STATIC", "Строки A:", WS_CHILD | WS_VISIBLE,
            20, 30, 80, 25, hWnd, NULL, NULL, NULL);
        hEditRowsA = CreateWindowA("EDIT", "5", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            100, 30, 60, 25, hWnd, NULL, NULL, NULL);

        HWND hLblColsA = CreateWindowA("STATIC", "Столбцы A:", WS_CHILD | WS_VISIBLE,
            200, 30, 80, 25, hWnd, NULL, NULL, NULL);
        hEditColsA = CreateWindowA("EDIT", "3", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            280, 30, 60, 25, hWnd, NULL, NULL, NULL);

        // Строка 1: B
        HWND hLblRowsB = CreateWindowA("STATIC", "Строки B:", WS_CHILD | WS_VISIBLE,
            20, 65, 80, 25, hWnd, NULL, NULL, NULL);
        hEditRowsB = CreateWindowA("EDIT", "5", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            100, 65, 60, 25, hWnd, NULL, NULL, NULL);

        HWND hLblColsB = CreateWindowA("STATIC", "Столбцы B:", WS_CHILD | WS_VISIBLE,
            200, 65, 80, 25, hWnd, NULL, NULL, NULL);
        hEditColsB = CreateWindowA("EDIT", "4", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            280, 65, 60, 25, hWnd, NULL, NULL, NULL);

        // Строка 2: Диапазон
        HWND hLblMin = CreateWindowA("STATIC", "Минимум:", WS_CHILD | WS_VISIBLE,
            20, 100, 80, 25, hWnd, NULL, NULL, NULL);
        hEditMin = CreateWindowA("EDIT", "-10", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            100, 100, 60, 25, hWnd, NULL, NULL, NULL);

        HWND hLblMax = CreateWindowA("STATIC", "Максимум:", WS_CHILD | WS_VISIBLE,
            200, 100, 80, 25, hWnd, NULL, NULL, NULL);
        hEditMax = CreateWindowA("EDIT", "10", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            280, 100, 60, 25, hWnd, NULL, NULL, NULL);

        // Кнопки
        hBtnSolve = CreateWindowA("BUTTON", "Решение", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 140, 100, 35, hWnd, (HMENU)1, NULL, NULL);
        hBtnClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, 140, 100, 35, hWnd, (HMENU)2, NULL, NULL);
        hBtnExit = CreateWindowA("BUTTON", "Выход", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            250, 140, 100, 35, hWnd, (HMENU)3, NULL, NULL);

        // Списки
        HWND hLblA = CreateWindowA("STATIC", "Матрица A:", WS_CHILD | WS_VISIBLE,
            20, 190, 120, 25, hWnd, NULL, NULL, NULL);
        hListA = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            20, 220, 250, 250, hWnd, NULL, NULL, NULL);

        HWND hLblB = CreateWindowA("STATIC", "Матрица B:", WS_CHILD | WS_VISIBLE,
            290, 190, 120, 25, hWnd, NULL, NULL, NULL);
        hListB = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            290, 220, 250, 250, hWnd, NULL, NULL, NULL);

        HWND hLblR = CreateWindowA("STATIC", "Результат R:", WS_CHILD | WS_VISIBLE,
            560, 190, 120, 25, hWnd, NULL, NULL, NULL);
        hListR = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            560, 220, 250, 250, hWnd, NULL, NULL, NULL);

        // Статус
        hLblStatus = CreateWindowA("STATIC", "Введите параметры и нажмите 'Решение'",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 490, 790, 30, hWnd, NULL, NULL, NULL);

        // Установка шрифтов
        SendMessage(hLblRowsA, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditRowsA, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblColsA, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditColsA, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblRowsB, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditRowsB, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblColsB, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditColsB, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblMin, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditMin, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblMax, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditMax, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblA, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblB, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblR, WM_SETFONT, (WPARAM)hFont, TRUE);
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
    wc.lpszClassName = "Lab12Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Ошибка регистрации класса!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    hMainWnd = CreateWindowA("Lab12Class", "ЛР №12 - Вариант 29",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 850, 580,
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