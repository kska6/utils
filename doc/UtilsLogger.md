# UtilsLogger.h — ロギングユーティリティ

## 概要

`include/UtilsLogger.h` はヘッダオンリーのロギングユーティリティです。  
`UtilsTime.h` に依存します。

- ログレベルによるフィルタリング（ERROR / WARNING / INFO / DEBUG / DEBUG_VERBOSE）
- コンソール（stdout）とファイルへの同時出力
- タイムスタンプ・ログレベルタグの付与
- ANSI カラー出力対応
- ログのバッファリングと定期フラッシュ
- タイムスタンプ付きログファイル名の自動生成
- 同一の `Logger` インスタンスに対する出力と設定変更は、並行利用を前提に扱えます。

**必要な C++ バージョン**: C++17 以上  
**依存ヘッダ**: `UtilsTime.h`

---

## ログレベル

```cpp
enum class LogLevel {
    LL_ERROR          = 0,  // エラー（赤）
    LL_WARNING        = 1,  // 警告（黄）
    LL_INFO           = 2,  // 情報（白）
    LL_DEBUG          = 3,  // デバッグ（グレー）
    LL_DEBUG_VERBOSE  = 4   // 詳細デバッグ（グレー）
};
```

コンソールとファイルでそれぞれ独立したレベルを設定できます。  
デフォルトはコンソール: `LL_INFO`、ファイル: `LL_DEBUG_VERBOSE`。

実際のログ出力では、各レベルは次のタグで表示されます。

| レベル | 表示タグ |
|---|---|
| `LL_ERROR` | `[ERROR]` |
| `LL_WARNING` | `[WARN]` |
| `LL_INFO` | `[INFO]` |
| `LL_DEBUG` | `[DEBUG]` |
| `LL_DEBUG_VERBOSE` | `[TRACE]` |

> **注意**: `LL_DEBUG_VERBOSE` の表示タグは `[TRACE]` です（`DEBUG_VERBOSE` ではありません）。

---

## クラス: `Logger`

シングルトンパターンで実装されています。すべての操作は静的メソッドで行います。

### ログ出力関数

| 関数 | レベル | 説明 |
|---|---|---|
| `Logger::Error(fmt, ...)` | ERROR | エラーを出力（ログファイルを開いている場合は常にファイルにも出力） |
| `Logger::Warning(fmt, ...)` | WARNING | 警告を出力 |
| `Logger::Info(fmt, ...)` | INFO | 情報を出力 |
| `Logger::Debug(fmt, ...)` | DEBUG | デバッグ情報を出力 |
| `Logger::DebugVerbose(fmt, ...)` | DEBUG_VERBOSE | 詳細デバッグ情報を出力 |
| `Logger::Log(level, fmt, ...)` | 任意 | レベルを直接指定して出力 |
| `Logger::TimePrint(timeInfo)` | DEBUG | `TimeInfo` の内容をデバッグ出力 |
| `Logger::BlankLine()` | — | 空行を出力 |

#### Force 系関数

バッファリングが有効な場合でも即座に出力します。

| 関数 | 説明 |
|---|---|
| `Logger::ForceWarning(fmt, ...)` | バッファをスキップして WARNING を即出力 |
| `Logger::ForceInfo(fmt, ...)` | バッファをスキップして INFO を即出力 |
| `Logger::ForceDebug(fmt, ...)` | バッファをスキップして DEBUG を即出力 |
| `Logger::ForceDebugVerbose(fmt, ...)` | バッファをスキップして DEBUG_VERBOSE を即出力 |
| `Logger::ForceBlankLine()` | バッファをスキップして空行を即出力 |

---

### 設定関数

| 関数 | 説明 |
|---|---|
| `Logger::SetLogLevel(console_level, file_level)` | コンソールとファイルのログレベルを設定 |
| `Logger::GetLogLevel()` | 現在のコンソールログレベルを取得 |
| `Logger::SetLogFile(path)` | ログファイルのパスを指定して開く |
| `Logger::SetLogFileTimestamped(basePath)` | タイムスタンプ付きファイル名でログファイルを開く |
| `Logger::CloseLogFile()` | ログファイルを閉じる |
| `Logger::SetTimestampEnabled(bool)` | タイムスタンプの表示をオン/オフ |
| `Logger::SetLevelTagEnabled(bool)` | ログレベルタグの表示をオン/オフ |
| `Logger::SetColorEnabled(bool)` | ANSI カラー出力をオン/オフ |

