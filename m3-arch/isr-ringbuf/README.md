# M3 · isr-ringbuf — ISR-safe SPSC lock-free кільцевий буфер

> **Статус:** ✅ закрито
> **Що демонструє:** lock-free обмін «один пише ↔ один читає» на `_Atomic` з
> acquire/release-семантикою; відсутність гонок підтверджено **ThreadSanitizer**, а
> не очима. Це M2-SPSC, доведений до **ISR-safe**.
> **Залізо:** не потрібне — усе на хості (gcc + make).

## Мета
Зробити кільцевий буфер, який коректно працює, коли producer — це **ISR**, а
consumer — фонова задача (`main`), на двоядерному ESP32. Ключ — не лише структура
SPSC (вона була ще в M2), а **правильна модель пам'яті**: де `volatile` замало і
чому потрібні справжні атоміки з упорядкуванням.

## Теорія (стисло)
- **SPSC, один слот у жертву.** Корисна місткість = `size − 1`: один слот жертвуємо,
  щоб відрізнити повний (`(head+1)%size == tail`) від порожнього (`head == tail`).
  `head` пише лише producer, `tail` — лише consumer.
- **`volatile` ≠ упорядкування.** `volatile` забороняє кешувати значення в регістрі,
  але **не** дає міжпроцесорного порядку. На двоядерному цього мало.
- **acquire/release.** Producer пише дані у слот, потім робить **release-store**
  `head` — публікація («спершу дані, тоді індекс»). Consumer робить **acquire-load**
  `head` перед читанням слота — бачить дані, лише якщо побачив новий `head`. Свій
  власний індекс кожна сторона читає `relaxed` (його ніхто інший не пише). ⤳ той
  самий патерн у UART-драйвері (M5) і під RTOS (M7).

## Передумови
- `gcc` + `make`; для гонок — підтримка **ThreadSanitizer** (gcc на Linux).
- `cppcheck` для `make lint`: `sudo apt-get install -y cppcheck`.
- Vendored Unity у `../test/unity/` — спільний для host-проєктів M3 (закомічений у репо).

## Запуск (відтворюваний з нуля)
```bash
cd m3-arch/isr-ringbuf
make test       # Unity під ASan/UBSan
```
_Перевірка:_ `6 Tests 0 Failures 0 Ignored / OK` (порожній/повний, round-trip,
заповнення до місткості, pop з порожнього, FIFO через wraparound).

```bash
make tsan       # ThreadSanitizer, ASLR вимкнено (setarch -R) — лише Linux
```
_Перевірка:_ producer/consumer у двох потоках качають ~1 млн байтів; TSan **без**
data race, `TSAN SPSC: … FIFO ok, buffer drained`. На Windows (MSYS2) ціль чесно
повідомить, що TSan недоступний, і пропуститься.

```bash
make lint       # cppcheck
```
_Перевірка:_ без знахідок (exit 0).

## Артефакти
- `src/ringbuf.{h,c}` — SPSC на `_Atomic size_t` head/tail з `memory_order_*`.
- `test/test_ringbuf.c` — Unity (6 тестів).
- `test/tsan_ringbuf.c` — двопотоковий харнес під ThreadSanitizer.
- `Makefile` — цілі `test`, `tsan` (Linux-guard), `lint`, `clean`.

## Граблі / лог факапів
> Симптом → причину → фікс.
- **`make test` → «Нема правила для `../test/unity/unity.c`»** → причина: Makefile
  скопійований з `m2-c/frame-codec`, де `../test/unity` існує, а в M3 спільної теки
  Unity не було; фікс: вендорнув Unity у `m3-arch/test/unity/` — тепер `../test/unity`
  працює для всіх host-проєктів M3.
- **`make tsan` ламається на Windows (MSYS2)** → причина: MinGW не має `libtsan`, а
  `setarch` (вимкнути ASLR) — Linux-утиліта; фікс: ціль під `ifeq ($(shell uname -s),Linux)`
  — зрозуміле повідомлення замість падіння лінкера.

## Відомі обмеження (на потім)
- **`% size`** обрано заради ясності; embedded-перф-ідіома — місткість степінь-двійки
  + бітмаска (`& (size-1)`) замість ділення. ⤳ M5/M7.
- На реальному ESP32 байтовий `_Atomic size_t` lock-free (4 байти), тож модель
  переноситься; але справжню перевірку ISR↔task робимо вже на залізі (M5).

## CI
GitHub Actions ([.github/workflows/ci.yml](../../.github/workflows/ci.yml), джоб
`m3-arch`, ubuntu): `make test` (ASan/UBSan) + `make tsan` (TSan) + `make lint`
(cppcheck) на кожен push.

## Чекліст «працює»
- [x] 6 тестів зелені під ASan/UBSan
- [x] TSan без data race на двопотоковому харнесі
- [x] `cppcheck` без знахідок
- [x] у CI (джоб `m3-arch`)

## Джерела
- J. Preshing, «Acquire and Release Semantics» — модель пам'яті для lock-free SPSC.
- cppreference — `<stdatomic.h>`, `memory_order_*`.
- Unity — ThrowTheSwitch/Unity (host unit-test фреймворк).
