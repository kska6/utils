# Utils

ヘッダオンリーの共通ユーティリティ集です。

## 使い方

`include/` をインクルードパスに追加し、必要なヘッダだけを読み込みます。

```cpp
#include "UtilsTime.h"
#include "UtilsCSV.h"
#include "UtilsInput.h"
#include "UtilsLogger.h"
```

## 対応環境

- C++17 以上
- Windows / Unix 系の両方を想定

## 各ヘッダの使い方

### `UtilsTime.h`

経過時間、フレーム時間、ローカル時刻、Unix time、スリープを扱います。

```cpp
TimeManager::Initialize();
UpdateFrameTime();

const double elapsed_ms = GetCurrentTimeMs();
const TimeInfo info = GetTimeInfo();
SleepMilliseconds(16);
```

### `UtilsCSV.h`

数値配列の読み書きに使います。

```cpp
#include <vector>

std::vector<int> values = {1, 2, 3};
SaveCSVValue("data.csv", values, "value");

std::vector<int> loaded;
LoadCSVValue("data.csv", loaded, values.size());
```

`LoadCSVValue` は `index,value` 形式を読み込みます。`SaveCSVValue` は同形式で保存し、既存ファイルがある場合は `.bak` を作成します。

`UtilsCSV.h` のログ出力を `Logger` に接続する場合は、`UtilsCSV.h` を読む前に `UTILS_ENABLE_LOGGER` を定義してください。

```cpp
#define UTILS_ENABLE_LOGGER
#include "UtilsLogger.h"
#include "UtilsCSV.h"
```

### `UtilsInput.h`

キー入力を取得します。関数は `input_ns` 名前空間にあります。

```cpp
if (input_ns::KbHit()) {
    const int ch = input_ns::GetCh();
}
```

### `UtilsLogger.h`

コンソール出力、ファイル出力、ログレベル制御、バッファリングに使います。

```cpp
Logger::SetLogLevel(LogLevel::LL_INFO, LogLevel::LL_DEBUG_VERBOSE);
Logger::SetLogFileTimestamped("logs/app");
Logger::Info("start");
Logger::Warning("warning");
Logger::Error("error");
```

フレーム時間を出力する場合は `TimeInfo` を渡します。

```cpp
TimeManager::UpdateFrameTime();
Logger::TimePrint(GetTimeInfo());
```

## 最小例

```cpp
#include "UtilsTime.h"
#include "UtilsLogger.h"

int main() {
    TimeManager::Initialize();
    Logger::SetLogFileTimestamped("logs/app");
    Logger::Info("hello");
    return 0;
}
```

## 参照ファイル

- [include/UtilsCSV.h](/Users/kska6/kska6local/src/github.com/kska6/utils/include/UtilsCSV.h)
- [include/UtilsInput.h](/Users/kska6/kska6local/src/github.com/kska6/utils/include/UtilsInput.h)
- [include/UtilsLogger.h](/Users/kska6/kska6local/src/github.com/kska6/utils/include/UtilsLogger.h)
- [include/UtilsTime.h](/Users/kska6/kska6local/src/github.com/kska6/utils/include/UtilsTime.h)
