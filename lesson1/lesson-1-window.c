#include <windows.h>

// Прототип оконной процедуры
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Точка входа
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR pCmdLine, int nCmdShow)
{
    // 1. РЕГИСТРИРУЕМ КЛАСС ОКНА
    const wchar_t CLASS_NAME[] = L"MyWindowClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // 2. СОЗДАЕМ ОКНО
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Урок 1: Моё первое окно",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 3. ПОКАЗЫВАЕМ ОКНО
    ShowWindow(hwnd, nCmdShow);

    // 4. ЦИКЛ СООБЩЕНИЙ
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// ФУНКЦИЯ ОБРАБОТКИ ОКНА
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Очищаем фон окна
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        // Выводим текст
        TextOutW(hdc, 50, 50, L"Поздравляю! Вы создали:", 26);
        TextOutW(hdc, 70, 80, L"1. Настоящее окно Windows", 26);
        TextOutW(hdc, 70, 110, L"2. С обработкой сообщений", 26);
        TextOutW(hdc, 70, 140, L"3. С возможностью рисования", 28);

        TextOutW(hdc, 50, 180, L"Закройте это окно для завершения.", 33);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
