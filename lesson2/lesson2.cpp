#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Прототипы функций
void PrintMenu();
void ConvertStrings();
void AnalyzeError();
void CheckFileExists();
void DemonstrateCustomError();
void SafeStringOperationsDemo();
void PrintError(DWORD errorCode, const char* functionName);

// Главная функция
int main()
{
    // НАСТРОЙКА РУССКОЙ ЛОКАЛИ
    SetConsoleCP(1251);           // Кодовая страница ввода Windows-1251
    SetConsoleOutputCP(1251);     // Кодовая страница вывода Windows-1251

    printf("=== Утилита безопасной работы со строками и ошибками WinAPI ===\n\n");

    int choice = 0;
    char input[10];

    while (1)
    {
        PrintMenu();
        printf("Выберите действие (1-6): ");

        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            choice = atoi(input);
        }

        switch (choice)
        {
        case 1:
            ConvertStrings();
            break;
        case 2:
            AnalyzeError();
            break;
        case 3:
            CheckFileExists();
            break;
        case 4:
            DemonstrateCustomError();
            break;
        case 5:
            SafeStringOperationsDemo();
            break;
        case 6:
            printf("Выход из программы.\n");
            return 0;
        default:
            printf("Неверный выбор. Попробуйте снова.\n");
            break;
        }

        if (choice != 6)
        {
            printf("\nНажмите Enter для продолжения...");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }

    return 0;
}

// Функция 1: Конвертер строк
void ConvertStrings()
{
    printf("\n=== КОНВЕРТЕР СТРОК ===\n\n");

    printf("Введите строку для конвертации: ");

    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        printf("Ошибка ввода.\n");
        return;
    }

    // Удаление символа новой строки
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
    {
        input[len - 1] = '\0';
    }

    printf("Исходная строка: %s\n", input);
    printf("Длина строки: %zu символов\n", strlen(input));

    // ANSI (Windows-1251) -> Unicode (UTF-16)
    wchar_t unicodeBuffer[256];
    int unicodeLength = MultiByteToWideChar(1251, 0, input, -1, unicodeBuffer, 256);

    if (unicodeLength > 0)
    {
        printf("Unicode версия (%d символов): ", unicodeLength - 1);

        // Выводим коды Unicode (UTF-16) для каждого символа
        for (int i = 0; i < unicodeLength - 1; i++)  // -1 чтобы исключить нулевой терминатор
        {
            printf("U+%04X ", (unsigned int)unicodeBuffer[i]);
        }
        printf("\n");
    }
    else
    {
        PrintError(GetLastError(), "MultiByteToWideChar");
    }

    // Unicode -> ANSI (обратная конвертация)
    char ansiBuffer[256];
    int ansiLength = WideCharToMultiByte(1251, 0, unicodeBuffer, -1, ansiBuffer, 256, NULL, NULL);

    if (ansiLength > 0)
    {
        printf("ANSI версия после обратной конвертации (%d байт): %s\n", ansiLength - 1, ansiBuffer);
    }
}

