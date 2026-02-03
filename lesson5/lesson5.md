# 🧵 **УРОК 5: ПОТОКИ (THREADS) В WINDOWS — СОЗДАНИЕ, УПРАВЛЕНИЕ И СИНХРОНИЗАЦИЯ**

**Цель урока:**  
Освоить работу с потоками в Windows API: создание, управление, приоритеты и синхронизация. Научиться использовать критические секции, мьютексы и события для безопасного многопоточного программирования.

---

## 📚 **ТЕОРЕТИЧЕСКАЯ ЧАСТЬ (по книге Иосифовича)**

### **Главы для изучения:**
- **"Thread Creation and Management"** — создание и управление потоками
- **"Thread Synchronization"** — синхронизация потоков
- **"Thread Priorities and Scheduling"** — приоритеты и планирование

### **Ключевые концепции:**

#### **1. Потоки в Windows:**
- **Thread vs Process:** Поток — единица выполнения в рамках процесса, разделяющая его память и ресурсы
- **Основной поток (Main Thread):** Первый поток, создаваемый системой при запуске процесса
- **Worker Threads:** Дополнительные потоки для параллельного выполнения задач
- **Контекст потока:** Регистры, стек, приоритет и состояние потока

#### **2. Создание и управление потоками:**
- **`CreateThread()`:** Основная функция создания потока
- **`_beginthreadex()`:** Безопасная версия из CRT (C Runtime)
- **`ExitThread()` и `TerminateThread()`:** Способы завершения потока
- **`SuspendThread()` / `ResumeThread()`:** Приостановка и возобновление потока
- **Приоритеты:** `THREAD_PRIORITY_NORMAL`, `THREAD_PRIORITY_HIGH`, `THREAD_PRIORITY_LOW`

#### **3. Синхронизация потоков:**
- **Критические секции (`CRITICAL_SECTION`):** Быстрая синхронизация в пределах одного процесса
- **Мьютексы (`HANDLE`):** Межпроцессная синхронизация с возможностью ожидания
- **События (`EVENT`):** Уведомление потоков о наступлении события
- **Семафоры (`SEMAPHORE`):** Ограничение доступа к ресурсу определенным числом потоков

#### **4. Проблемы многопоточности:**
- **Race Condition:** Гонки данных при одновременном доступе
- **Deadlock:** Взаимная блокировка потоков
- **Starvation:** Голодание потока из-за высокого приоритета других
- **Thread Safety:** Потокобезопасность функций и структур данных

---

## 📝 **ЗАДАНИЕ**

### **Часть 1: Теория (ответьте письменно)**
1. Чем отличается `CreateThread()` от `_beginthreadex()` и какая функция предпочтительнее?
2. Что такое "гонка данных" (race condition) и как ее предотвратить?
3. Когда следует использовать критические секции, а когда мьютексы?
4. Какие проблемы могут возникнуть при использовании `TerminateThread()`?
5. Как работают события (Event objects) и в каких сценариях они полезны?

### **Часть 2: Практическая работа**

**Задача:** Создать многопоточное приложение для параллельной обработки данных с различными механизмами синхронизации.

**Требования к программе:**
1. Создание рабочих потоков с разными приоритетами
2. Параллельная обработка массива данных
3. Реализация синхронизации через критические секции
4. Использование мьютексов для защиты разделяемых ресурсов
5. Координация потоков через события
6. Измерение производительности и анализ накладных расходов

---

## 💻 **ПОЛНЫЙ ИСХОДНЫЙ КОД ПРОГРАММЫ**

