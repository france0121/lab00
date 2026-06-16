 // Lab14_29.cpp
// Лабораторная работа №14 - Вариант 29
// Чистый Win32 API, без C++/CLI, без pch.h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

HWND hMainWnd;
HWND hEditInput, hEditOutput;
HWND hListInput, hListOutput;
HWND hLblStatus;
HWND hBtnSelectInput, hBtnSelectOutput, hBtnProcess, hBtnClear, hBtnExit;

std::string g_inputPath;
std::string g_outputPath;

// ====================================================================
// ФУНКЦИИ РАБОТЫ С LISTBOX
// ====================================================================

void ClearListBox(HWND hList) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
}

void AddToListBox(HWND hList, const std::string& text) {
    SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
}

void LoadFileToList(const std::string& path, HWND hList) {
    ClearListBox(hList);
    std::ifstream file(path);
    if (!file.is_open()) {
        AddToListBox(hList, "Файл не найден или не может быть открыт");
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        AddToListBox(hList, line);
    }
    file.close();
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

void SetEditText(HWND hEdit, const std::string& text) {
    SetWindowTextA(hEdit, text.c_str());
}

void UpdateStatus(const std::string& msg, bool isError = false) {
    SetWindowTextA(hLblStatus, msg.c_str());
}

// ====================================================================
// ФУНКЦИЯ ВЫБОРА ФАЙЛА (ОТКРЫТИЕ)
// ====================================================================

std::string OpenFileDialog() {
    OPENFILENAMEA ofn = { 0 };
    char fileName[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);
    ofn.lpstrFilter = "Текстовые файлы\0*.txt\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(fileName);
    }
    return "";
}

// ====================================================================
// ФУНКЦИЯ ВЫБОРА ФАЙЛА (СОХРАНЕНИЕ)
// ====================================================================

std::string SaveFileDialog() {
    OPENFILENAMEA ofn = { 0 };
    char fileName[260] = "output.txt";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);
    ofn.lpstrFilter = "Текстовые файлы\0*.txt\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    if (GetSaveFileNameA(&ofn)) {
        std::string path(fileName);
        // Проверка расширения
        if (path.find(".txt") == std::string::npos) {
            path += ".txt";
        }
        return path;
    }
    return "";
}

// ====================================================================
// ОБРАБОТЧИКИ
// ====================================================================

void OnSelectInput() {
    std::string path = OpenFileDialog();
    if (!path.empty()) {
        g_inputPath = path;
        SetEditText(hEditInput, path);
        LoadFileToList(path, hListInput);
        UpdateStatus("Входной файл выбран: " + path.substr(path.find_last_of("\\") + 1));
    }
}

void OnSelectOutput() {
    std::string path = SaveFileDialog();
    if (!path.empty()) {
        g_outputPath = path;
        SetEditText(hEditOutput, path);
        UpdateStatus("Выходной файл выбран: " + path.substr(path.find_last_of("\\") + 1));
    }
}

void OnProcess() {
    if (g_inputPath.empty()) {
        MessageBoxA(hMainWnd, "Выберите входной файл!", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }
    if (g_outputPath.empty()) {
        MessageBoxA(hMainWnd, "Выберите выходной файл!", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    std::ifstream inFile(g_inputPath);
    if (!inFile.is_open()) {
        MessageBoxA(hMainWnd, "Не удалось открыть входной файл!", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    ClearListBox(hListOutput);

    std::ofstream outFile(g_outputPath);
    if (!outFile.is_open()) {
        inFile.close();
        MessageBoxA(hMainWnd, "Не удалось создать выходной файл!", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    std::string line;
    int totalLines = 0;
    int foundLines = 0;

    while (std::getline(inFile, line)) {
        totalLines++;
        if (line.find('@') != std::string::npos) {
            // Замена @ на [at]
            size_t pos = 0;
            while ((pos = line.find('@', pos)) != std::string::npos) {
                line.replace(pos, 1, "[at]");
                pos += 4;
            }
            outFile << line << std::endl;
            AddToListBox(hListOutput, line);
            foundLines++;
        }
    }

    inFile.close();
    outFile.close();

    char status[256];
    sprintf_s(status, "Обработано %d строк. Найдено %d строк с @ → [at]", totalLines, foundLines);
    UpdateStatus(status);

    if (foundLines == 0) {
        MessageBoxA(hMainWnd, "Строк с символом @ не найдено.", "Информация", MB_OK | MB_ICONINFORMATION);
    }
}

void OnClear() {
    g_inputPath = "";
    g_outputPath = "";
    SetEditText(hEditInput, "");
    SetEditText(hEditOutput, "");
    ClearListBox(hListInput);
    ClearListBox(hListOutput);
    UpdateStatus("Поля очищены. Выберите файлы.");
    SetFocus(hEditInput);
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

        // Входной файл
        HWND hLblInput = CreateWindowA("STATIC", "Входной:", WS_CHILD | WS_VISIBLE,
            20, 30, 70, 25, hWnd, NULL, NULL, NULL);
        hEditInput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
            100, 30, 500, 25, hWnd, NULL, NULL, NULL);
        hBtnSelectInput = CreateWindowA("BUTTON", "Выбрать...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            610, 30, 100, 25, hWnd, (HMENU)1, NULL, NULL);

        // Выходной файл
        HWND hLblOutput = CreateWindowA("STATIC", "Выходной:", WS_CHILD | WS_VISIBLE,
            20, 65, 70, 25, hWnd, NULL, NULL, NULL);
        hEditOutput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
            100, 65, 500, 25, hWnd, NULL, NULL, NULL);
        hBtnSelectOutput = CreateWindowA("BUTTON", "Выбрать...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            610, 65, 100, 25, hWnd, (HMENU)2, NULL, NULL);

        // Кнопка обработки
        hBtnProcess = CreateWindowA("BUTTON", "Обработка", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            730, 35, 120, 50, hWnd, (HMENU)3, NULL, NULL);

        // Списки
        HWND hLblInList = CreateWindowA("STATIC", "Входной файл:", WS_CHILD | WS_VISIBLE,
            20, 110, 150, 25, hWnd, NULL, NULL, NULL);
        hListInput = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            20, 140, 400, 300, hWnd, NULL, NULL, NULL);

        HWND hLblOutList = CreateWindowA("STATIC", "Выходной файл (строки с @ → [at]):", WS_CHILD | WS_VISIBLE,
            450, 110, 250, 25, hWnd, NULL, NULL, NULL);
        hListOutput = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            450, 140, 400, 300, hWnd, NULL, NULL, NULL);

        // Статус
        hLblStatus = CreateWindowA("STATIC", "Выберите файлы и нажмите 'Обработка'",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 460, 830, 30, hWnd, NULL, NULL, NULL);

        // Кнопки
        hBtnClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            280, 505, 130, 35, hWnd, (HMENU)4, NULL, NULL);
        hBtnExit = CreateWindowA("BUTTON", "Выход", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            430, 505, 130, 35, hWnd, (HMENU)5, NULL, NULL);

        // Установка шрифтов
        SendMessage(hLblInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblInList, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblOutList, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hLblStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
        case 1: OnSelectInput(); break;
        case 2: OnSelectOutput(); break;
        case 3: OnProcess(); break;
        case 4: OnClear(); break;
        case 5: OnExit(); break;
        }
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
    wc.lpszClassName = "Lab14Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Ошибка регистрации класса!", "Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    hMainWnd = CreateWindowA("Lab14Class", "ЛР №14 - Вариант 29",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 890, 600,
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