// Функция 2: Анализатор ошибок
void AnalyzeError()
{
    printf("\n=== АНАЛИЗАТОР ОШИБОК WINAPI ===\n\n");

    printf("Введите код ошибки (например, 5 для ERROR_ACCESS_DENIED): ");

    char errorStr[20];
    if (fgets(errorStr, sizeof(errorStr), stdin) == NULL)
    {
        printf("Ошибка ввода.\n");
        return;
    }

    // Удаление символа новой строки
    size_t len = strlen(errorStr);
    if (len > 0 && errorStr[len - 1] == '\n')
    {
        errorStr[len - 1] = '\0';
    }

    DWORD errorCode = atoi(errorStr);

    // Получение системного сообщения об ошибке
    LPSTR errorMsg = NULL;
    DWORD chars = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorMsg,
        0,
        NULL
    );

    if (chars > 0 && errorMsg != NULL)
    {
        printf("\nКод ошибки: %lu (0x%08lX)\n", errorCode, errorCode);
        printf("Системное сообщение: %s\n", errorMsg);

        // Дополнительная информация о распространенных ошибках
        switch (errorCode)
        {
        case ERROR_SUCCESS:
            printf("Описание: Операция выполнена успешно\n");
            break;
        case ERROR_FILE_NOT_FOUND:
            printf("Описание: Указанный файл не найден\n");
            printf("Возможные причины: неправильный путь, файл удален\n");
            break;
        case ERROR_PATH_NOT_FOUND:
            printf("Описание: Указанный путь не найден\n");
            printf("Возможные причины: неправильная структура каталогов\n");
            break;
        case ERROR_ACCESS_DENIED:
            printf("Описание: Доступ запрещен\n");
            printf("Возможные причины: недостаточно прав, файл заблокирован\n");
            break;
        case ERROR_INVALID_PARAMETER:
            printf("Описание: Неверный параметр\n");
            printf("Возможные причины: нулевой указатель, некорректное значение\n");
            break;
        default:
            printf("Используйте этот код для поиска в документации WinAPI\n");
            break;
        }

        LocalFree(errorMsg);
    }
    else
    {
        printf("Не удалось получить сообщение для кода %lu\n", errorCode);
        printf("Возможно, это пользовательский или неизвестный код ошибки\n");
    }
}

// Функция 3: Проверка существования файла
void CheckFileExists()
{
    printf("\n=== ПРОВЕРКА СУЩЕСТВОВАНИЯ ФАЙЛА ===\n\n");

    printf("Введите путь к файлу: ");

    char filePath[MAX_PATH];
    if (fgets(filePath, sizeof(filePath), stdin) == NULL)
    {
        printf("Ошибка ввода.\n");
        return;
    }

    // Удаление символа новой строки
    size_t len = strlen(filePath);
    if (len > 0 && filePath[len - 1] == '\n')
    {
        filePath[len - 1] = '\0';
    }

    printf("Проверяю файл: %s\n", filePath);

    // Конвертация в Unicode (для GetFileAttributesW)
    wchar_t wfilePath[MAX_PATH];
    MultiByteToWideChar(1251, 0, filePath, -1, wfilePath, MAX_PATH);

    // Проверка файла с обработкой ошибок
    DWORD attributes = GetFileAttributesW(wfilePath);

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        DWORD error = GetLastError();
        printf("Файл не существует.\n");
        PrintError(error, "GetFileAttributes");
    }
    else
    {
        printf("✓ Файл существует\n");

        printf("Атрибуты файла:\n");
        if (attributes & FILE_ATTRIBUTE_READONLY)
            printf("  - Только для чтения\n");
        if (attributes & FILE_ATTRIBUTE_HIDDEN)
            printf("  - Скрытый\n");
        if (attributes & FILE_ATTRIBUTE_SYSTEM)
            printf("  - Системный\n");
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
            printf("  - Каталог\n");
        if (attributes & FILE_ATTRIBUTE_ARCHIVE)
            printf("  - Архивный\n");
        if (attributes & FILE_ATTRIBUTE_NORMAL)
            printf("  - Обычный файл\n");
        if (attributes & FILE_ATTRIBUTE_COMPRESSED)
            printf("  - Сжатый\n");
        if (attributes & FILE_ATTRIBUTE_ENCRYPTED)
            printf("  - Зашифрованный\n");
    }
}