```c
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>  // Для _beginthreadex

// Константы и глобальные переменные
#define THREAD_COUNT 4
#define ARRAY_SIZE 1000000
#define CHUNK_SIZE (ARRAY_SIZE / THREAD_COUNT)

// Структура для передачи данных в поток
typedef struct {
    int id;
    int priority;
    int* data;
    int start_idx;
    int end_idx;
    long long partial_sum;
    HANDLE start_event;
    HANDLE done_event;
    HANDLE mutex;
    CRITICAL_SECTION* critical_section;
} ThreadData;

// Глобальные переменные для синхронизации
CRITICAL_SECTION g_critical_section;
HANDLE g_mutex = NULL;
HANDLE g_start_event = NULL;
HANDLE g_done_events[THREAD_COUNT];
long long g_global_sum = 0;
int g_shared_counter = 0;

// Прототипы функций
unsigned __stdcall WorkerThread(void* param);
void CreateThreadsWithPriorities();
void DemonstrateCriticalSection();
void DemonstrateMutex();
void DemonstrateEvents();
void ParallelArrayProcessing();
void MeasurePerformance();
void PrintThreadInfo(DWORD thread_id, HANDLE thread_handle);

// Главная функция
int main()
{
    // Настройка русской локали
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    printf("=== МНОГОПОТОЧНОЕ ПРИЛОЖЕНИЕ WINDOWS API ===\n\n");

    // Инициализация механизмов синхронизации
    InitializeCriticalSection(&g_critical_section);
    g_mutex = CreateMutex(NULL, FALSE, NULL);
    g_start_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    for (int i = 0; i < THREAD_COUNT; i++) {
        g_done_events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    int choice = 0;
    char input[10];

    while (1) {
        printf("\n=== ГЛАВНОЕ МЕНЮ ===\n\n");
        printf("1. Создание потоков с разными приоритетами\n");
        printf("2. Демонстрация критических секций\n");
        printf("3. Демонстрация мьютексов\n");
        printf("4. Демонстрация событий (Events)\n");
        printf("5. Параллельная обработка массива\n");
        printf("6. Измерение производительности\n");
        printf("7. Выход\n\n");
        printf("Выберите действие (1-7): ");

        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
        }

        switch (choice) {
        case 1:
            CreateThreadsWithPriorities();
            break;
        case 2:
            DemonstrateCriticalSection();
            break;
        case 3:
            DemonstrateMutex();
            break;
        case 4:
            DemonstrateEvents();
            break;
        case 5:
            ParallelArrayProcessing();
            break;
        case 6:
            MeasurePerformance();
            break;
        case 7:
            printf("Завершение программы...\n");

            // Очистка ресурсов
            DeleteCriticalSection(&g_critical_section);
            CloseHandle(g_mutex);
            CloseHandle(g_start_event);

            for (int i = 0; i < THREAD_COUNT; i++) {
                CloseHandle(g_done_events[i]);
            }

            return 0;
        default:
            printf("Неверный выбор. Попробуйте снова.\n");
            break;
        }

        printf("\nНажмите Enter для продолжения...");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    return 0;
}

// Функция 1: Создание потоков с разными приоритетами
void CreateThreadsWithPriorities()
{
    printf("\n=== СОЗДАНИЕ ПОТОКОВ С РАЗНЫМИ ПРИОРИТЕТАМИ ===\n\n");

    HANDLE threads[THREAD_COUNT];
    DWORD thread_ids[THREAD_COUNT];
    ThreadData* thread_data[THREAD_COUNT];

    // Инициализируем массивы нулями
    ZeroMemory(threads, sizeof(threads));
    ZeroMemory(thread_ids, sizeof(thread_ids));
    ZeroMemory(thread_data, sizeof(thread_data));

    printf("Создаем %d потоков с разными приоритетами:\n\n", THREAD_COUNT);

    // Создаем потоки с разными приоритетами
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_data[i] = (ThreadData*)malloc(sizeof(ThreadData));
        if (thread_data[i] == NULL) {
            printf("Ошибка выделения памяти для потока %d\n", i + 1);
            // Освобождаем ранее выделенную память
            for (int j = 0; j < i; j++) {
                if (thread_data[j]) free(thread_data[j]);
            }
            return;
        }

        // Инициализируем все поля структуры
        thread_data[i]->id = i + 1;
        thread_data[i]->data = NULL;
        thread_data[i]->start_idx = 0;
        thread_data[i]->end_idx = 0;
        thread_data[i]->partial_sum = 0;
        thread_data[i]->start_event = NULL;
        thread_data[i]->done_event = NULL;
        thread_data[i]->mutex = NULL;
        thread_data[i]->critical_section = NULL;

        // Назначаем разные приоритеты
        switch (i) {
        case 0:
            thread_data[i]->priority = THREAD_PRIORITY_IDLE;
            printf("Поток %d: THREAD_PRIORITY_IDLE\n", i + 1);
            break;
        case 1:
            thread_data[i]->priority = THREAD_PRIORITY_LOWEST;
            printf("Поток %d: THREAD_PRIORITY_LOWEST\n", i + 1);
            break;
        case 2:
            thread_data[i]->priority = THREAD_PRIORITY_NORMAL;
            printf("Поток %d: THREAD_PRIORITY_NORMAL\n", i + 1);
            break;
        case 3:
            thread_data[i]->priority = THREAD_PRIORITY_HIGHEST;
            printf("Поток %d: THREAD_PRIORITY_HIGHEST\n", i + 1);
            break;
        }

        // Создаем поток с использованием _beginthreadex
        threads[i] = (HANDLE)_beginthreadex(
            NULL,                   // Атрибуты безопасности
            0,                      // Размер стека (0 = по умолчанию)
            WorkerThread,          // Функция потока
            thread_data[i],        // Параметры
            0,                      // Флаги создания (0 = запуск сразу)
            (unsigned int*)&thread_ids[i] // ID потока
        );

        if (threads[i] == NULL) {
            printf("Ошибка создания потока %d: %lu\n", i + 1, GetLastError());
            free(thread_data[i]);
            thread_data[i] = NULL;
            continue;
        }

        // Устанавливаем приоритет потока
        SetThreadPriority(threads[i], thread_data[i]->priority);

        // Получаем и выводим информацию о потоке
        PrintThreadInfo(thread_ids[i], threads[i]);
    }

    printf("\nПотоки созданы. Ожидаем их завершения...\n");

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

    printf("Все потоки завершены.\n");

    // Освобождаем ресурсы
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }
}

// Функция 2: Демонстрация критических секций
void DemonstrateCriticalSection()
{
    printf("\n=== ДЕМОНСТРАЦИЯ КРИТИЧЕСКИХ СЕКЦИЙ ===\n\n");

    printf("Критические секции используются для быстрой синхронизации\n");
    printf("в пределах одного процесса. Они не являются объектами ядра,\n");
    printf("поэтому работают быстрее, чем мьютексы.\n\n");

    HANDLE threads[3];
    DWORD thread_ids[3];

    // Сбрасываем глобальный счетчик
    g_shared_counter = 0;

    printf("Начальное значение счетчика: %d\n", g_shared_counter);
    printf("Запускаем 3 потока, каждый увеличивает счетчик на 100000...\n");

    // Создаем потоки для демонстрации
    for (int i = 0; i < 3; i++) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->id = i + 1;
        data->critical_section = &g_critical_section;
        data->data = NULL;
        data->start_idx = 0;
        data->end_idx = 0;
        data->partial_sum = 0;
        data->start_event = NULL;
        data->done_event = NULL;
        data->mutex = NULL;
        data->priority = THREAD_PRIORITY_NORMAL;

        threads[i] = (HANDLE)_beginthreadex(
            NULL, 0, WorkerThread, data, 0, (unsigned int*)&thread_ids[i]
        );

        if (threads[i] == NULL) {
            printf("Ошибка создания потока %d\n", i + 1);
            free(data);
        }
    }

    // Ожидаем завершения потоков
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    printf("\nИтоговое значение счетчика: %d\n", g_shared_counter);
    printf("Ожидаемое значение: 300000\n");
    printf("Результат: %s\n\n", g_shared_counter == 300000 ? "Правильно" : "Ошибка синхронизации!");

    // Освобождаем ресурсы
    for (int i = 0; i < 3; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }

    printf("Важно: Критические секции:\n");
    printf("  - Работают только в пределах одного процесса\n");
    printf("  - Не имеют timeout при ожидании\n");
    printf("  - Могут вызывать deadlock при рекурсивном входе\n");
    printf("  - Должны быть удалены через DeleteCriticalSection\n");
}

// Функция 3: Демонстрация мьютексов
void DemonstrateMutex()
{
    printf("\n=== ДЕМОНСТРАЦИЯ МЬЮТЕКСОВ ===\n\n");

    printf("Мьютексы (mutex) — объекты ядра для межпроцессной синхронизации.\n");
    printf("Они медленнее критических секций, но могут использоваться\n");
    printf("для синхронизации между разными процессами.\n\n");

    HANDLE threads[2];
    DWORD thread_ids[2];

    // Сбрасываем глобальный счетчик
    g_shared_counter = 0;

    printf("Создаем именованный мьютекс для межпроцессной синхронизации\n");
    HANDLE named_mutex = CreateMutex(
        NULL,                       // Атрибуты безопасности
        FALSE,                      // Не владеем мьютексом изначально
        L"Global\\MyNamedMutex"     // Имя мьютекса
    );

    if (named_mutex == NULL) {
        printf("Ошибка создания именованного мьютекса: %lu\n", GetLastError());
        named_mutex = g_mutex;  // Используем обычный мьютекс
    }

    printf("Начальное значение счетчика: %d\n", g_shared_counter);
    printf("Запускаем 2 потока с использованием мьютекса...\n");

    // Создаем потоки для демонстрации мьютексов
    for (int i = 0; i < 2; i++) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->id = i + 1;
        data->mutex = named_mutex;
        data->critical_section = NULL;
        data->data = NULL;
        data->start_idx = 0;
        data->end_idx = 0;
        data->partial_sum = 0;
        data->start_event = NULL;
        data->done_event = NULL;
        data->priority = THREAD_PRIORITY_NORMAL;

        threads[i] = (HANDLE)_beginthreadex(
            NULL, 0, WorkerThread, data, 0, (unsigned int*)&thread_ids[i]
        );

        if (threads[i] == NULL) {
            printf("Ошибка создания потока %d\n", i + 1);
            free(data);
        }
    }

    // Ожидаем завершения потоков
    WaitForMultipleObjects(2, threads, TRUE, 5000);  // 5 секунд timeout

    printf("\nИтоговое значение счетчика: %d\n", g_shared_counter);
    printf("Ожидаемое значение: 200000\n");
    printf("Результат: %s\n\n", g_shared_counter == 200000 ? "Правильно" : "Ошибка синхронизации!");

    // Демонстрация timeout для мьютекса
    printf("Демонстрация WaitForSingleObject с timeout:\n");
    DWORD wait_result = WaitForSingleObject(named_mutex, 100);  // 100ms timeout

    switch (wait_result) {
    case WAIT_OBJECT_0:
        printf("  Мьютекс получен успешно\n");
        ReleaseMutex(named_mutex);
        break;
    case WAIT_TIMEOUT:
        printf("  Таймаут ожидания мьютекса\n");
        break;
    case WAIT_ABANDONED:
        printf("  Мьютекс был заброшен предыдущим владельцем\n");
        break;
    case WAIT_FAILED:
        printf("  Ошибка ожидания: %lu\n", GetLastError());
        break;
    }

    // Освобождаем ресурсы
    for (int i = 0; i < 2; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }

    if (named_mutex != g_mutex) {
        CloseHandle(named_mutex);
    }

    printf("\nПреимущества мьютексов:\n");
    printf("  - Межпроцессная синхронизация\n");
    printf("  - Поддержка timeout при ожидании\n");
    printf("  - Обнаружение смерти владельца (WAIT_ABANDONED)\n");
    printf("  - Могут быть именованными для доступа из других процессов\n");
}

// Функция 4: Демонстрация событий (Events)
void DemonstrateEvents()
{
    printf("\n=== ДЕМОНСТРАЦИЯ СОБЫТИЙ (EVENTS) ===\n\n");

    printf("События (Event objects) — механизм уведомления потоков.\n");
    printf("Используются для координации работы потоков и сигнализации\n");
    printf("о наступлении определенных условий.\n\n");

    HANDLE threads[THREAD_COUNT];
    DWORD thread_ids[THREAD_COUNT];

    // Сбрасываем события
    ResetEvent(g_start_event);
    for (int i = 0; i < THREAD_COUNT; i++) {
        ResetEvent(g_done_events[i]);
    }

    // Создаем рабочие потоки
    for (int i = 0; i < THREAD_COUNT; i++) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->id = i + 1;
        data->start_event = g_start_event;
        data->done_event = g_done_events[i];
        data->priority = THREAD_PRIORITY_NORMAL;
        data->data = NULL;
        data->start_idx = 0;
        data->end_idx = 0;
        data->partial_sum = 0;
        data->mutex = NULL;
        data->critical_section = NULL;

        threads[i] = (HANDLE)_beginthreadex(
            NULL, 0, WorkerThread, data, 0, (unsigned int*)&thread_ids[i]
        );

        if (threads[i] == NULL) {
            printf("Ошибка создания потока %d\n", i + 1);
            free(data);
        }
    }

    printf("Потоки созданы и ожидают события START...\n");
    printf("Нажмите Enter чтобы запустить все потоки одновременно\n");
    getchar();

    // Запускаем все потоки одновременно
    SetEvent(g_start_event);

    printf("\nПотоки запущены. Ожидаем их завершения...\n");

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(THREAD_COUNT, g_done_events, TRUE, INFINITE);

    printf("Все потоки завершили работу.\n");

    // Даем потокам время корректно завершиться
    WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, 1000);

    // Освобождаем ресурсы
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }

    printf("\nТипы событий:\n");
    printf("  1. Auto-reset событие: сбрасывается автоматически после пробуждения одного потока\n");
    printf("  2. Manual-reset событие: требуется ручной сброс через ResetEvent\n");
    printf("  3. Именованные события: могут использоваться для межпроцессной синхронизации\n");

    // Демонстрация различных типов событий
    printf("\nСоздаем различные типы событий:\n");

    // Auto-reset событие
    HANDLE auto_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    printf("  Auto-reset событие: %s\n", auto_event ? "создано" : "ошибка");

    // Manual-reset событие
    HANDLE manual_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    printf("  Manual-reset событие: %s\n", manual_event ? "создано" : "ошибка");

    // Именованное событие (для межпроцессного использования)
    HANDLE named_event = CreateEvent(NULL, TRUE, FALSE, L"Global\\MyNamedEvent");
    printf("  Именованное событие: %s\n", named_event ? "создано" : "ошибка");

    // Освобождаем события
    if (auto_event) CloseHandle(auto_event);
    if (manual_event) CloseHandle(manual_event);
    if (named_event) CloseHandle(named_event);
}

// Функция 5: Параллельная обработка массива
void ParallelArrayProcessing()
{
    printf("\n=== ПАРАЛЛЕЛЬНАЯ ОБРАБОТКА МАССИВА ===\n\n");

    // Создаем и инициализируем большой массив
    printf("Создаем массив из %d элементов...\n", ARRAY_SIZE);
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));

    if (data == NULL) {
        printf("Ошибка выделения памяти!\n");
        return;
    }

    // Заполняем массив случайными числами
    srand((unsigned int)time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 100;
    }

    printf("Массив создан. Запускаем параллельную обработку...\n\n");

    HANDLE threads[THREAD_COUNT];
    DWORD thread_ids[THREAD_COUNT];
    ThreadData* thread_data[THREAD_COUNT];

    // Инициализируем массивы нулями
    ZeroMemory(threads, sizeof(threads));
    ZeroMemory(thread_ids, sizeof(thread_ids));
    ZeroMemory(thread_data, sizeof(thread_data));

    // Подготавливаем данные для потоков
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_data[i] = (ThreadData*)malloc(sizeof(ThreadData));
        if (thread_data[i] == NULL) {
            printf("Ошибка выделения памяти для данных потока %d\n", i + 1);
            // Освобождаем ранее выделенную память
            for (int j = 0; j < i; j++) {
                if (thread_data[j]) free(thread_data[j]);
            }
            free(data);
            return;
        }

        thread_data[i]->id = i + 1;
        thread_data[i]->data = data;
        thread_data[i]->start_idx = i * CHUNK_SIZE;
        thread_data[i]->end_idx = (i == THREAD_COUNT - 1) ? ARRAY_SIZE : (i + 1) * CHUNK_SIZE;
        thread_data[i]->partial_sum = 0;
        thread_data[i]->mutex = g_mutex;
        thread_data[i]->critical_section = &g_critical_section;
        thread_data[i]->priority = THREAD_PRIORITY_NORMAL;
        thread_data[i]->start_event = NULL;
        thread_data[i]->done_event = NULL;
    }

    // Измеряем время выполнения с помощью GetTickCount64
    ULONGLONG start_time = GetTickCount64();

    // Создаем и запускаем потоки
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (thread_data[i] != NULL) {
            threads[i] = (HANDLE)_beginthreadex(
                NULL, 0, WorkerThread, thread_data[i], 0, (unsigned int*)&thread_ids[i]
            );

            if (threads[i] == NULL) {
                printf("Ошибка создания потока %d\n", i + 1);
                // Освобождаем память для этого потока
                free(thread_data[i]);
                thread_data[i] = NULL;
            }
        }
    }

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

    ULONGLONG end_time = GetTickCount64();
    DWORD parallel_time = (DWORD)(end_time - start_time);

    // Вычисляем общую сумму из частичных сумм
    long long total_sum = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (thread_data[i] != NULL) {
            total_sum += thread_data[i]->partial_sum;
        }
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }

    printf("Параллельная обработка завершена:\n");
    printf("  Время выполнения: %lu мс\n", parallel_time);
    printf("  Сумма элементов массива: %lld\n", total_sum);

    // Сравниваем с однопоточной версией
    printf("\nСравнение с однопоточной версией:\n");

    start_time = GetTickCount64();

    long long single_thread_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        single_thread_sum += data[i];
    }

    end_time = GetTickCount64();
    DWORD single_thread_time = (DWORD)(end_time - start_time);

    printf("  Однопоточное время: %lu мс\n", single_thread_time);
    printf("  Сумма элементов: %lld\n", single_thread_sum);

    if (parallel_time > 0) {
        printf("  Ускорение: %.2fx\n", (float)single_thread_time / parallel_time);
    }
    else {
        printf("  Ускорение: очень большое (время параллельной обработки < 1 мс)\n");
    }
    printf("  Корректность: %s\n\n", total_sum == single_thread_sum ? "OK" : "Ошибка!");

    // Освобождаем ресурсы
    free(data);

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (thread_data[i] != NULL) {
            free(thread_data[i]);
        }
    }

    printf("Выводы:\n");
    printf("  - Многопоточность ускоряет вычисления на многоядерных CPU\n");
    printf("  - Синхронизация добавляет накладные расходы\n");
    printf("  - Оптимальное число потоков примерно равно числу ядер процессора\n");
    printf("  - Важно равномерно распределять работу между потоками\n");
}

// Функция 6: Измерение производительности
void MeasurePerformance()
{
    printf("\n=== ИЗМЕРЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===\n\n");

    printf("Сравнение производительности разных механизмов синхронизации:\n\n");

    int iterations = 1000000;  // Количество операций

    // 1. Без синхронизации (некорректно, но быстро)
    printf("1. Без синхронизации:\n");
    DWORD start_time = GetTickCount();

    volatile LONG counter = 0;
    for (int i = 0; i < iterations; i++) {
        counter++;  // Небезопасно в многопоточном режиме!
    }

    DWORD end_time = GetTickCount();
    printf("   Время: %lu мс\n", end_time - start_time);

    // 2. С использованием InterlockedIncrement (атомарные операции)
    printf("\n2. InterlockedIncrement (атомарные операции):\n");
    start_time = GetTickCount();

    counter = 0;
    for (int i = 0; i < iterations; i++) {
        InterlockedIncrement(&counter);
    }

    end_time = GetTickCount();
    printf("   Время: %lu мс\n", end_time - start_time);

    // 3. С использованием критической секции
    printf("\n3. Критическая секция:\n");
    start_time = GetTickCount();

    counter = 0;
    for (int i = 0; i < iterations; i++) {
        EnterCriticalSection(&g_critical_section);
        counter++;
        LeaveCriticalSection(&g_critical_section);
    }

    end_time = GetTickCount();
    printf("   Время: %lu мс\n", end_time - start_time);

    // 4. С использованием мьютекса
    printf("\n4. Мьютекс:\n");
    start_time = GetTickCount();

    counter = 0;
    for (int i = 0; i < iterations; i++) {
        WaitForSingleObject(g_mutex, INFINITE);
        counter++;
        ReleaseMutex(g_mutex);
    }

    end_time = GetTickCount();
    printf("   Время: %lu мс\n", end_time - start_time);

    printf("\nРекомендации по выбору механизма синхронизации:\n");
    printf("  1. Interlocked операции - для простых атомарных операций\n");
    printf("  2. Критические секции - для внутрипроцессной синхронизации\n");
    printf("  3. Мьютексы - для межпроцессной синхронизации\n");
    printf("  4. События - для координации потоков\n");
    printf("  5. Семафоры - для ограничения числа одновременных доступов\n");
}

// Рабочая функция потока
unsigned __stdcall WorkerThread(void* param)
{
    if (param == NULL) {
        return 0;
    }

    ThreadData* data = (ThreadData*)param;

    // Ожидание события START
    if (data->start_event != NULL) {
        WaitForSingleObject(data->start_event, INFINITE);
        printf("Поток %d запущен\n", data->id);
    }

    // Выполняем разные задачи в зависимости от контекста
    if (data->data != NULL) {
        // Обработка массива - используем критическую секцию для безопасности
        long long local_sum = 0;
        for (int i = data->start_idx; i < data->end_idx; i++) {
            local_sum += data->data[i];
        }
        data->partial_sum = local_sum;
    }
    else if (data->critical_section != NULL) {
        // Демонстрация критических секций
        for (int i = 0; i < 100000; i++) {
            EnterCriticalSection(data->critical_section);
            g_shared_counter++;
            LeaveCriticalSection(data->critical_section);
        }
    }
    else if (data->mutex != NULL) {
        // Демонстрация мьютексов
        for (int i = 0; i < 100000; i++) {
            WaitForSingleObject(data->mutex, INFINITE);
            g_shared_counter++;
            ReleaseMutex(data->mutex);
        }
    }
    else {
        // Простая работа потока
        for (int i = 0; i < 5; i++) {
            printf("Поток %d (приоритет: %d) - шаг %d\n",
                data->id, data->priority, i + 1);
            Sleep(100);
        }
    }

    // Сигнализируем о завершении работы
    if (data->done_event != NULL) {
        SetEvent(data->done_event);
    }

    // Освобождаем память только для простых потоков
    if (data->data == NULL || data->start_event != NULL || data->done_event != NULL) {
        free(data);
    }

    return 0;
}

// Вспомогательная функция: вывод информации о потоке
void PrintThreadInfo(DWORD thread_id, HANDLE thread_handle)
{
    printf("  ID потока: %lu\n", thread_id);

    // Получаем приоритет потока
    int priority = GetThreadPriority(thread_handle);
    printf("  Приоритет: ");

    switch (priority) {
    case THREAD_PRIORITY_IDLE:
        printf("IDLE (-15)\n");
        break;
    case THREAD_PRIORITY_LOWEST:
        printf("LOWEST (-2)\n");
        break;
    case THREAD_PRIORITY_BELOW_NORMAL:
        printf("BELOW_NORMAL (-1)\n");
        break;
    case THREAD_PRIORITY_NORMAL:
        printf("NORMAL (0)\n");
        break;
    case THREAD_PRIORITY_ABOVE_NORMAL:
        printf("ABOVE_NORMAL (+1)\n");
        break;
    case THREAD_PRIORITY_HIGHEST:
        printf("HIGHEST (+2)\n");
        break;
    case THREAD_PRIORITY_TIME_CRITICAL:
        printf("TIME_CRITICAL (+15)\n");
        break;
    default:
        printf("%d\n", priority);
        break;
    }

    // Получаем время создания потока (пример использования GetThreadTimes)
    FILETIME create_time, exit_time, kernel_time, user_time;
    if (GetThreadTimes(thread_handle, &create_time, &exit_time,
        &kernel_time, &user_time)) {
        printf("  Создан: %lu мс с момента загрузки системы\n",
            create_time.dwLowDateTime / 10000);
    }

    printf("\n");
}
```

