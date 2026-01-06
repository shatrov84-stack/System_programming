Отличный материал для третьего урока! Вот оформленная версия в стиле предыдущего урока:

# 🚀 **УРОК 3: ПРОЦЕССЫ WINDOWS — СОЗДАНИЕ И УПРАВЛЕНИЕ**

**Цель урока:**  
Понимать жизненный цикл, структуру и механизмы управления процессами в Windows. Научиться создавать, настраивать, перечислять и анализировать процессы с помощью WinAPI.

---

## 📚 **ТЕОРЕТИЧЕСКАЯ ЧАСТЬ (по книге Иосифовича)**

### **Страницы для изучения:**
- **Глава 3 (страницы 45-60):** "Process Basics" — основы процессов
- **Глава 3 (страницы 61-75):** "Process Creation and Termination" — создание и завершение
- **Глава 3 (страницы 76-85):** "Process Information and Enumeration" — информация и перечисление

### **Ключевые концепции:**

#### **1. Что такое процесс Windows:**
- **Контейнер ресурсов:** Виртуальное адресное пространство, код, данные, дескрипторы
- **Объект ядра:** Управляется диспетчером объектов Windows
- **Состояния:** Создан, готов, выполняется, ожидает, завершен
- **Атрибуты:** PID, токен доступа, приоритет, квоты

#### **2. Создание процессов:**
- **`CreateProcess()`:** Основная функция создания процессов
- **`STARTUPINFO`:** Параметры запуска (окно, потоки ввода/вывода)
- **`PROCESS_INFORMATION`:** Результат создания (дескрипторы, идентификаторы)
- **Флаги создания:** `CREATE_SUSPENDED`, `CREATE_NEW_CONSOLE`, `CREATE_UNICODE_ENVIRONMENT`

#### **3. Управление процессами:**
- **`WaitForSingleObject()`:** Ожидание завершения процесса
- **`GetExitCodeProcess()`:** Получение кода завершения
- **`TerminateProcess()`:** Принудительное завершение
- **`GetCurrentProcessId()`:** Получение PID текущего процесса

#### **4. Информация о процессах:**
- **ToolHelp API:** `CreateToolhelp32Snapshot()`, `Process32First()`, `Process32Next()`
- **Структуры:** `PROCESSENTRY32` — информация о процессе
- **PSAPI:** Альтернативный API для работы с процессами

#### **5. Окружение процессов:**
- **Блок окружения:** Набор строк "ИМЯ=ЗНАЧЕНИЕ"
- **`GetEnvironmentVariable()` / `SetEnvironmentVariable()`:** Работа с переменными
- **Наследование:** Дочерние процессы получают копию окружения родителя

---

## 📝 **ЗАДАНИЕ**

### **Часть 1: Теория (ответьте письменно)**
1. Чем отличается PID процесса от его дескриптора? Почему дескриптор надежнее для долгоживущих приложений?
2. Что произойдет, если не закрыть дескрипторы `hProcess` и `hThread`, полученные из `CreateProcess()`?
3. Почему для перечисления процессов используется `CreateToolhelp32Snapshot()` вместо прямого доступа?
4. Как переменные окружения передаются дочерним процессам и можно ли это изменить?
5. Что такое PPL (Protected Process Light) и зачем нужен этот механизм?

### **Часть 2: Практическая работа**

**Задача:** Создать утилиту для управления процессами Windows

**Требования к программе:**
1. Создание процессов с различными параметрами
2. Мониторинг и перечисление запущенных процессов
3. Работа с переменными окружения
4. Демонстрация разных способов завершения процессов
5. Поиск процессов по имени и фильтрация

---

## 💻 **ПОЛНЫЙ ИСХОДНЫЙ КОД ПРОГРАММЫ**

