Отлично — задача понятна. Ниже — полная архитектура, логика и идеи реализации, которые позволят вам быстро перейти к коду. Я даю: общий дизайн, последовательность действий, предложения по потоковой обработке больших файлов, API DLL, структуру проекта / CMake, примеры ключевых функций (псевдо/реальный код), идеи логирования и юнит-тестов.

Я буду писать по-русски и структурированно — можно сразу брать и реализовывать.

---

# 1. Коротко — идея в двух предложениях

1. Рекурсивно обойти директорию (std::filesystem), собрать пути файлов.
2. Для каждого файла потоково (чанками) вычислить MD5, сверить с таблицей вредоносных хешей (загруженной в `unordered_map` из CSV). Всё параллельно: продьюсер (обход) + пул воркеров (вычисление MD5 + проверка) + отдельный поток логирования для записи в файл. Итог — отчёт с атомарными счетчиками и время работы.

---

# 2. Компоненты / архитектура

Проект делится на 2 компонента, как сказано в ТЗ:

1. **DLL** — основная логика сканирования (реализует API для запуска сканирования, конфигурацию, колбэки для прогресса).

   * Экспортирует функцию(и): `ScanResult scan(const ScanConfig& cfg)` или C API `extern "C" ScanResult scan(...)`.
   * Содержит: CSV-парсер, коллекцию вредоносных хешей, пул потоков, вычисление MD5, логгер.

2. **CLI-приложение (exe)** — парсит аргументы командной строки (`--base`, `--log`, `--path`) и вызывает DLL. Отвечает за вывод финального отчёта в консоль.

Дополнительно: `tests/` с GoogleTest покрывают ключевые части (парсер CSV, compute\_md5 на мелких файлах, поведение при ошибке чтения, интеграционный тест с временными файлами).

---

# 3. Основные сущности (C++ классы / структуры)

* `struct ScanConfig { std::filesystem::path baseCsv; std::filesystem::path rootPath; std::filesystem::path logPath; size_t threadCount; bool followSymlinks; /*etc*/ };`
* `struct ScanResult { uint64_t totalFiles; uint64_t maliciousFound; uint64_t errors; std::chrono::duration<double> elapsed; };`
* `class MalwareDB` — загружает CSV в `unordered_map<std::string, std::string>` (md5 -> verdict).
* `class ThreadSafeQueue<T>` — многопоточная очередь для продьюсер→консьюмер.
* `class Logger` — асинхронный логгер: очередь сообщений + писатель в отдельном потоке.
* `class Scanner` — основной менеджер: запускает продьюсер (directory walker), стартует N воркеров, собирает счётчики, возвращает `ScanResult`.

---

# 4. Потоковая модель и синхронизация

* **Producer**: один поток обходит файловую систему (`std::filesystem::recursive_directory_iterator`) и пушит пути в `ThreadSafeQueue<std::filesystem::path>`.

  * Если файл недоступен для чтения, увеличивает `errors` и пишет лог ошибки (через Logger).

* **Worker pool**: `N = std::thread::hardware_concurrency()` (или `cfg.threadCount`) воркеров. Каждый воркер:

  1. Берёт путь из очереди.
  2. Открывает файл в бинарном режиме, читает чанками (напр., 1 MiB) и обновляет MD5-контекст.
  3. Получает строковый hex-формат MD5.
  4. Сверяет с `MalwareDB` (O(1)).
  5. Если найдено — увеличивает `maliciousFound` и записывает лог-позицию через Logger (путь, hash, verdict).
  6. Всегда увеличивает `totalFiles` (атомарно).
  7. В случае ошибок чтения/IO — увеличивает `errors`.

* **Log thread**: отдельно читает очередь лог-сообщений и пишет в файл, чтобы избежать блокировок на синхронный файл-IO от множества воркеров.

* **Завершение**: продьюсер помечает очередь как закрыт (`close()`), воркеры заканчивают при пустой и закрытой очереди. Затем логгер флашит и завершает.

Используем атомарные счётчики: `std::atomic<uint64_t> totalFiles, maliciousFound, errors;`.

---

# 5. MD5 для больших файлов (стриминг)

Файлы могут быть больше RAM — поэтому вычисляем MD5 чанками.

Рекомендуемые варианты:

* Использовать OpenSSL (`EVP_DigestInit`, `EVP_DigestUpdate`, `EVP_DigestFinal`) — кроссплатформенно, быстрый.
* Если не хочется завязываться на OpenSSL — включить небольшой public-domain/скалярный MD5-реализацию (обычно \~300 строк) в проект.

Пример логики (псевдо/реальный код):