---

## 🔧 **КОМПИЛЯЦИЯ И ЗАПУСК:**

### **В Visual Studio:**
1. Создайте **консольное приложение**
2. Настройте многопоточную библиотеку CRT: **C/C++ → Code Generation → Runtime Library → `/MT` или `/MTd`**
3. Вставьте код в `main.c`
4. Добавьте флаг `-D_WIN32_WINNT=0x0600` для поддержки Windows Vista+ API
5. Нажмите **Ctrl+F5** для запуска

### **Командная строка:**
```bash
cl lesson5.c /MT /D_WIN32_WINNT=0x0600 /link kernel32.lib user32.lib
```

### **Для Mingw/GCC:**
```bash
gcc lesson5.c -o lesson5.exe -D_WIN32_WINNT=0x0600 -lkernel32 -luser32 -lpthread
```

---

## 📋 **АНАЛИЗ ПРОГРАММЫ:**

1. **Почему используется `_beginthreadex()` вместо `CreateThread()`?**
   - `_beginthreadex` правильно инициализирует CRT для нового потока
   - Предотвращает утечки памяти и проблемы с локальными переменными потока (TLS)
   - Обеспечивает корректное освобождение ресурсов при завершении через `_endthreadex`

2. **Чем отличается критическая секция от мьютекса?**
   - Критические секции работают только в пределах одного процесса, быстрее
   - Мьютексы — объекты ядра, поддерживают межпроцессную синхронизацию и timeout
   - Критические секции не могут быть использованы для ожидания нескольких объектов