```c
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <tchar.h>

// Прототипы функций
void PrintMenu();
void CreateSimpleProcess();
void CreateProcessWithRedirect();
void ListAllProcesses();
void FindProcessByName();
void ManageProcessEnvironment();
void TerminateProcessDemo();
void ProcessInfoDemo();

// Вспомогательные функции
void PrintError(const char* functionName, DWORD errorCode);
void PrintProcessInfo(PROCESSENTRY32* pe);
void WaitForProcessWithTimeout(HANDLE hProcess, DWORD timeoutMs);

// Главная функция
int main()
{
    // Настройка консоли для русского языка
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    _tprintf(_T("=== Утилита управления процессами Windows ===\n\n"));
    
    int choice = 0;
    TCHAR input[10];
    
    while (1)
    {
        PrintMenu();
        _tprintf(_T("Выберите действие (1-8): "));
        
        if (_fgetts(input, sizeof(input) / sizeof(TCHAR), stdin) != NULL)
        {
            choice = _ttoi(input);
        }
        
        switch (choice)
        {
        case 1:
            CreateSimpleProcess();
            break;
        case 2:
            CreateProcessWithRedirect();
            break;
        case 3:
            ListAllProcesses();
            break;
        case 4:
            FindProcessByName();
            break;
        case 5:
            ManageProcessEnvironment();
            break;
        case 6:
            TerminateProcessDemo();
            break;
        case 7:
            ProcessInfoDemo();
            break;
        case 8:
            _tprintf(_T("Выход из программы.\n"));
            return 0;
        default:
            _tprintf(_T("Неверный выбор. Попробуйте снова.\n"));
            break;
        }
        
        if (choice != 8)
        {
            _tprintf(_T("\nНажмите Enter для продолжения..."));
            int ch;
            while ((ch = _gettchar()) != _T('\n') && ch != EOF);
        }
    }
    
    return 0;
}

// Функция 1: Простое создание процесса
void CreateSimpleProcess()
{
    _tprintf(_T("\n=== СОЗДАНИЕ ПРОСТОГО ПРОЦЕССА ===\n\n"));
    
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    // Запрашиваем у пользователя программу для запуска
    _tprintf(_T("Введите имя программы для запуска (например, notepad.exe): "));
    TCHAR cmdLine[MAX_PATH];
    
    if (_fgetts(cmdLine, MAX_PATH, stdin) == NULL)
    {
        _tprintf(_T("Ошибка ввода.\n"));
        return;
    }
    
    // Убираем символ новой строки
    size_t len = _tcslen(cmdLine);
    if (len > 0 && cmdLine[len - 1] == _T('\n'))
    {
        cmdLine[len - 1] = _T('\0');
    }
    
    // Если строка пустая, используем notepad.exe по умолчанию
    if (_tcslen(cmdLine) == 0)
    {
        _tcscpy_s(cmdLine, MAX_PATH, _T("notepad.exe"));
        _tprintf(_T("Используется программа по умолчанию: %s\n"), cmdLine);
    }
    
    _tprintf(_T("\nЗапускаем процесс: %s\n"), cmdLine);
    _tprintf(_T("1. Используем обычный приоритет\n"));
    _tprintf(_T("2. Создаем новое консольное окно\n"));
    _tprintf(_T("3. Используем окружение родительского процесса\n\n"));
    
    // Создаем процесс
    BOOL success = CreateProcess(
        NULL,                   // Имя приложения (указываем в командной строке)
        cmdLine,                // Командная строка
        NULL,                   // Атрибуты безопасности процесса
        NULL,                   // Атрибуты безопасности потока
        FALSE,                  // Наследование дескрипторов
        CREATE_NEW_CONSOLE,     // Флаги создания
        NULL,                   // Окружение (используем родительское)
        NULL,                   // Текущий каталог
        &si,                    // STARTUPINFO
        &pi                     // PROCESS_INFORMATION
    );
    
    if (!success)
    {
        PrintError("CreateProcess", GetLastError());
        return;
    }
    
    _tprintf(_T("✓ Процесс успешно создан!\n"));
    _tprintf(_T("   PID процесса: %lu\n"), pi.dwProcessId);
    _tprintf(_T("   PID основного потока: %lu\n"), pi.dwThreadId);
    _tprintf(_T("   Дескриптор процесса: 0x%p\n"), pi.hProcess);
    
    // Даем пользователю выбор: ждать завершения или нет
    _tprintf(_T("\nВыберите действие:\n"));
    _tprintf(_T("1. Ждать завершения процесса\n"));
    _tprintf(_T("2. Запустить и продолжить (не ждать)\n"));
    _tprintf(_T("Ваш выбор: "));
    
    TCHAR choice[10];
    _fgetts(choice, sizeof(choice) / sizeof(TCHAR), stdin);
    
    if (_ttoi(choice) == 1)
    {
        _tprintf(_T("\nОжидаем завершения процесса...\n"));
        WaitForProcessWithTimeout(pi.hProcess, 30000); // 30 секунд таймаут
    }
    else
    {
        _tprintf(_T("\nПроцесс запущен в фоновом режиме.\n"));
        _tprintf(_T("Для завершения можно использовать наш монитор процессов.\n"));
    }
    
    // Закрываем дескрипторы (важно всегда это делать!)
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    _tprintf(_T("\nДескрипторы закрыты.\n"));
}

// Функция 2: Создание процесса с перенаправлением вывода
void CreateProcessWithRedirect()
{
    _tprintf(_T("\n=== СОЗДАНИЕ ПРОЦЕССА С ПЕРЕНАПРАВЛЕНИЕМ ВЫВОДА ===\n\n"));
    
    _tprintf(_T("Этот пример запускает cmd.exe с перенаправлением вывода в файл.\n"));
    
    // Создаем файл для вывода
    HANDLE hOutputFile = CreateFile(
        _T("process_output.txt"),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hOutputFile == INVALID_HANDLE_VALUE)
    {
        PrintError("CreateFile", GetLastError());
        return;
    }
    
    // Настраиваем STARTUPINFO для перенаправления
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hOutputFile;     // Перенаправляем stdout в файл
    si.hStdError = hOutputFile;      // Перенаправляем stderr в тот же файл
    
    // Команда для выполнения
    TCHAR cmdLine[] = _T("cmd.exe /c dir & echo Hello from redirected process!");
    
    _tprintf(_T("Запускаем команду: %s\n"), cmdLine);
    _tprintf(_T("Вывод будет сохранен в process_output.txt\n\n"));
    
    // Создаем процесс
    BOOL success = CreateProcess(
        NULL,
        cmdLine,
        NULL,
        NULL,
        TRUE,                   // TRUE = дескрипторы наследуются
        CREATE_NO_WINDOW,       // Не создавать окно
        NULL,
        NULL,
        &si,
        &pi
    );
    
    if (!success)
    {
        PrintError("CreateProcess", GetLastError());
        CloseHandle(hOutputFile);
        return;
    }
    
    _tprintf(_T("Процесс создан. PID: %lu\n"), pi.dwProcessId);
    
    // Ждем завершения
    WaitForSingleObject(pi.hProcess, 5000);
    
    // Получаем код завершения
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode))
    {
        _tprintf(_T("Код завершения: %lu\n"), exitCode);
    }
    
    // Закрываем дескрипторы
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hOutputFile);
    
    _tprintf(_T("\nВывод сохранен в process_output.txt\n"));
    _tprintf(_T("Содержимое файла:\n"));
    _tprintf(_T("================\n"));
    
    // Читаем и выводим содержимое файла
    FILE* file = _tfopen(_T("process_output.txt"), _T("r"));
    if (file)
    {
        TCHAR buffer[256];
        while (_fgetts(buffer, sizeof(buffer) / sizeof(TCHAR), file))
        {
            _tprintf(_T("%s"), buffer);
        }
        fclose(file);
    }
}

// Функция 3: Перечисление всех процессов
void ListAllProcesses()
{
    _tprintf(_T("\n=== СПИСОК ВСЕХ ПРОЦЕССОВ ===\n\n"));
    
    // Создаем снимок процессов
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintError("CreateToolhelp32Snapshot", GetLastError());
        return;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    _tprintf(_T("%-8s  %-40s  %-8s  %-8s\n"), 
             _T("PID"), _T("Имя процесса"), _T("Потоков"), _T("Родитель"));
    _tprintf(_T("--------------------------------------------------------------------------------\n"));
    
    // Получаем первый процесс
    if (Process32First(hSnapshot, &pe32))
    {
        int count = 0;
        do
        {
            PrintProcessInfo(&pe32);
            count++;
        } while (Process32Next(hSnapshot, &pe32));
        
        _tprintf(_T("\nВсего процессов: %d\n"), count);
    }
    else
    {
        PrintError("Process32First", GetLastError());
    }
    
    CloseHandle(hSnapshot);
}

// Функция 4: Поиск процесса по имени
void FindProcessByName()
{
    _tprintf(_T("\n=== ПОИСК ПРОЦЕССА ПО ИМЕНИ ===\n\n"));
    
    _tprintf(_T("Введите имя процесса для поиска (например, explorer.exe): "));
    TCHAR searchName[MAX_PATH];
    
    if (_fgetts(searchName, MAX_PATH, stdin) == NULL)
    {
        _tprintf(_T("Ошибка ввода.\n"));
        return;
    }
    
    // Убираем символ новой строки
    size_t len = _tcslen(searchName);
    if (len > 0 && searchName[len - 1] == _T('\n'))
    {
        searchName[len - 1] = _T('\0');
    }
    
    if (_tcslen(searchName) == 0)
    {
        _tcscpy_s(searchName, MAX_PATH, _T("explorer.exe"));
    }
    
    _tprintf(_T("\nИщем процессы содержащие: %s\n\n"), searchName);
    
    // Создаем снимок процессов
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        PrintError("CreateToolhelp32Snapshot", GetLastError());
        return;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    int foundCount = 0;
    
    if (Process32First(hSnapshot, &pe32))
    {
        _tprintf(_T("%-8s  %-40s  %-8s  %-8s\n"), 
                 _T("PID"), _T("Имя процесса"), _T("Потоков"), _T("Родитель"));
        _tprintf(_T("--------------------------------------------------------------------------------\n"));
        
        do
        {
            // Поиск по частичному совпадению (без учета регистра)
            if (_tcsstr(_tcslwr(_tcsdup(pe32.szExeFile)), _tcslwr(_tcsdup(searchName))) != NULL)
            {
                PrintProcessInfo(&pe32);
                foundCount++;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    if (foundCount == 0)
    {
        _tprintf(_T("Процессы с именем '%s' не найдены.\n"), searchName);
    }
    else
    {
        _tprintf(_T("\nНайдено процессов: %d\n"), foundCount);
    }
    
    CloseHandle(hSnapshot);
}

// Функция 5: Управление переменными окружения
void ManageProcessEnvironment()
{
    _tprintf(_T("\n=== УПРАВЛЕНИЕ ПЕРЕМЕННЫМИ ОКРУЖЕНИЯ ===\n\n"));
    
    _tprintf(_T("1. Показать все переменные окружения\n"));
    _tprintf(_T("2. Получить значение конкретной переменной\n"));
    _tprintf(_T("3. Установить новую переменную\n"));
    _tprintf(_T("4. Удалить переменную\n"));
    _tprintf(_T("Ваш выбор: "));
    
    TCHAR choice[10];
    _fgetts(choice, sizeof(choice) / sizeof(TCHAR), stdin);
    
    switch (_ttoi(choice))
    {
    case 1:
        {
            _tprintf(_T("\n=== ВСЕ ПЕРЕМЕННЫЕ ОКРУЖЕНИЯ ===\n\n"));
            
            // Получаем блок переменных окружения
            LPTCH env = GetEnvironmentStrings();
            
            if (env == NULL)
            {
                PrintError("GetEnvironmentStrings", GetLastError());
                return;
            }
            
            // Переменные представлены как последовательность строк
            // Каждая строка: VAR=value\0, блок заканчивается двойным \0\0
            int count = 0;
            for (LPTCH var = env; *var; var += _tcslen(var) + 1)
            {
                _tprintf(_T("%s\n"), var);
                count++;
            }
            
            FreeEnvironmentStrings(env);
            _tprintf(_T("\nВсего переменных: %d\n"), count);
        }
        break;
        
    case 2:
        {
            _tprintf(_T("\nВведите имя переменной: "));
            TCHAR varName[MAX_PATH];
            _fgetts(varName, MAX_PATH, stdin);
            
            size_t len = _tcslen(varName);
            if (len > 0 && varName[len - 1] == _T('\n'))
            {
                varName[len - 1] = _T('\0');
            }
            
            TCHAR buffer[4096];
            DWORD result = GetEnvironmentVariable(varName, buffer, sizeof(buffer) / sizeof(TCHAR));
            
            if (result == 0)
            {
                DWORD error = GetLastError();
                if (error == ERROR_ENVVAR_NOT_FOUND)
                {
                    _tprintf(_T("Переменная '%s' не найдена.\n"), varName);
                }
                else
                {
                    PrintError("GetEnvironmentVariable", error);
                }
            }
            else if (result > sizeof(buffer) / sizeof(TCHAR))
            {
                _tprintf(_T("Значение переменной слишком длинное (%lu символов).\n"), result);
            }
            else
            {
                _tprintf(_T("%s = %s\n"), varName, buffer);
            }
        }
        break;
        
    case 3:
        {
            _tprintf(_T("\nВведите имя переменной: "));
            TCHAR varName[MAX_PATH];
            _fgetts(varName, MAX_PATH, stdin);
            
            size_t len = _tcslen(varName);
            if (len > 0 && varName[len - 1] == _T('\n'))
            {
                varName[len - 1] = _T('\0');
            }
            
            _tprintf(_T("Введите значение: "));
            TCHAR varValue[MAX_PATH];
            _fgetts(varValue, MAX_PATH, stdin);
            
            len = _tcslen(varValue);
            if (len > 0 && varValue[len - 1] == _T('\n'))
            {
                varValue[len - 1] = _T('\0');
            }
            
            if (SetEnvironmentVariable(varName, varValue))
            {
                _tprintf(_T("Переменная '%s' установлена в '%s'\n"), varName, varValue);
                _tprintf(_T("Примечание: переменная доступна только этому процессу и его дочерним процессам.\n"));
            }
            else
            {
                PrintError("SetEnvironmentVariable", GetLastError());
            }
        }
        break;
        
    case 4:
        {
            _tprintf(_T("\nВведите имя переменной для удаления: "));
            TCHAR varName[MAX_PATH];
            _fgetts(varName, MAX_PATH, stdin);
            
            size_t len = _tcslen(varName);
            if (len > 0 && varName[len - 1] == _T('\n'))
            {
                varName[len - 1] = _T('\0');
            }
            
            if (SetEnvironmentVariable(varName, NULL))
            {
                _tprintf(_T("Переменная '%s' удалена.\n"), varName);
            }
            else
            {
                PrintError("SetEnvironmentVariable", GetLastError());
            }
        }
        break;
        
    default:
        _tprintf(_T("Неверный выбор.\n"));
        break;
    }
}

// Функция 6: Демонстрация завершения процесса
void TerminateProcessDemo()
{
    _tprintf(_T("\n=== ДЕМОНСТРАЦИЯ ЗАВЕРШЕНИЯ ПРОЦЕССА ===\n\n"));
    
    _tprintf(_T("Внимание: эта функция позволяет завершить любой процесс!\n"));
    _tprintf(_T("Используйте осторожно.\n\n"));
    
    _tprintf(_T("Введите PID процесса для завершения (0 - отмена): "));
    
    TCHAR pidStr[20];
    _fgetts(pidStr, sizeof(pidStr) / sizeof(TCHAR), stdin);
    
    DWORD pid = _ttoi(pidStr);
    
    if (pid == 0)
    {
        _tprintf(_T("Отменено.\n"));
        return;
    }
    
    // Пытаемся открыть процесс
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pid);
    
    if (hProcess == NULL)
    {
        DWORD error = GetLastError();
        
        if (error == ERROR_ACCESS_DENIED)
        {
            _tprintf(_T("Доступ запрещен. Возможные причины:\n"));
            _tprintf(_T("1. Процесс с PID %lu не существует\n"), pid);
            _tprintf(_T("2. У вас недостаточно прав (админ?)\n"));
            _tprintf(_T("3. Это системный или PPL процесс\n"));
            
            // Пробуем открыть с меньшими правами
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
            if (hProcess != NULL)
            {
                _tprintf(_T("Процесс существует, но не можем завершить.\n"));
                CloseHandle(hProcess);
            }
        }
        else
        {
            PrintError("OpenProcess", error);
        }
        return;
    }
    
    // Получаем информацию о процессе перед завершением
    TCHAR processName[MAX_PATH] = _T("неизвестно");
    DWORD size = MAX_PATH;
    QueryFullProcessImageName(hProcess, 0, processName, &size);
    
    _tprintf(_T("Процесс для завершения:\n"));
    _tprintf(_T("  PID: %lu\n"), pid);
    _tprintf(_T("  Имя: %s\n"), processName);
    
    _tprintf(_T("\nВы уверены что хотите завершить этот процесс? (y/N): "));
    
    TCHAR confirm[10];
    _fgetts(confirm, sizeof(confirm) / sizeof(TCHAR), stdin);
    
    if (_totlower(confirm[0]) == _T('y'))
    {
        if (TerminateProcess(hProcess, 0))
        {
            _tprintf(_T("Процесс завершен принудительно.\n"));
        }
        else
        {
            PrintError("TerminateProcess", GetLastError());
        }
    }
    else
    {
        _tprintf(_T("Отменено.\n"));
    }
    
    CloseHandle(hProcess);
}

// Функция 7: Информация о текущем процессе
void ProcessInfoDemo()
{
    _tprintf(_T("\n=== ИНФОРМАЦИЯ О ТЕКУЩЕМ ПРОЦЕССЕ ===\n\n"));
    
    // PID текущего процесса
    DWORD pid = GetCurrentProcessId();
    _tprintf(_T("PID текущего процесса: %lu\n"), pid);
    
    // Дескриптор текущего процесса
    HANDLE hProcess = GetCurrentProcess();
    _tprintf(_T("Псевдодескриптор текущего процесса: 0x%p\n"), hProcess);
    
    // Приоритет процесса
    DWORD priority = GetPriorityClass(hProcess);
    _tprintf(_T("Класс приоритета: "));
    
    switch (priority)
    {
    case NORMAL_PRIORITY_CLASS:
        _tprintf(_T("NORMAL_PRIORITY_CLASS\n"));
        break;
    case HIGH_PRIORITY_CLASS:
        _tprintf(_T("HIGH_PRIORITY_CLASS\n"));
        break;
    case REALTIME_PRIORITY_CLASS:
        _tprintf(_T("REALTIME_PRIORITY_CLASS\n"));
        break;
    case IDLE_PRIORITY_CLASS:
        _tprintf(_T("IDLE_PRIORITY_CLASS\n"));
        break;
    case BELOW_NORMAL_PRIORITY_CLASS:
        _tprintf(_T("BELOW_NORMAL_PRIORITY_CLASS\n"));
        break;
    case ABOVE_NORMAL_PRIORITY_CLASS:
        _tprintf(_T("ABOVE_NORMAL_PRIORITY_CLASS\n"));
        break;
    default:
        _tprintf(_T("0x%08lX\n"), priority);
        break;
    }
    
    // Время работы процесса
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        SYSTEMTIME sysTime;
        FileTimeToSystemTime(&creationTime, &sysTime);
        
        _tprintf(_T("Время создания: %02d:%02d:%02d\n"), 
                 sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
        
        // Преобразуем время в миллисекунды
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
        
        _tprintf(_T("Время в ядре: %llu мс\n"), kernel.QuadPart / 10000);
        _tprintf(_T("Время в пользовательском режиме: %llu мс\n"), user.QuadPart / 10000);
    }
    
    // Информация о памяти
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        _tprintf(_T("\nИспользование памяти:\n"));
        _tprintf(_T("  Пиковое использование: %lu КБ\n"), pmc.PeakWorkingSetSize / 1024);
        _tprintf(_T("  Текущее использование: %lu КБ\n"), pmc.WorkingSetSize / 1024);
        _tprintf(_T("  Пиковый размер страничного файла: %lu КБ\n"), pmc.PeakPagefileUsage / 1024);
        _tprintf(_T("  Текущий размер страничного файла: %lu КБ\n"), pmc.PagefileUsage / 1024);
    }
    
    // Не закрываем hProcess - это псевдодескриптор
}

// Вспомогательная функция: печать информации о процессе
void PrintProcessInfo(PROCESSENTRY32* pe)
{
    _tprintf(_T("%-8lu  %-40s  %-8lu  %-8lu\n"), 
             pe->th32ProcessID, 
             pe->szExeFile, 
             pe->cntThreads, 
             pe->th32ParentProcessID);
}

// Вспомогательная функция: ожидание процесса с таймаутом
void WaitForProcessWithTimeout(HANDLE hProcess, DWORD timeoutMs)
{
    DWORD waitResult = WaitForSingleObject(hProcess, timeoutMs);
    
    switch (waitResult)
    {
    case WAIT_OBJECT_0:
        {
            DWORD exitCode;
            if (GetExitCodeProcess(hProcess, &exitCode))
            {
                _tprintf(_T("Процесс завершился с кодом: %lu\n"), exitCode);
            }
        }
        break;
        
    case WAIT_TIMEOUT:
        _tprintf(_T("Таймаут! Процесс все еще выполняется.\n"));
        _tprintf(_T("Хотите завершить процесс? (y/N): "));
        
        TCHAR choice[10];
        _fgetts(choice, sizeof(choice) / sizeof(TCHAR), stdin);
        
        if (_totlower(choice[0]) == _T('y'))
        {
            if (TerminateProcess(hProcess, 1))
            {
                _tprintf(_T("Процесс завершен принудительно.\n"));
            }
            else
            {
                PrintError("TerminateProcess", GetLastError());
            }
        }
        break;
        
    case WAIT_FAILED:
        PrintError("WaitForSingleObject", GetLastError());
        break;
    }
}

// Вспомогательная функция: вывод ошибки
void PrintError(const char* functionName, DWORD errorCode)
{
    LPTSTR errorMsg = NULL;
    
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&errorMsg,
        0,
        NULL
    );
    
    if (errorMsg != NULL)
    {
        // Конвертируем в многобайтовую строку для вывода
        size_t converted = 0;
        char mbErrorMsg[512];
        wcstombs_s(&converted, mbErrorMsg, errorMsg, sizeof(mbErrorMsg));
        
        printf("Ошибка в функции %s (код %lu): %s", functionName, errorCode, mbErrorMsg);
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
    _tprintf(_T("\n=== ГЛАВНОЕ МЕНЮ ===\n\n"));
    
    _tprintf(_T("1. Создать простой процесс\n"));
    _tprintf(_T("2. Создать процесс с перенаправлением вывода\n"));
    _tprintf(_T("3. Показать все процессы\n"));
    _tprintf(_T("4. Найти процесс по имени\n"));
    _tprintf(_T("5. Управление переменными окружения\n"));
    _tprintf(_T("6. Завершить процесс\n"));
    _tprintf(_T("7. Информация о текущем процессе\n"));
    _tprintf(_T("8. Выход\n\n"));
}
```