---

### バッファリング関数

| 関数 | 説明 |
|---|---|
| `Logger::SetBufferingEnabled(bool)` | バッファリングをオン/オフ |
| `Logger::FlushBuffer()` | バッファ内のログを即座に出力 |
| `Logger::ClearBuffer()` | バッファ内のログを破棄 |
| `Logger::SetFlushIntervalMs(long long ms)` | バッファフラッシュの間隔（ミリ秒）を設定 |
| `Logger::GetFlushIntervalMs()` | 現在のフラッシュ間隔を取得 |
| `Logger::FlushOrClearBuffer(long long ms)` | フラッシュ間隔が経過していればバッファをフラッシュし、未到達の場合はバッファをクリア |

---

### タイミング取得関数

| 関数 | 戻り値 | 説明 |
|---|---|---|
| `Logger::GetLastOutputTimeMs()` | `long long` | 最後に通常ログを出力した時刻（Unix ms） |
| `Logger::GetLastAnyOutputTimeMs()` | `long long` | 最後にいずれかのログを出力した時刻（Unix ms） |

---

## ログファイルのタイムスタンプ命名規則

`SetLogFileTimestamped("logs/app")` を呼ぶと、以下のようなファイルが生成されます。

```
logs/app_20260424_163612.log
```

`basePath` に拡張子がある場合（例: `"logs/app.txt"`）は、ステム直後にタイムスタンプが挿入されます。

```
logs/app_20260424_163612.txt
```

---

## 出力フォーマット

```
HH:MM:SS [LEVEL] メッセージ
```

例:
```
16:36:12 [INFO] アプリを開始します
16:36:12 [WARN] 設定ファイルが見つかりません
16:36:12 [ERROR] ファイルの読み込みに失敗しました
```

---

## サンプルプログラム

### 基本：ログレベルを設定して各レベルで出力する

```cpp
#include "UtilsLogger.h"

int main() {
    // コンソール: INFO 以上、ファイル: DEBUG_VERBOSE 以上
    Logger::SetLogLevel(LogLevel::LL_INFO, LogLevel::LL_DEBUG_VERBOSE);
    Logger::SetLogFileTimestamped("logs/app");

    Logger::Info("アプリを開始します");
    Logger::Warning("警告: 設定が未指定です");
    Logger::Error("エラー: ファイルが見つかりません");
    Logger::Debug("デバッグ情報（コンソールには表示されない）");
    Logger::DebugVerbose("詳細デバッグ（コンソールには表示されない）");

    Logger::CloseLogFile();
    return 0;
}
```

---

### ループ：ループ処理の進捗をログに出す

```cpp
#include "UtilsLogger.h"
#include <vector>

int main() {
    Logger::SetLogLevel(LogLevel::LL_DEBUG, LogLevel::LL_DEBUG_VERBOSE);
    Logger::SetLogFileTimestamped("logs/loop");

    std::vector<int> data = {10, 25, 3, 47, 8, 99, 56, 14};

    Logger::Info("処理開始: %zu 件", data.size());

    int sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
        Logger::Debug("data[%zu] = %d, 累計 = %d", i, data[i], sum);
    }

    Logger::Info("処理完了: 合計 = %d", sum);
    Logger::CloseLogFile();
    return 0;
}
```

---

### ループ：フレームループでフレーム時間を出力する

`Logger::TimePrint()` を使ってフレームごとの時間情報を定期ログする例です。