3. **Как работает событие (Event) с auto-reset vs manual-reset?**
   - Auto-reset: автоматически сбрасывается после пробуждения одного потока
   - Manual-reset: остается в сигнальном состоянии до явного сброса через `ResetEvent`
   - Auto-reset полезен для уведомления одного потока, manual-reset — для уведомления всех

4. **Что такое race condition и как его избежать?**
   - Гонка данных возникает при одновременном доступе к общим данным
   - Для защиты используйте: критические секции, мьютексы, атомарные операции
   - Пример гонки: `counter++` в многопоточном режиме без синхронизации

5. **Как правильно завершать потоки?**
   - Предпочтительно: поток завершается сам при возврате из функции
   - Избегайте `TerminateThread()` — он не освобождает ресурсы
   - Используйте события или флаги для корректного завершения

---

## 🎯 **ПРАКТИЧЕСКИЕ ЗАДАНИЯ ДЛЯ САМОСТОЯТЕЛЬНОЙ РАБОТЫ:**

### **Уровень 1: Модификация кода**
1. Добавьте поддержку **семафоров (Semaphore)** в программу
2. Реализуйте **пул потоков (Thread Pool)** с использованием событий
3. Добавьте **обработку исключений** в потоках через `__try/__except`

### **Уровень 2: Новые функции**
1. Создайте **производитель-потребитель (Producer-Consumer)** с кольцевым буфером
2. Реализуйте **читатели-писатели (Readers-Writers)** с приоритетом для читателей
3. Добавьте **deadlock detection** для критических секций и мьютексов