// Функция 4: Демонстрация пользовательской ошибки
void DemonstrateCustomError()
{
    printf("\n=== ПОЛЬЗОВАТЕЛЬСКИЕ ОШИБКИ ===\n\n");

    // Пример 1: Установка и получение кода ошибки
    printf("1. Установка пользовательского кода ошибки:\n");

    // Сохраняем текущий код ошибки
    DWORD originalError = GetLastError();
    printf("   Текущий код ошибки: %lu\n", originalError);

    // Устанавливаем пользовательский код
    DWORD customError = 0x1000;
    SetLastError(customError);

    // Получаем обратно
    DWORD retrievedError = GetLastError();
    printf("   Установлен код: 0x%08lX\n", retrievedError);
    printf("   Проверка: %s\n", (retrievedError == customError) ? "OK" : "Ошибка");

    // Восстанавливаем исходный код
    SetLastError(originalError);

    // Пример 2: Стандартные коды ошибок
    printf("\n2. Стандартные коды ошибок Windows:\n");
    printf("   ERROR_SUCCESS = %d (успешное выполнение)\n", ERROR_SUCCESS);
    printf("   ERROR_FILE_NOT_FOUND = %d (файл не найден)\n", ERROR_FILE_NOT_FOUND);
    printf("   ERROR_PATH_NOT_FOUND = %d (путь не найден)\n", ERROR_PATH_NOT_FOUND);
    printf("   ERROR_ACCESS_DENIED = %d (доступ запрещен)\n", ERROR_ACCESS_DENIED);
    printf("   ERROR_INVALID_PARAMETER = %d (неверный параметр)\n", ERROR_INVALID_PARAMETER);
    printf("   ERROR_OUTOFMEMORY = %d (недостаточно памяти)\n", ERROR_OUTOFMEMORY);

    // Пример 3: Проверка успешности операции
    printf("\n3. Проверка успешности операции:\n");

    // Симулируем успешную операцию
    SetLastError(ERROR_SUCCESS);
    if (GetLastError() == ERROR_SUCCESS)
    {
        printf("   Последняя операция выполнена успешно\n");
    }

    // Симулируем ошибочную операцию
    SetLastError(ERROR_FILE_NOT_FOUND);
    DWORD currentError = GetLastError();
    if (currentError != ERROR_SUCCESS)
    {
        printf("   Последняя операция завершилась с ошибкой: %lu\n", currentError);

        // Получаем описание ошибки
        LPSTR errorMsg = NULL;
        if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, currentError, 0, (LPSTR)&errorMsg, 0, NULL) > 0)
        {
            printf("   Описание: %s", errorMsg);
            LocalFree(errorMsg);
        }
    }
}