```cpp
#include "UtilsLogger.h"
#include "UtilsTime.h"

int main() {
    TimeManager::Initialize();
    Logger::SetLogLevel(LogLevel::LL_DEBUG, LogLevel::LL_DEBUG_VERBOSE);
    Logger::SetLogFileTimestamped("logs/frame");

    const int total_frames = 60;
    for (int frame = 0; frame < total_frames; ++frame) {
        UpdateFrameTime();
        TimeInfo info = GetTimeInfo();

        Logger::TimePrint(info);            // フレーム時間を DEBUG で出力

        if (frame % 10 == 0) {
            Logger::Info("frame %d / %d", frame, total_frames);
        }

        SleepMilliseconds(16);
    }

    Logger::CloseLogFile();
    return 0;
}
```

---

### ループ：バッファリングを使って高頻度ログをまとめて出力する

高頻度でログを出力する場合、バッファリングを有効にしてまとめてフラッシュすることで  
パフォーマンスへの影響を抑えられます。

```cpp
#include "UtilsLogger.h"
#include "UtilsTime.h"

int main() {
    TimeManager::Initialize();
    Logger::SetLogLevel(LogLevel::LL_DEBUG_VERBOSE, LogLevel::LL_DEBUG_VERBOSE);
    Logger::SetLogFileTimestamped("logs/buffered");
    Logger::SetBufferingEnabled(true);
    Logger::SetFlushIntervalMs(200);   // 200ms ごとにフラッシュ

    const long long flush_check_interval_ms = 250;   // 250ms ごとにフラッシュ判定
    long long last_flush_check_ms = GetCurrentUnixTimeMs();

    for (int frame = 0; frame < 300; ++frame) {
        UpdateFrameTime();

        Logger::DebugVerbose("frame %d", frame);

        long long now = GetCurrentUnixTimeMs();
        if (now - last_flush_check_ms >= flush_check_interval_ms) {
            Logger::FlushOrClearBuffer(now);
            last_flush_check_ms = now;
        }

        SleepMilliseconds(8);
    }

    // 残ったバッファを出力
    Logger::FlushBuffer();
    Logger::CloseLogFile();
    return 0;
}
```

---

### ループ：エラー発生時にループを中断してログに記録する

```cpp
#include "UtilsLogger.h"
#include <vector>
#include <string>

static bool process_item(int value) {
    return value >= 0;   // 負の値はエラー
}

int main() {
    Logger::SetLogLevel(LogLevel::LL_INFO, LogLevel::LL_DEBUG_VERBOSE);
    Logger::SetLogFileTimestamped("logs/error_check");

    std::vector<int> items = {5, 12, -3, 7, 20};
    bool ok = true;

    for (size_t i = 0; i < items.size(); ++i) {
        if (!process_item(items[i])) {
            Logger::Error("items[%zu] = %d: 不正な値です", i, items[i]);
            ok = false;
            break;
        }
        Logger::Info("items[%zu] = %d: OK", i, items[i]);
    }

    if (ok) {
        Logger::Info("全件処理完了");
    } else {
        Logger::Warning("処理を中断しました");
    }

    Logger::CloseLogFile();
    return 0;
}
```

---

### タイムスタンプ・カラーを無効にする

```cpp
#include "UtilsLogger.h"

int main() {
    Logger::SetTimestampEnabled(false);  // タイムスタンプなし
    Logger::SetLevelTagEnabled(false);   // レベルタグなし
    Logger::SetColorEnabled(false);      // カラーなし

    Logger::Info("シンプルな出力");
    Logger::Warning("警告");
    return 0;
}
```

---

## 注意事項

- `Logger` はシングルトンです。複数インスタンスを生成することはできません（コピー・ムーブ禁止）。
- ログファイルは `SetLogFile` または `SetLogFileTimestamped` で明示的に開いてください。  
  開かなければコンソールのみに出力されます。
- アプリ終了時には `CloseLogFile()` を呼ぶか、デストラクタで自動クローズされます。
- `ForceXxx` 系関数はバッファリングを無視して即座に出力します。  
  重要なメッセージや最終ステータスの出力に使ってください。
- バッファリングを有効にした場合、`FlushBuffer()` を呼ばないとログが出力されないことがあります。  
  アプリ終了前には必ず `FlushBuffer()` を呼んでください。