### **Уровень 3: Продвинутые задачи**
1. Реализуйте **асинхронный ввод/вывод (Async I/O)** с использованием потоков
2. Создайте **распараллеленную сортировку** (Merge Sort) с использованием потоков
3. Добавьте **мониторинг потоков** с отображением их состояния и стеков вызовов

---

## 🔍 **КРИТЕРИИ ПРОВЕРКИ:**

### **Обязательные требования:**
1. ✅ Программа компилируется без ошибок и предупреждений
2. ✅ Корректное создание и завершение потоков
3. ✅ Использование `_beginthreadex` вместо `CreateThread`
4. ✅ Реализованы все 6 функций демонстрации
5. ✅ Правильная работа механизмов синхронизации

### **Качество кода:**
1. ✅ Отсутствие утечек памяти и дескрипторов
2. ✅ Обработка ошибок создания потоков
3. ✅ Безопасный доступ к общим данным
4. ✅ Корректное освобождение ресурсов
5. ✅ Комментарии к сложным разделам кода

---

## 📞 **ФОРМАТ СДАЧИ:**

### **Что отправить на проверку:**
1. **Ответы** на 5 теоретических вопросов
2. **Исходный код** с выполненными заданиями
3. **Примеры работы** программы (скриншоты/логи)
4. **Анализ производительности** разных механизмов синхронизации
5. **Список проблем**, с которыми столкнулись при разработке

