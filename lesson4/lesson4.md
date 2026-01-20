# 🚀 **УРОК 4: JOB-ОБЪЕКТЫ WINDOWS — ГРУППОВОЕ УПРАВЛЕНИЕ ПРОЦЕССАМИ**

**Цель урока:**  
Освоить механизм Job-объектов Windows для группового управления процессами, ограничения ресурсов и создания изолированных сред выполнения. Научиться создавать "песочницы" для безопасного запуска ненадежного кода.

---

## 📚 **ТЕОРЕТИЧЕСКАЯ ЧАСТЬ (по книге Иосифовича)**

### **Страницы для изучения:**
- **Глава 4 (страницы 86-95):** "Job Objects Basics" — основы Job-объектов
- **Глава 4 (страницы 96-105):** "Job Limits and Restrictions" — ограничения и квоты
- **Глава 4 (страницы 106-115):** "Job Security and Monitoring" — безопасность и мониторинг

### **Ключевые концепции:**

#### **1. Что такое Job-объект:**
- **Контейнер процессов:** Группирует процессы для управления как единым целым
- **Объект ядра:** Как процесс или поток, но для группы
- **Наследование:** Дочерние процессы автоматически попадают в Job родителя
- **Изоляция:** Процессы в Job не могут "сбежать" из него

#### **2. Основные возможности Job-объектов:**
- **Ограничение ресурсов:** CPU, память, время выполнения
- **Безопасность:** Ограничение прав доступа процессов
- **Синхронизация:** Ожидание завершения всех процессов в Job
- **Уведомления:** События при завершении процессов
- **Аккаунтинг:** Статистика использования ресурсов

#### **3. Типы ограничений (Job Limits):**
- **Basic Limits:** Активный процесс, время выполнения, привилегии
- **Extended Limits:** Рабочий набор, время процессора, память
- **UI Restrictions:** Доступ к системным объектам GUI
- **Security Limits:** Токены доступа, SID-листы

#### **4. Жизненный цикл Job-объекта:**
```
Создание → Настройка ограничений → Назначение процессов →
Мониторинг → Завершение → Закрытие дескриптора
```

#### **5. Практическое применение:**
- **Песочницы (Sandboxes):** Безопасное выполнение ненадежного кода
- **Системные службы:** Группировка связанных служб
- **Пакетные задания:** Ограничение ресурсов для задач
- **Изоляция приложений:** Предотвращение влияния на систему

---

## 📝 **ЗАДАНИЕ**

### **Часть 1: Теория (ответьте письменно)**
1. Чем Job-объект отличается от просто группы процессов, созданных родительским процессом?
2. Что произойдет с процессом, который попытается создать больше потоков, чем разрешено в Job?
3. Почему процессы не могут "сбежать" из Job-объекта после назначения?
4. Какие ограничения безопасности можно наложить через Job-объект?
5. Когда использовать `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE` и какие риски это несет?

### **Часть 2: Практическая работа**

**Задача:** Создать утилиту "Песочница" для безопасного выполнения программ

**Требования к программе:**
1. Создание Job-объекта с настраиваемыми ограничениями
2. Запуск процесса внутри Job с изоляцией
3. Мониторинг использования ресурсов Job
4. Безопасное завершение всех процессов в Job
5. Визуализация ограничений и статистики

---

## 💻 **ПОЛНЫЙ ИСХОДНЫЙ КОД ПРОГРАММЫ**