// Функция 5: Безопасные строковые операции
void SafeStringOperationsDemo()
{
    printf("\n=== БЕЗОПАСНЫЕ СТРОКОВЫЕ ОПЕРАЦИИ ===\n\n");

    // Пример 1: Безопасное копирование строк
    printf("1. Безопасное копирование строк:\n");
    char source[] = "Пример строки для копирования";
    char dest1[50];

    // Используем strcpy_s вместо strcpy
    errno_t result = strcpy_s(dest1, sizeof(dest1), source);

    if (result == 0)
    {
        printf("   Копирование успешно: %s\n", dest1);
    }
    else
    {
        printf("   Ошибка копирования: код %d\n", result);
        printf("   Возможные причины: буфер слишком мал или нулевой указатель\n");
    }

    // Пример 2: Предотвращение переполнения будера
    printf("\n2. Предотвращение переполнения буфера:\n");
    char dest2[10];  // Маленький буфер

    // Используем strncpy_s вместо strcpy_s - она не вызывает assertion
    result = strncpy_s(dest2, sizeof(dest2), source, _TRUNCATE);

    if (result == 0)
    {
        printf("   Копирование успешно: %s\n", dest2);
    }
    else if (result == STRUNCATE)
    {
        printf("   Безопасная функция предотвратила переполнение буфера!\n");
        printf("   Строка была усечена чтобы поместиться в буфер\n");
        printf("   Размер буфера: %zu байт\n", sizeof(dest2));
        printf("   Результат: %s\n", dest2);
        printf("   Длина результата: %zu символов\n", strlen(dest2));
    }
    else
    {
        printf("   Ошибка: код %d\n", result);
    }

    // Пример 3: Безопасная конкатенация - ИСПРАВЛЕННЫЙ ВАРИАНТ
    printf("\n3. Безопасная конкатенация строк:\n");
    char path[MAX_PATH] = "C:\\Windows\\";
    char filename[] = "system32\\kernel32.dll";

    // Показываем начальное состояние
    printf("   Исходная строка: %s\n", path);
    printf("   Размер буфера: %zu байт\n", sizeof(path));
    printf("   Длина исходной строки: %zu символов\n", strlen(path));
    printf("   Строка для добавления: %s (длина: %zu символов)\n", filename, strlen(filename));

    result = strcat_s(path, sizeof(path), filename);

    if (result == 0)
    {
        printf("   Конкатенация успешна: %s\n", path);
        printf("   Итоговая длина: %zu символов\n", strlen(path));
    }
    else if (result == ERANGE)
    {
        printf("   Ошибка: результирующая строка слишком длинная для буфера!\n");
        printf("   Ожидаемая длина: %zu символов\n", strlen("C:\\Windows\\") + strlen(filename));
        printf("   Максимальная длина: %zu символов\n", sizeof(path) - 1);
        printf("   Буфер остался нетронутым: %s\n", path);

        // Альтернатива: безопасное копирование с усечением
        char alt_path[MAX_PATH] = "C:\\Windows\\";
        strncat_s(alt_path, sizeof(alt_path), filename,
            sizeof(alt_path) - strlen(alt_path) - 1); // -1 для нуль-терминатора
        printf("   Использовано безопасное добавление: %s\n", alt_path);
    }
    else
    {
        printf("   Другая ошибка: код %d\n", result);
    }

    // Пример 4: Безопасное форматирование
    printf("\n4. Безопасное форматирование строк:\n");
    char buffer[100];
    int number = 42;
    double pi = 3.14159;
    const char* text = "текст";

    result = sprintf_s(buffer, sizeof(buffer),
        "Число: %d, Pi: %.2f, Текст: %s", number, pi, text);

    if (result >= 0)
    {
        printf("   Форматирование успешно: %s\n", buffer);
        printf("   Записано символов: %d\n", result);
    }
    else
    {
        printf("   Ошибка форматирования: код %d\n", result);
    }

    // Пример 5: Сравнение строк
    printf("\n5. Сравнение строк:\n");
    char str1[] = "Hello";
    char str2[] = "Hello";
    char str3[] = "World";

    int cmp1 = strcmp(str1, str2);
    int cmp2 = strcmp(str1, str3);

    printf("   strcmp(\"%s\", \"%s\") = %d\n", str1, str2, cmp1);
    printf("   strcmp(\"%s\", \"%s\") = %d\n", str1, str3, cmp2);

    if (cmp1 == 0)
    {
        printf("   Строки идентичны\n");
    }
    else if (cmp2 < 0)
    {
        printf("   Строка \"%s\" меньше строки \"%s\"\n", str1, str3);
    }
    else
    {
        printf("   Строка \"%s\" больше строки \"%s\"\n", str1, str3);
    }
}

// Вспомогательная функция: вывод сообщения об ошибке
void PrintError(DWORD errorCode, const char* functionName)
{
    LPSTR errorMsg = NULL;
    DWORD chars = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorMsg,
        0,
        NULL
    );

    if (chars > 0 && errorMsg != NULL)
    {
        printf("Ошибка в функции %s (код %lu): %s", functionName, errorCode, errorMsg);
        LocalFree(errorMsg);
    }
    else
    {
        printf("Неизвестная ошибка в функции %s: код %lu\n", functionName, errorCode);
    }
}

// Вспомогательная функция: вывод меню
void PrintMenu()
{
    printf("\n=== ГЛАВНОЕ МЕНЮ ===\n\n");

    printf("1. Конвертер строк (ANSI <-> Unicode)\n");
    printf("2. Анализатор кодов ошибок WinAPI\n");
    printf("3. Проверка существования файла\n");
    printf("4. Демонстрация пользовательских ошибок\n");
    printf("5. Безопасные строковые операции\n");
    printf("6. Выход\n\n");
}