---

## 💡 **СОВЕТЫ ПО ВЫПОЛНЕНИЮ:**

1. **Всегда проверяйте возвращаемые значения** функций создания потоков
2. **Используйте отладчик** для анализа состояния потоков
3. **Тестируйте на разных конфигурациях** (количество ядер, версия Windows)
4. **Измеряйте производительность** перед оптимизацией
5. **Документируйте** предположения о потокобезопасности функций
6. **Используйте static analysis tools** для поиска гонок данных

---

## ⚠️ **ЧАСТЫЕ ОШИБКИ:**

1. **Утечка дескрипторов:** Не закрытие `HANDLE` после создания потока
2. **Гонки данных:** Доступ к общим переменным без синхронизации
3. **Deadlock:** Неправильный порядок захвата мьютексов
4. **Starvation:** Низкоприоритетные потоки никогда не выполняются
5. **Использование `TerminateThread`:** Не освобождает ресурсы потока
6. **Игнорирование `_beginthreadex`:** Проблемы с CRT в новых потоках

---

## 🔗 **ПОЛЕЗНЫЕ РЕСУРСЫ:**

1. **MSDN Threading API:** 
2. **Synchronization API:** https://docs.microsoft.com/en-us/windows/win32/sync/synchronization-functions
3. **Concurrency Runtime (ConcRT):** https://learn.microsoft.com/ru-ru/cpp/parallel/concrt/concurrency-runtime?view=msvc-170
4. **Intel Threading Building Blocks (TBB):** Кроссплатформенная библиотека параллелизма
5. **Стандарт C++11 `<thread>`:** Кроссплатформенная многопоточность

---

**Этот урок заложит фундамент для создания высокопроизводительных многопоточных приложений Windows. Понимание потоков и синхронизации критически важно для системного программирования, разработки серверов, игр и высоконагруженных приложений.**