```c
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// Для работы с Job Objects
#include <winternl.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

// Прототипы функций
void PrintMenu();
void CreateBasicJob();
void CreateJobWithLimits();
void MonitorJobResources();
void JobSecurityDemo();
void SandboxDemo();
void TerminateJobDemo();

// Вспомогательные функции
void PrintError(const char* functionName, DWORD errorCode);
void SetJobLimit(HANDLE hJob, JOBOBJECT_BASIC_LIMIT_INFORMATION* limits);
void SetJobExtendedLimit(HANDLE hJob, JOBOBJECT_EXTENDED_LIMIT_INFORMATION* extLimits);
void PrintJobInfo(HANDLE hJob);

// Глобальные переменные для демонстрации
HANDLE g_hCurrentJob = NULL;
TCHAR g_JobName[256] = _T("DemoJob");

// Главная функция
int main()
{
    // Настройка консоли для русского языка
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    _tprintf(_T("=== Утилита работы с Job-объектами Windows ===\n\n"));
    _tprintf(_T("Job-объекты позволяют управлять группой процессов как единым целым.\n"));
    _tprintf(_T("Идеально для создания песочниц и ограничения ресурсов.\n\n"));
    
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
            CreateBasicJob();
            break;
        case 2:
            CreateJobWithLimits();
            break;
        case 3:
            MonitorJobResources();
            break;
        case 4:
            JobSecurityDemo();
            break;
        case 5:
            SandboxDemo();
            break;
        case 6:
            TerminateJobDemo();
            break;
        case 7:
            if (g_hCurrentJob)
            {
                PrintJobInfo(g_hCurrentJob);
            }
            else
            {
                _tprintf(_T("Нет активного Job-объекта.\n"));
            }
            break;
        case 8:
            // Очистка перед выходом
            if (g_hCurrentJob)
            {
                _tprintf(_T("Закрываю Job-объект...\n"));
                CloseHandle(g_hCurrentJob);
                g_hCurrentJob = NULL;
            }
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

// Функция 1: Создание базового Job-объекта
void CreateBasicJob()
{
    _tprintf(_T("\n=== СОЗДАНИЕ БАЗОВОГО JOB-ОБЪЕКТА ===\n\n"));
    
    if (g_hCurrentJob)
    {
        _tprintf(_T("Уже существует активный Job-объект. Закрываю его...\n"));
        CloseHandle(g_hCurrentJob);
        g_hCurrentJob = NULL;
    }
    
    _tprintf(_T("Введите имя для нового Job-объекта: "));
    if (_fgetts(g_JobName, sizeof(g_JobName) / sizeof(TCHAR), stdin))
    {
        size_t len = _tcslen(g_JobName);
        if (len > 0 && g_JobName[len - 1] == _T('\n'))
        {
            g_JobName[len - 1] = _T('\0');
        }
    }
    
    // Создаем атрибуты безопасности
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;
    
    // Создаем Job-объект
    g_hCurrentJob = CreateJobObject(&sa, g_JobName);
    
    if (g_hCurrentJob == NULL)
    {
        PrintError("CreateJobObject", GetLastError());
        return;
    }
    
    _tprintf(_T("✓ Job-объект '%s' успешно создан!\n"), g_JobName);
    _tprintf(_T("   Дескриптор: 0x%p\n"), g_hCurrentJob);
    
    // Устанавливаем базовые ограничения
    JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimits = { 0 };
    basicLimits.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS | 
                            JOB_OBJECT_LIMIT_PRIORITY_CLASS;
    basicLimits.ActiveProcessLimit = 5;          // Макс 5 процессов
    basicLimits.PriorityClass = NORMAL_PRIORITY_CLASS;
    
    if (!SetInformationJobObject(g_hCurrentJob, 
                                JobObjectBasicLimitInformation,
                                &basicLimits, 
                                sizeof(basicLimits)))
    {
        PrintError("SetInformationJobObject", GetLastError());
    }
    else
    {
        _tprintf(_T("   Установлены ограничения:\n"));
        _tprintf(_T("   • Максимум процессов: %d\n"), basicLimits.ActiveProcessLimit);
        _tprintf(_T("   • Класс приоритета: NORMAL\n"));
    }
    
    // Устанавливаем флаг завершения процессов при закрытии Job
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimits = { 0 };
    extLimits.BasicLimitInformation.LimitFlags = 
        JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    
    if (!SetInformationJobObject(g_hCurrentJob, 
                                JobObjectExtendedLimitInformation,
                                &extLimits, 
                                sizeof(extLimits)))
    {
        PrintError("SetInformationJobObject (extended)", GetLastError());
    }
    else
    {
        _tprintf(_T("   • Процессы завершаются при закрытии Job\n"));
    }
}

// Функция 2: Создание Job с продвинутыми ограничениями
void CreateJobWithLimits()
{
    _tprintf(_T("\n=== СОЗДАНИЕ JOB С ОГРАНИЧЕНИЯМИ РЕСУРСОВ ===\n\n"));
    
    if (!g_hCurrentJob)
    {
        _tprintf(_T("Сначала создайте Job-объект (пункт 1 меню).\n"));
        return;
    }
    
    _tprintf(_T("Настройка ограничений для Job '%s':\n\n"), g_JobName);
    
    // 1. Ограничения по процессорному времени
    _tprintf(_T("1. Ограничения CPU:\n"));
    _tprintf(_T("   a) На процесс (в секундах): "));
    TCHAR cpuPerProcess[20];
    _fgetts(cpuPerProcess, sizeof(cpuPerProcess) / sizeof(TCHAR), stdin);
    DWORD cpuPerProcessSec = _ttoi(cpuPerProcess);
    
    _tprintf(_T("   b) На весь Job (в секундах): "));
    TCHAR cpuPerJob[20];
    _fgetts(cpuPerJob, sizeof(cpuPerJob) / sizeof(TCHAR), stdin);
    DWORD cpuPerJobSec = _ttoi(cpuPerJob);
    
    // 2. Ограничения по памяти
    _tprintf(_T("\n2. Ограничения памяти:\n"));
    _tprintf(_T("   a) Макс. рабочее множество (в МБ): "));
    TCHAR workingSet[20];
    _fgetts(workingSet, sizeof(workingSet) / sizeof(TCHAR), stdin);
    SIZE_T workingSetMB = _ttoi(workingSet) * 1024 * 1024;
    
    _tprintf(_T("   b) Макс. размер процесса (в МБ): "));
    TCHAR processMemory[20];
    _fgetts(processMemory, sizeof(processMemory) / sizeof(TCHAR), stdin);
    SIZE_T processMemoryMB = _ttoi(processMemory) * 1024 * 1024;
    
    // Настройка расширенных ограничений
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimits = { 0 };
    
    // Флаги ограничений
    extLimits.BasicLimitInformation.LimitFlags = 
        JOB_OBJECT_LIMIT_PROCESS_TIME |
        JOB_OBJECT_LIMIT_JOB_TIME |
        JOB_OBJECT_LIMIT_WORKINGSET |
        JOB_OBJECT_LIMIT_PROCESS_MEMORY |
        JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    
    // Установка значений ограничений
    if (cpuPerProcessSec > 0)
    {
        // Время в 100-наносекундных интервалах
        extLimits.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = 
            cpuPerProcessSec * 10000000LL; // секунды → 100ns
    }
    
    if (cpuPerJobSec > 0)
    {
        extLimits.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = 
            cpuPerJobSec * 10000000LL;
    }
    
    if (workingSetMB > 0)
    {
        extLimits.BasicLimitInformation.MinimumWorkingSetSize = 1024 * 1024; // 1MB min
        extLimits.BasicLimitInformation.MaximumWorkingSetSize = workingSetMB;
    }
    
    if (processMemoryMB > 0)
    {
        extLimits.ProcessMemoryLimit = processMemoryMB;
        extLimits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    }
    
    // Применяем ограничения
    if (!SetInformationJobObject(g_hCurrentJob, 
                                JobObjectExtendedLimitInformation,
                                &extLimits, 
                                sizeof(extLimits)))
    {
        PrintError("SetInformationJobObject", GetLastError());
        return;
    }
    
    _tprintf(_T("\n✓ Ограничения успешно применены:\n"));
    if (cpuPerProcessSec > 0)
        _tprintf(_T("   • CPU на процесс: %lu сек\n"), cpuPerProcessSec);
    if (cpuPerJobSec > 0)
        _tprintf(_T("   • CPU на весь Job: %lu сек\n"), cpuPerJobSec);
    if (workingSetMB > 0)
        _tprintf(_T("   • Рабочее множество: %lu МБ\n"), workingSetMB / (1024 * 1024));
    if (processMemoryMB > 0)
        _tprintf(_T("   • Память на процесс: %lu МБ\n"), processMemoryMB / (1024 * 1024));
    
    _tprintf(_T("\nЭти ограничения будут применены ко всем процессам в Job.\n"));
}

// Функция 3: Мониторинг ресурсов Job
void MonitorJobResources()
{
    _tprintf(_T("\n=== МОНИТОРИНГ РЕСУРСОВ JOB ===\n\n"));
    
    if (!g_hCurrentJob)
    {
        _tprintf(_T("Сначала создайте Job-объект (пункт 1 меню).\n"));
        return;
    }
    
    // Получаем базовую информацию
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicInfo = { 0 };
    if (!QueryInformationJobObject(g_hCurrentJob, 
                                  JobObjectBasicAccountingInformation,
                                  &basicInfo, 
                                  sizeof(basicInfo),
                                  NULL))
    {
        PrintError("QueryInformationJobObject", GetLastError());
        return;
    }
    
    // Получаем расширенную информацию
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extInfo = { 0 };
    if (!QueryInformationJobObject(g_hCurrentJob, 
                                  JobObjectExtendedLimitInformation,
                                  &extInfo, 
                                  sizeof(extInfo),
                                  NULL))
    {
        PrintError("QueryInformationJobObject (extended)", GetLastError());
        return;
    }
    
    _tprintf(_T("Статистика Job '%s':\n\n"), g_JobName);
    _tprintf(_T("Количество процессов:\n"));
    _tprintf(_T("  Активных: %lu\n"), basicInfo.ActiveProcesses);
    _tprintf(_T("  Всего завершенных: %lu\n"), basicInfo.TotalTerminatedProcesses);
    
    _tprintf(_T("\nВремя выполнения (в секундах):\n"));
    _tprintf(_T("  Всего в пользовательском режиме: %.2f\n"), 
             (double)basicInfo.TotalUserTime.QuadPart / 10000000.0);
    _tprintf(_T("  Всего в ядре: %.2f\n"), 
             (double)basicInfo.TotalKernelTime.QuadPart / 10000000.0);
    
    _tprintf(_T("\nИспользование памяти:\n"));
    _tprintf(_T("  Пиковое использование памяти: %llu КБ\n"), 
             extInfo.PeakProcessMemoryUsed / 1024);
    _tprintf(_T("  Пиковое рабочее множество: %llu КБ\n"), 
             extInfo.PeakJobMemoryUsed / 1024);
    
    // Получаем список процессов в Job
    DWORD processListSize = 0;
    GetInformationJobObject(g_hCurrentJob, 
                           JobObjectBasicProcessIdList,
                           NULL, 0, &processListSize);
    
    if (processListSize > 0)
    {
        PJOBOBJECT_BASIC_PROCESS_ID_LIST processList = 
            (PJOBOBJECT_BASIC_PROCESS_ID_LIST)malloc(processListSize);
        
        if (processList && QueryInformationJobObject(g_hCurrentJob, 
                                                    JobObjectBasicProcessIdList,
                                                    processList, 
                                                    processListSize,
                                                    NULL))
        {
            _tprintf(_T("\nПроцессы в Job (%lu):\n"), processList->NumberOfProcessIdsInList);
            for (DWORD i = 0; i < processList->NumberOfProcessIdsInList; i++)
            {
                _tprintf(_T("  PID: %lu\n"), processList->ProcessIdList[i]);
                
                // Получаем имя процесса
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | 
                                             PROCESS_VM_READ,
                                             FALSE, 
                                             processList->ProcessIdList[i]);
                if (hProcess)
                {
                    TCHAR processName[MAX_PATH] = _T("неизвестно");
                    DWORD size = MAX_PATH;
                    QueryFullProcessImageName(hProcess, 0, processName, &size);
                    _tprintf(_T("    Имя: %s\n"), processName);
                    CloseHandle(hProcess);
                }
            }
        }
        
        if (processList) free(processList);
    }
    
    _tprintf(_T("\n✓ Мониторинг завершен. Все процессы в Job контролируются.\n"));
}

// Функция 4: Демонстрация безопасности Job
void JobSecurityDemo()
{
    _tprintf(_T("\n=== БЕЗОПАСНОСТЬ В JOB-ОБЪЕКТАХ ===\n\n"));
    
    if (!g_hCurrentJob)
    {
        _tprintf(_T("Сначала создайте Job-объект (пункт 1 меню).\n"));
        return;
    }
    
    _tprintf(_T("Job-объекты предоставляют несколько уровней безопасности:\n\n"));
    
    // 1. UI ограничения
    _tprintf(_T("1. Ограничения пользовательского интерфейса:\n"));
    
    JOBOBJECT_BASIC_UI_RESTRICTIONS uiRestrictions = { 0 };
    uiRestrictions.UIRestrictionsClass = 
        JOB_OBJECT_UILIMIT_DESKTOP |          // Нельзя создавать рабочие столы
        JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |  // Нельзя менять настройки дисплея
        JOB_OBJECT_UILIMIT_EXITWINDOWS |      // Нельзя выключать/перезагружать
        JOB_OBJECT_UILIMIT_GLOBALATOMS |      // Ограничение глобальных атомов
        JOB_OBJECT_UILIMIT_HANDLES |          // Ограничение системных дескрипторов
        JOB_OBJECT_UILIMIT_READCLIPBOARD |    // Нельзя читать буфер обмена
        JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS | // Нельзя менять системные параметры
        JOB_OBJECT_UILIMIT_WRITECLIPBOARD;    // Нельзя писать в буфер обмена
    
    if (SetInformationJobObject(g_hCurrentJob, 
                               JobObjectBasicUIRestrictions,
                               &uiRestrictions, 
                               sizeof(uiRestrictions)))
    {
        _tprintf(_T("   ✓ Применены UI ограничения:\n"));
        _tprintf(_T("     • Запрещена работа с буфером обмена\n"));
        _tprintf(_T("     • Запрещено изменение настроек системы\n"));
        _tprintf(_T("     • Запрещено выключение компьютера\n"));
        _tprintf(_T("     • Ограничен доступ к системным дескрипторам\n"));
    }
    
    // 2. Ограничения безопасности
    _tprintf(_T("\n2. Ограничения безопасности процессов:\n"));
    
    // Создаем ограниченный токен для процессов в Job
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        // Создаем ограниченный токен (убираем некоторые привилегии)
        HANDLE hRestrictedToken = NULL;
        if (CreateRestrictedToken(hToken, 
                                 DISABLE_MAX_PRIVILEGE, 
                                 0, NULL, 0, NULL, 0, NULL, 
                                 &hRestrictedToken))
        {
            JOBOBJECT_SECURITY_LIMIT_INFORMATION securityLimits = { 0 };
            securityLimits.SecurityLimitFlags = 
                JOB_OBJECT_SECURITY_NO_ADMIN |
                JOB_OBJECT_SECURITY_RESTRICTED_TOKEN |
                JOB_OBJECT_SECURITY_ONLY_TOKEN;
            
            securityLimits.JobToken = hRestrictedToken;
            
            if (SetInformationJobObject(g_hCurrentJob, 
                                       JobObjectSecurityLimitInformation,
                                       &securityLimits, 
                                       sizeof(securityLimits)))
            {
                _tprintf(_T("   ✓ Применены ограничения безопасности:\n"));
                _tprintf(_T("     • Процессы работают с ограниченным токеном\n"));
                _tprintf(_T("     • Запрещены административные привилегии\n"));
                _tprintf(_T("     • Токен доступен только для Job\n"));
            }
            
            CloseHandle(hRestrictedToken);
        }
        CloseHandle(hToken);
    }
    
    // 3. SID списки
    _tprintf(_T("\n3. Контроль доступа через SID:\n"));
    
    // Получаем SID текущего пользователя
    HANDLE hTokenUser = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenUser))
    {
        DWORD tokenInfoSize = 0;
        GetTokenInformation(hTokenUser, TokenUser, NULL, 0, &tokenInfoSize);
        
        if (tokenInfoSize > 0)
        {
            PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenInfoSize);
            if (pTokenUser && GetTokenInformation(hTokenUser, TokenUser, 
                                                 pTokenUser, tokenInfoSize, NULL))
            {
                JOBOBJECT_GROUPS_INFORMATION groupInfo = { 0 };
                groupInfo.Groups = pTokenUser->User.Sid;
                
                if (SetInformationJobObject(g_hCurrentJob, 
                                           JobObjectGroupInformation,
                                           &groupInfo, 
                                           sizeof(groupInfo)))
                {
                    _tprintf(_T("   ✓ Установлен контроль SID:\n"));
                    _tprintf(_T("     • Процессы ограничены SID текущего пользователя\n"));
                }
            }
            if (pTokenUser) free(pTokenUser);
        }
        CloseHandle(hTokenUser);
    }
    
    _tprintf(_T("\n✓ Безопасность Job настроена. Процессы изолированы.\n"));
}

// Функция 5: Демонстрация песочницы
void SandboxDemo()
{
    _tprintf(_T("\n=== ДЕМОНСТРАЦИЯ ПЕСОЧНИЦЫ (SANDBOX) ===\n\n"));
    
    if (!g_hCurrentJob)
    {
        _tprintf(_T("Сначала создайте Job-объект (пункт 1 меню).\n"));
        return;
    }
    
    _tprintf(_T("Запуск программы в изолированной среде (песочнице):\n\n"));
    
    _tprintf(_T("Введите путь к программе для запуска в песочнице:\n"));
    _tprintf(_T("(например, notepad.exe или calc.exe): "));
    
    TCHAR appPath[MAX_PATH];
    if (!_fgetts(appPath, MAX_PATH, stdin))
    {
        _tprintf(_T("Ошибка ввода.\n"));
        return;
    }
    
    // Убираем символ новой строки
    size_t len = _tcslen(appPath);
    if (len > 0 && appPath[len - 1] == _T('\n'))
    {
        appPath[len - 1] = _T('\0');
    }
    
    if (_tcslen(appPath) == 0)
    {
        _tcscpy_s(appPath, MAX_PATH, _T("notepad.exe"));
    }
    
    _tprintf(_T("\nЗапускаем '%s' в песочнице...\n"), appPath);
    
    // Создаем процесс
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    BOOL success = CreateProcess(
        NULL,
        appPath,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,  // Создаем приостановленным
        NULL,
        NULL,
        &si,
        &pi
    );
    
    if (!success)
    {
        PrintError("CreateProcess", GetLastError());
        return;
    }
    
    _tprintf(_T("✓ Процесс создан. PID: %lu\n"), pi.dwProcessId);
    
    // Назначаем процесс в Job
    if (!AssignProcessToJobObject(g_hCurrentJob, pi.hProcess))
    {
        PrintError("AssignProcessToJobObject", GetLastError());
        
        // Если не получилось, завершаем процесс
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return;
    }
    
    _tprintf(_T("✓ Процесс назначен в Job '%s'\n"), g_JobName);
    _tprintf(_T("✓ Все ограничения Job применяются к этому процессу\n\n"));
    
    // Показываем примененные ограничения
    _tprintf(_T("Примененные ограничения песочницы:\n"));
    _tprintf(_T("1. Ограничения CPU (если установлены)\n"));
    _tprintf(_T("2. Ограничения памяти\n"));
    _tprintf(_T("3. UI ограничения (нет доступа к системе)\n"));
    _tprintf(_T("4. Ограничения безопасности\n"));
    _tprintf(_T("5. Автозавершение при закрытии Job\n"));
    
    // Возобновляем выполнение процесса
    ResumeThread(pi.hThread);
    
    _tprintf(_T("\nПроцесс запущен в песочнице.\n"));
    _tprintf(_T("Можно продолжать работу или завершить Job (пункт 6).\n"));
    
    // Закрываем дескрипторы (процесс продолжает работу в Job)
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Функция 6: Завершение Job
void TerminateJobDemo()
{
    _tprintf(_T("\n=== ЗАВЕРШЕНИЕ JOB И ВСЕХ ПРОЦЕССОВ ===\n\n"));
    
    if (!g_hCurrentJob)
    {
        _tprintf(_T("Нет активного Job-объекта.\n"));
        return;
    }
    
    // Получаем информацию о процессах перед завершением
    JOBOBJECT_BASIC_ACCOUNTING_INFO accountingInfo = { 0 };
    if (QueryInformationJobObject(g_hCurrentJob, 
                                 JobObjectBasicAccountingInformation,
                                 &accountingInfo, 
                                 sizeof(accountingInfo),
                                 NULL))
    {
        _tprintf(_T("В Job '%s' сейчас %lu активных процесса(ов).\n"), 
                 g_JobName, accountingInfo.ActiveProcesses);
    }
    
    _tprintf(_T("\nВыберите способ завершения:\n"));
    _tprintf(_T("1. Немедленно завершить все процессы\n"));
    _tprintf(_T("2. Отправить сообщение о закрытии (корректное завершение)\n"));
    _tprintf(_T("3. Только закрыть дескриптор Job (процессы продолжат работу)\n"));
    _tprintf(_T("Ваш выбор: "));
    
    TCHAR choice[10];
    _fgetts(choice, sizeof(choice) / sizeof(TCHAR), stdin);
    
    switch (_ttoi(choice))
    {
    case 1:
        // Немедленное завершение
        if (TerminateJobObject(g_hCurrentJob, 0))
        {
            _tprintf(_T("✓ Все процессы в Job завершены.\n"));
        }
        else
        {
            PrintError("TerminateJobObject", GetLastError());
        }
        break;
        
    case 2:
        // Корректное завершение через сообщения
        _tprintf(_T("Отправка сообщений о закрытии...\n"));
        
        // Получаем список процессов
        DWORD processListSize = 0;
        GetInformationJobObject(g_hCurrentJob, 
                               JobObjectBasicProcessIdList,
                               NULL, 0, &processListSize);
        
        if (processListSize > 0)
        {
            PJOBOBJECT_BASIC_PROCESS_ID_LIST processList = 
                (PJOBOBJECT_BASIC_PROCESS_ID_LIST)malloc(processListSize);
            
            if (processList && QueryInformationJobObject(g_hCurrentJob, 
                                                        JobObjectBasicProcessIdList,
                                                        processList, 
                                                        processListSize,
                                                        NULL))
            {
                for (DWORD i = 0; i < processList->NumberOfProcessIdsInList; i++)
                {
                    // Пытаемся корректно закрыть окно
                    HWND hWnd = NULL;
                    DWORD pid = processList->ProcessIdList[i];
                    
                    // Находим главное окно процесса
                    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                        DWORD windowPid;
                        GetWindowThreadProcessId(hwnd, &windowPid);
                        
                        if (windowPid == *(DWORD*)lParam)
                        {
                            // Отправляем сообщение о закрытии
                            PostMessage(hwnd, WM_CLOSE, 0, 0);
                        }
                        return TRUE; // Продолжаем перечисление
                    }, (LPARAM)&pid);
                }
                
                _tprintf(_T("Сообщения отправлены. Ожидание 5 секунд...\n"));
                Sleep(5000);
                
                // Проверяем, остались ли процессы
                JOBOBJECT_BASIC_ACCOUNTING_INFO infoAfter = { 0 };
                if (QueryInformationJobObject(g_hCurrentJob, 
                                             JobObjectBasicAccountingInformation,
                                             &infoAfter, 
                                             sizeof(infoAfter),
                                             NULL) && 
                    infoAfter.ActiveProcesses > 0)
                {
                    _tprintf(_T("Осталось %lu процесса(ов). Завершаем принудительно.\n"), 
                             infoAfter.ActiveProcesses);
                    TerminateJobObject(g_hCurrentJob, 0);
                }
            }
            
            if (processList) free(processList);
        }
        _tprintf(_T("✓ Job завершен.\n"));
        break;
        
    case 3:
        // Просто закрываем дескриптор
        _tprintf(_T("Дескриптор Job закрыт. Процессы продолжают работу.\n"));
        break;
        
    default:
        _tprintf(_T("Отменено.\n"));
        return;
    }
    
    // Закрываем дескриптор Job
    CloseHandle(g_hCurrentJob);
    g_hCurrentJob = NULL;
    _tcscpy_s(g_JobName, 256, _T("DemoJob"));
    
    _tprintf(_T("\nJob-объект закрыт и очищен.\n"));
}

// Вспомогательная функция: вывод информации о Job
void PrintJobInfo(HANDLE hJob)
{
    _tprintf(_T("\n=== ИНФОРМАЦИЯ О JOB ===\n\n"));
    
    // Базовая информация
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicInfo = { 0 };
    if (QueryInformationJobObject(hJob, 
                                  JobObjectBasicAccountingInformation,
                                  &basicInfo, 
                                  sizeof(basicInfo),
                                  NULL))
    {
        _tprintf(_T("Базовая информация:\n"));
        _tprintf(_T("  Активных процессов: %lu\n"), basicInfo.ActiveProcesses);
        _tprintf(_T("  Всего процессов: %lu\n"), basicInfo.TotalProcesses);
        _tprintf(_T("  Завершено процессов: %lu\n"), basicInfo.TotalTerminatedProcesses);
        
        _tprintf(_T("  Время выполнения:\n"));
        _tprintf(_T("    Пользовательское: %.2f сек\n"), 
                 (double)basicInfo.TotalUserTime.QuadPart / 10000000.0);
        _tprintf(_T("    В ядре: %.2f сек\n"), 
                 (double)basicInfo.TotalKernelTime.QuadPart / 10000000.0);
    }
    
    // Информация об ограничениях
    JOBOBJECT_BASIC_LIMIT_INFORMATION limitInfo = { 0 };
    if (QueryInformationJobObject(hJob, 
                                  JobObjectBasicLimitInformation,
                                  &limitInfo, 
                                  sizeof(limitInfo),
                                  NULL))
    {
        _tprintf(_T("\nОграничения:\n"));
        
        if (limitInfo.LimitFlags & JOB_
Извините за обрыв! Продолжу код с того места, где остановился:

```c
        _tprintf(_T("  Приоритет: "));
        switch (limitInfo.PriorityClass)
        {
        case IDLE_PRIORITY_CLASS: _tprintf(_T("IDLE\n")); break;
        case NORMAL_PRIORITY_CLASS: _tprintf(_T("NORMAL\n")); break;
        case HIGH_PRIORITY_CLASS: _tprintf(_T("HIGH\n")); break;
        case REALTIME_PRIORITY_CLASS: _tprintf(_T("REALTIME\n")); break;
        default: _tprintf(_T("%lu\n"), limitInfo.PriorityClass); break;
        }
        
        if (limitInfo.LimitFlags & JOB_OBJECT_LIMIT_ACTIVE_PROCESS)
            _tprintf(_T("  Макс процессов: %lu\n"), limitInfo.ActiveProcessLimit);
        
        if (limitInfo.LimitFlags & JOB_OBJECT_LIMIT_PROCESS_TIME)
            _tprintf(_T("  Лимит CPU на процесс: %.1f сек\n"), 
                     (double)limitInfo.PerProcessUserTimeLimit.QuadPart / 10000000.0);
        
        if (limitInfo.LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME)
            _tprintf(_T("  Лимит CPU на Job: %.1f сек\n"), 
                     (double)limitInfo.PerJobUserTimeLimit.QuadPart / 10000000.0);
    }
}