```cpp
std::string compute_md5_hex(const std::filesystem::path& p) {
    constexpr size_t BUF_SIZE = 1024 * 1024; // 1 MiB
    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("open failed");

    MD5_CTX ctx;
    MD5_Init(&ctx);

    std::vector<char> buf(BUF_SIZE);
    while (in) {
        in.read(buf.data(), buf.size());
        std::streamsize n = in.gcount();
        if (n > 0) MD5_Update(&ctx, buf.data(), static_cast<size_t>(n));
    }

    unsigned char digest[16];
    MD5_Final(digest, &ctx);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) oss << std::setw(2) << static_cast<int>(digest[i]);
    return oss.str();
}
```

(здесь `MD5_` — часть OpenSSL или любой MD5-реализации)

---

# 6. Парсер CSV (маленький, безопасный)

CSV формат: `hash;Verdict\n`. Нужно:

* Читать файл как текст, построчно.
* Для каждой строки: trim, split по `;`, проверить длину хеша (32 hex-символа для MD5).
* Поместить в `unordered_map<std::string, std::string>`.

Если строка некорректна, логировать предупреждение и продолжить.

---

# 7. Формат лог-файла

Требуется логгировать минимум: путь, hash, verdict.

Например, CSV-лог:

```
timestamp;path;md5;verdict
2025-09-13T12:00:00Z;C:\folder\evil.exe;a9963513d093ffb2bc7ceb9807771ad4;Exploit
```

Преимущество — легко парсить позже.

---

# 8. DLL API (пример)

C++ интерфейс (советую экспортировать C API для простоты использования из exe):

```cpp
// dll_api.h (export)
#ifdef _WIN32
  #ifdef BUILD_DLL
    #define DLL_API __declspec(dllexport)
  #else
    #define DLL_API __declspec(dllimport)
  #endif
#else
  #define DLL_API
#endif

extern "C" {
  struct DLL_API ScanConfig {
    const char* baseCsv;
    const char* rootPath;
    const char* logPath;
    int threadCount; // 0 => auto
  };

  struct DLL_API ScanResult {
    uint64_t totalFiles;
    uint64_t maliciousFound;
    uint64_t errors;
    double elapsedSeconds;
  };

  DLL_API ScanResult run_scan(const ScanConfig* cfg);
}
```

Реализация `run_scan` внутри DLL создаёт `Scanner` и возвращает `ScanResult`. При ошибках — можно возвращать `errors` > 0 и логировать подробности.

---

# 9. CLI (exe) — парсинг аргументов

Простой вариант — ручной разбор `argv` (3 параметра), или использовать `cxxopts`/`CLI11` (одно-загруженные заголовки). После парсинга:

* Сформировать `ScanConfig cfg`, загрузить DLL (LoadLibrary/GetProcAddress на Windows) или линковать с DLL .lib, вызвать `run_scan`.
* По результату вывести в консоль красиво:

```
Scan finished:
Total files: 12345
Malicious: 12
Errors: 3
Elapsed: 12.34 s
Full log: C:\path\report.log
```

---

# 10. Юнит-тесты (gtest)

Покрыть:

* `MalwareDB` — корректное чтение CSV, плохо форматированные строки.
* `compute_md5_hex` — тест на маленьких файлах (создавать временные файлы с known content).
* `Scanner` отдельно: интеграционный тест с временной папкой, где генерируются файлы, один файл с известным хешем из тестовой базы — проверяем, что `maliciousFound==1`.
* Тест на сценарий «файл без прав на чтение» — на Unix можно chmod, на Windows потребуются специальные атрибуты; можно смоделировать, бросая исключение при чтении в тестовой реализации.

Используйте `TemporaryDirectory` helpers в тестах.

---

# 11. CMake структура (пример)

```
/CMakeLists.txt         # top-level
 /app/                 # CLI exe
   CMakeLists.txt
 /scanner_dll/         # DLL code
   CMakeLists.txt
 /third_party/         # optional: md5 or CLI parser
 /tests/               # gtest tests
   CMakeLists.txt
```

`top-level CMakeLists.txt` подключает subdirs и добавляет опции: `-DBUILD_TESTS=ON`, `-DUSE_OPENSSL=ON`.

Если используете OpenSSL: `find_package(OpenSSL REQUIRED)` и линковать `OpenSSL::Crypto`.

---

# 12. Производительность и тонкости