---

## 🔧 **КОМПИЛЯЦИЯ И ЗАПУСК:**

### **В Visual Studio:**
1. Создайте **консольное приложение**
2. Включите опцию **"Character Set: Use Unicode Character Set"**
3. Добавьте библиотеки: `kernel32.lib`, `user32.lib`, `psapi.lib`
4. Вставьте код в `main.c`
5. Запустите от имени администратора для полного доступа

### **Командная строка:**
```bash
cl lesson3.c /link kernel32.lib user32.lib psapi.lib /D UNICODE /D _UNICODE
```

---

## 📋 **АНАЛИЗ ПРОГРАММЫ:**

1. Почему в функции `TerminateProcessDemo()` используется две попытки вызова `OpenProcess()`?
2. Что произойдет, если попытаться завершить системный процесс (например, `csrss.exe`)?
3. Почему дескриптор из `GetCurrentProcess()` не нужно закрывать?
4. Какой способ безопаснее для завершения процесса: `TerminateProcess()` или отправка сообщения WM_CLOSE?
5. Почему переменные окружения, установленные через `SetEnvironmentVariable()`, не видны другим уже запущенным процессам?

---

## 🎯 **ПРАКТИЧЕСКИЕ ЗАДАНИЯ ДЛЯ САМОСТОЯТЕЛЬНОЙ РАБОТЫ:**

### **Уровень 1: Модификация кода**
1. Добавьте в `ListAllProcesses()` сортировку процессов по PID или имени
2. Расширьте `FindProcessByName()` поиском по пути к исполняемому файлу
3. Добавьте в `CreateSimpleProcess()` возможность указания приоритета процесса

### **Уровень 2: Новые функции**
1. Реализуйте функцию "Дерево процессов" — отображение иерархии процессов
2. Создайте функцию для запуска процесса с правами другого пользователя (используйте `CreateProcessWithLogonW`)
3. Добавьте монитор использования ресурсов (память, CPU) для выбранного процесса

### **Уровень 3: Продвинутые задачи**
1. Реализуйте простой отладчик процессов — приостановку, возобновление потоков
2. Создайте