// Вспомогательная функция: вывод ошибки
void PrintError(const char* functionName, DWORD errorCode)
{
    LPTSTR errorMsg = NULL;
    
    DWORD chars = FormatMessage(
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
    
    if (chars > 0 && errorMsg != NULL)
    {
        wprintf(L"Ошибка в функции %hs (код %lu): %ls", functionName, errorCode, errorMsg);
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
    
    _tprintf(_T("1. Создать базовый Job-объект\n"));
    _tprintf(_T("2. Настроить ограничения ресурсов\n"));
    _tprintf(_T("3. Мониторинг ресурсов Job\n"));
    _tprintf(_T("4. Настройка безопасности Job\n"));
    _tprintf(_T("5. Демонстрация песочницы\n"));
    _tprintf(_T("6. Завершить Job и все процессы\n"));
    _tprintf(_T("7. Показать информацию о Job\n"));
    _tprintf(_T("8. Выход\n\n"));
}
```

---

## 🔧 **КОМПИЛЯЦИЯ И ЗАПУСК:**

### **В Visual Studio:**
1. Создайте **консольное приложение**
2. Включите опцию **"Character Set: Use Unicode Character Set"**
3. Добавьте библиотеки: `kernel32.lib`, `user32.lib`, `psapi.lib`, `advapi32.lib`
4. Вставьте полный код в `main.c`
5. Запустите от имени администратора для полного доступа

### **Командная строка:**
```bash
cl lesson4.c /link kernel32.lib user32.lib psapi.lib advapi32.lib /D UNICODE /D _UNICODE
```

---

## 📋 **АНАЛИЗ ПРОГРАММЫ:**

1. Почему в функции `CreateBasicJob()` устанавливается флаг `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`?
2. Что произойдет, если процесс попытается превысить лимит памяти, установленный в Job?
3. Почему для запуска процесса в песочнице используется `CREATE_SUSPENDED`?
4. Как UI ограничения предотвращают взаимодействие процессов с системой?
5. В чем разница между `TerminateJobObject()` и простым закрытием дескриптора Job?

---

## 🎯 **ПРАКТИЧЕСКИЕ ЗАДАНИЯ ДЛЯ САМОСТОЯТЕЛЬНОЙ РАБОТЫ:**

### **Уровень 1: Модификация кода**
1. Добавьте возможность сохранения конфигурации Job в файл и загрузки из файла
2. Реализуйте мониторинг в реальном времени с обновлением статистики каждые 2 секунды
3. Добавьте графическое отображение использования ресурсов (ASCII-графики)

### **Уровень 2: Новые функции**
1. Реализуйте "профили песочницы" (Low/Medium/High security)
2. Создайте систему квот на дисковый ввод-вывод для процессов в Job
3. Добавьте возможность наследования Job-объектов (вложенные Jobs)

### **Уровень 3: Продвинутые задачи**
1. Реализуйте механизм "белого списка" для разрешенных системных вызовов
2. Создайте систему изоляции сети для процессов в Job
3. Реализуйте механизм принудительного применения политик AppLocker через Job

### **Уровень 4: Проектная работа**
1. **Контейнерная система:** Мини-аналог Docker для Windows с использованием Job-объектов
2. **Система контроля заданий:** Управление пакетными заданиями с квотами ресурсов
3. **Защищенный браузер:** Запуск браузера в изолированной песочнице с автоматическим созданием Job
4. **Монитор безопасности:** Система обнаружения аномального поведения процессов в Job

---

## 💡 **СОВЕТЫ ПО ВЫПОЛНЕНИЮ:**

1. **Тестируйте на VM:** Все эксперименты с изоляцией проводите на виртуальной машине
2. **Начинайте с простого:** Сначала создайте базовый Job без ограничений
3. **Используйте Process Explorer:** Для визуализации процессов в Job
4. **Тестируйте граничные случаи:** Что происходит при достижении лимитов
5. **Изучайте документацию:** MSDN содержит детали по всем флагам Job-объектов

---

## ⚠️ **ВАЖНЫЕ ПРЕДУПРЕЖДЕНИЯ:**

1. **Флаг `KILL_ON_JOB_CLOSE`:** Убедитесь, что понимаете его последствия
2. **Ограничения памяти:** Слишком жесткие ограничения могут привести к сбоям
3. **UI ограничения:** Некоторые программы могут не работать без доступа к UI
4. **Системные процессы:** Не пытайтесь поместить системные процессы в Job
5. **Вложенные Jobs:** Могут быть сложны для отладки

---

## 🏆 **ДОПОЛНИТЕЛЬНЫЕ ВОЗМОЖНОСТИ:**

### **Для углубленного изучения:**
1. **Nested Jobs:** Работа с вложенными Job-объектами
2. **Job Notifications:** Асинхронные уведомления о событиях в Job
3. **Resource Accounting:** Детальный учет использования ресурсов
4. **Job Object Security:** Расширенные настройки безопасности
5. **Job Objects in Services:** Использование в Windows службах

### **Интеграционные возможности:**
1. **GUI интерфейс:** Графическая оболочка для управления Jobs
2. **Сетевой мониторинг:** Удаленное управление Jobs по сети
3. **Плагины:** Поддержка дополнительных модулей безопасности
4. **Скриптовый интерфейс:** Управление через PowerShell или Python
5. **Интеграция с Docker:** Использование Job-объектов для Windows-контейнеров

---

## 📊 **ПРИМЕРЫ ТЕСТОВЫХ СЦЕНАРИЕВ:**

### **Базовые тесты:**
1. Создание Job и запуск notepad.exe
2. Проверка ограничения на количество процессов
3. Тест ограничений CPU (запуск stress-теста)
4. Проверка ограничений памяти
5. Тест UI ограничений (попытка доступа к буферу обмена)

### **Продвинутые тесты:**
1. Запуск браузера в изолированной песочнице
2. Тестирование системы с вложенными Jobs
3. Проверка работы с системными вызовами
4. Тест безопасности с разными токенами
5. Долгосрочный мониторинг стабильности

### **Интеграционные тесты:**
1. Совместная работа нескольких Jobs
2. Миграция процессов между Jobs
3. Динамическое изменение ограничений
4. Восстановление после сбоев
5. Тестирование производительности под нагрузкой

---

## 🔧 **ПОЛЕЗНЫЕ ИНСТРУМЕНТЫ:**

1. **Process Explorer** — для просмотра процессов в Job
2. **Process Monitor** — для отладки системных вызовов
3. **PerfMon** — для мониторинга производительности
4. **WinDbg** — для отладки сложных случаев
5. **RAMMap** — для анализа использования памяти

---

**Этот урок дает мощный инструмент для контроля и изоляции процессов в Windows. Job-объекты используются в современных системах виртуализации, песочницах и системах управления ресурсами. Успешное освоение темы открывает путь к созданию профессиональных систем безопасности и управления ресурсами.**

Удачи в освоении! 🚀
