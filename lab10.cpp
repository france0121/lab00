 // Lab10_29.cpp
// Лабораторная работа №10 - Вариант 29
// Чистый Win32 API, без C++/CLI, без pch.h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <random>
#include <string>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

// ====================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ====================================================================
HWND hMainWnd;
HWND hEditSize, hEditMin, hEditMax, hEditK, hEditM;
HWND hListOriginal, hListResult;
HWND hBtnSolve, hBtnClear, hBtnExit;
HWND hLblStatus;

std::vector<int> g_originalArray;
std::vector<int> g_primeArray;

// ====================================================================
// ЛОГИЧЕСКИЕ ФУНКЦИИ
// ====================================================================

// Проверка, является ли число простым
bool IsPrime(int n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Проверка, все ли элементы в диапазоне [k, m] простые
bool AllInRangeArePrime(const std::vector<int>& arr, int k, int m) {
    if (k < 0 || m >= (int)arr.size() || k > m) return false;
    for (int i = k; i <= m; ++i) {
        if (!IsPrime(arr[i])) return false;
    }
    return true;
}

// Генерация случайного массива
void GenerateArray(int size, int minVal, int maxVal, std::vector<int>& arr) {
    arr.clear();
    arr.reserve(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    for (int i = 0; i < size; ++i) {
        arr.push_back(dist(gen));
    }
}

// ====================================================================
// ФУНКЦИИ РАБОТЫ С LISTBOX
// ====================================================================

// Очистка ListBox
void ClearListBox(HWND hList) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
}

// Добавление строки в ListBox
void AddToListBox(HWND hList, const std::string& text) {
    SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
}

// Отображение массива в ListBox
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

// ====================================================================
// ОБНОВЛЕНИЕ СТАТУСА
// ====================================================================
void UpdateStatus(const std::string& msg, bool isError = false) {
    SetWindowTextA(hLblStatus, msg.c_str());
    if (isError) {
        SetWindowLongPtr(hLblStatus, GWL_EXSTYLE, 0);
    } else {
        SetWindowLongPtr(hLblStatus, GWL_EXSTYLE, 0);
    }
}

// ====================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ====================================================================

// Получение текста из Edit
std::string GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthA(hEdit);
    if (len == 0) return "";
    char* buffer = new char[len + 1];
    GetWindowTextA(hEdit, buffer, len + 1);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

// Преобразование строки в число
bool StringToInt(const std::string& str, int& value) {
    if (str.empty()) return false;
    char* endptr;
    long val = strtol(str.c_str(), &endptr, 10);
    if (*endptr != '\0') return false;
    value = (int)val;
    return true;
}

// ====================================================================
// ОБРАБОТЧИК КНОПКИ "РЕШЕНИЕ"
// ====================================================================
void OnSolve() {
    int size, minVal, maxVal, k, m;

    // Чтение размера
    std::string sizeStr = GetEditText(hEditSize);
    if (!StringToInt(sizeStr, size) || size < 1 || size > 1000) {
        MessageBoxA(hMainWnd, "Размер должен быть от 1 до 1000!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditSize);
        return;
    }

    // Чтение минимума
    std::string minStr = GetEditText(hEditMin);
    if (!StringToInt(minStr, minVal)) {
        MessageBoxA(hMainWnd, "Введите корректное минимальное значение!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMin);
        return;
    }

    // Чтение максимума
    std::string maxStr = GetEditText(hEditMax);
    if (!StringToInt(maxStr, maxVal)) {
        MessageBoxA(hMainWnd, "Введите корректное максимальное значение!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMax);
        return;
    }

    if (minVal >= maxVal) {
        MessageBoxA(hMainWnd, "Минимальное значение должно быть меньше максимального!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditMin);
        return;
    }

    // Чтение k
    std::string kStr = GetEditText(hEditK);
    if (!StringToInt(kStr, k) || k < 0) {
        MessageBoxA(hMainWnd, "k должно быть >= 0!", "Ошибка", MB_OK | MB_ICONERROR);
        SetFocus(hEditK);
        return;
    }

    // Чтение m
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

    // Генерация массива
    GenerateArray(size, minVal, maxVal, g_originalArray);
    DisplayArray(g_originalArray, hListOriginal);

    // Проверка условия
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
            AddToListBox(hListResult, "Условие не выполнено:");
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

// ====================================================================
// ОБРАБОТЧИК КНОПКИ "ОЧИСТИТЬ"
// ====================================================================
void OnClear() {
    SetWindowTextA(hEditSize, "");
    SetWindowTextA(hEditMin, "");
    SetWindowTextA(hEditMax, "");
    SetWindowTextA(hEditK, "");
    SetWindowTextA(hEditM, "");
    ClearListBox(hListOriginal);
    ClearListBox(hListResult);
    g_originalArray.clear();
    g_primeArray.clear();
    UpdateStatus("Введите параметры и нажмите 'Решение'");
    SetFocus(hEditSize);
}

// ====================================================================
// ОБРАБОТЧИК КНОПКИ "ВЫХОД"
// ====================================================================
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

        // Создание шрифта
        HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        // ---- ГРУППА ВВОДА ----
        // Заголовок
        HWND hLblSize = CreateWindowA("STATIC", "Размер:", WS_CHILD | WS_VISIBLE,
            20, 30, 100, 25, hWnd, NULL, NULL, NULL);
        hEditSize = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 30, 120, 25, hWnd, NULL, NULL, NULL);
        SendMessage(hEditSize, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLblMin = CreateWindowA("STATIC", "Минимум:", WS_CHILD | WS_VISIBLE,
            20, 65, 100, 25, hWnd, NULL, NULL, NULL);
        hEditMin = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 65, 120, 25, hWnd, NULL, NULL, NULL);
        SendMessage(hEditMin, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLblMax = CreateWindowA("STATIC", "Максимум:", WS_CHILD | WS_VISIBLE,
            20, 100, 100, 25, hWnd, NULL, NULL, NULL);
        hEditMax = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 100, 120, 25, hWnd, NULL, NULL, NULL);
        SendMessage(hEditMax, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLblK = CreateWindowA("STATIC", "k (начало):", WS_CHILD | WS_VISIBLE,
            20, 135, 100, 25, hWnd, NULL, NULL, NULL);
        hEditK = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 135, 120, 25, hWnd, NULL, NULL, NULL);
        SendMessage(hEditK, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLblM = CreateWindowA("STATIC", "m (конец):", WS_CHILD | WS_VISIBLE,
            20, 170, 100, 25, hWnd, NULL, NULL, NULL);
        hEditM = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 170, 120, 25, hWnd, NULL, NULL, NULL);
        SendMessage(hEditM, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Кнопки
        hBtnSolve = CreateWindowA("BUTTON", "Решение", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 210, 100, 35, hWnd, (HMENU)1, NULL, NULL);
        hBtnClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, 210, 100, 35, hWnd, (HMENU)2, NULL, NULL);
        hBtnExit = CreateWindowA("BUTTON", "Выход", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            250, 210, 100, 35, hWnd, (HMENU)3, NULL, NULL);

        // ---- СПИСКИ ----
        // Левый список - исходный массив
        HWND hLblOrig = CreateWindowA("STATIC", "Исходный массив:", WS_CHILD | WS_VISIBLE,
            290, 30, 150, 25, hWnd, NULL, NULL, NULL);
        hListOriginal = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            290, 60, 250, 200, hWnd, NULL, NULL, NULL);

        // Правый список - простые числа
        HWND hLblResult = CreateWindowA("STATIC", "Простые числа:", WS_CHILD | WS_VISIBLE,
            560, 30, 150, 25, hWnd, NULL, NULL, NULL);
        hListResult = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            560, 60, 250, 200, hWnd, NULL, NULL, NULL);

        // ---- СТАТУС ----
        hLblStatus = CreateWindowA("STATIC", "Введите параметры и нажмите 'Решение'",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 280, 790, 30, hWnd, NULL, NULL, NULL);

        // Установка шрифтов
        SendMessage(hLblSize, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblMin, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblMax, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblK, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblM, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblOrig, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblResult, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Заполнение примером
        SetWindowTextA(hEditSize, "20");
        SetWindowTextA(hEditMin, "-100");
        SetWindowTextA(hEditMax, "100");
        SetWindowTextA(hEditK, "0");
        SetWindowTextA(hEditM, "3");

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

// ====================================================================
// ТОЧКА ВХОДА
// ====================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "Lab10Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Ошибка регистрации класса!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Создание окна
    hMainWnd = CreateWindowA("Lab10Class", "ЛР №10 - Вариант 29",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 850, 370,
        NULL, NULL, hInstance, NULL);

    if (!hMainWnd) {
        MessageBoxA(NULL, "Ошибка создания окна!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}