* **Чанк**: 1 MiB — хорошее значение по умолчанию; можно экспериментировать.
* **IO-bound vs CPU-bound**: MD5 — CPU, чтение — IO. На современных SSD выгодно иметь `N = hardware_concurrency()` воркеров, но можно ограничить (например `min(2*cores, 32)`). Делать configurable.
* **Память**: читаем чанками, не загружаем весь файл.
* **Права доступа**: при ошибке `open()` — фиксировать как `errors` и логировать подробности.
* **Символьные ссылки / циклы**: при рекурсивном обходе можно столкнуться с рекурсией через ссылки — предусмотреть `followSymlinks` опцию и ограничение глубины либо набор уже посещённых inode (на Unix) / reparse points (Windows) — при необходимости.
* **Atomic counters** и `std::mutex` для общей структуры (например, при необходимости аккумулировать список найденных файлов — избегать хранения большого списка в памяти; лучше логировать на диск сразу).

---

# 13. Примеры кода (скелет)

`ThreadSafeQueue` (упрощённо):

```cpp
template<typename T>
class TSQueue {
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
    bool closed = false;
public:
    void push(T v) {
        { std::lock_guard lk(m); q.push(std::move(v)); }
        cv.notify_one();
    }
    // pop returns false if closed and empty
    bool pop(T& out) {
        std::unique_lock lk(m);
        cv.wait(lk, [&]{ return !q.empty() || closed; });
        if (q.empty()) return false;
        out = std::move(q.front()); q.pop();
        return true;
    }
    void close() {
        { std::lock_guard lk(m); closed = true; }
        cv.notify_all();
    }
};
```

Worker loop (упрощённо):

```cpp
void worker_loop(TSQueue<std::filesystem::path>& q,
                 MalwareDB& db,
                 Logger& logger,
                 std::atomic<uint64_t>& total,
                 std::atomic<uint64_t>& malicious,
                 std::atomic<uint64_t>& errors)
{
    std::filesystem::path p;
    while (q.pop(p)) {
        try {
            auto md5 = compute_md5_hex(p); // может throw
            total.fetch_add(1, std::memory_order_relaxed);
            auto it = db.find(md5);
            if (it != db.end()) {
                malicious.fetch_add(1, std::memory_order_relaxed);
                logger.log_malicious(p, md5, it->second);
            }
        } catch (const std::exception& e) {
            errors.fetch_add(1, std::memory_order_relaxed);
            logger.log_error(p, e.what());
        }
    }
}
```

---

# 14. Логирование — советы

* Асинхронный логгер: `Logger::log_malicious(path, md5, verdict)` пушит структуру в очередь.
* Формат: `ISO8601;path;md5;verdict\n`.
* Не забыть защиту от одновременных записей (в логгере одна нить делает fwrite/ostream).

---

# 15. Возможные расширения (если понадобится)

* Опция `--dry-run`.
* Поддержка SHA256/MD5 выбор (в CSV указывать тип хеша или фиксировать MD5).
* Поддержка ZIP/архивов (сканирование вложенных файлов) — это сложнее.
* GUI/отчёт в JSON.
* DB для хешей (например, LevelDB) если база огромная.

---

# 16. План реализации (итерации)

1. Определить структуры и интерфейс DLL (`dll_api.h`).
2. Реализовать `MalwareDB` — CSV loader + unit tests.
3. Реализовать `compute_md5_hex` (используя OpenSSL или встроенный md5) + тест.
4. Сделать `TSQueue`, `Logger`.
5. Реализовать `Scanner` (producer + workers + logger) + интеграционный тест.
6. Сборка DLL и exe через CMake.
7. Тесты gtest, CI (опционально).
8. Документация, инструкции по запуску (пример вызова, примеры CSV/log).

---

# 17. Пример CLI запуска (как в ТЗ)

```
scanner.exe --base base.csv --log report.log --path C:\folder --threads 8
```

---

# 18. Замечания по Windows / DLL

* Для экспорта символов — `__declspec(dllexport)` при сборке DLL. Для импорта — `__declspec(dllimport)`. Удобнее писать `extern "C"` с простыми структурами, чтобы exe мог легко обращаться к API.
* Или просто скомпилировать DLL и линковать статически (lib), тогда exe вызывает функции напрямую.
* Путь к лог-файлу убедитесь, что у процесса есть права на запись.

---

# 19. Что я могу сделать дальше (если хотите)

* Могу написать шаблонный/реальный код ключевых файлов: `MalwareDB.cpp`, `scanner.cpp`, `logger.cpp`, `dll_api.cpp` — примерно \~200–500 строк (включая простой MD5 реал).
* Могу подготовить `CMakeLists.txt` и пример теста gtest.
* Могу дать готовую маленькую реализацию compute\_md5 без внешних зависимостей (встроенный md5.c), чтобы не подключать OpenSSL.

Напишите, что вам удобнее: готовый код-скелет DLL + exe + CMake, или только несколько ключевых файлов/функций. Я сразу выдам код — никаких ожиданий или "подождите" (всё делаю в текущем ответе).
