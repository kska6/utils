# UtilsTime.h — 時間ユーティリティ

## 概要

`include/UtilsTime.h` はヘッダオンリーの時間管理ユーティリティです。  
`std::chrono` を使ったクロスプラットフォーム実装で、以下の機能を提供します。

- アプリ起動からの経過時間の取得（秒・ミリ秒）
- メインループ向けのフレーム差分時間（デルタタイム）管理
- ローカル時刻・Unix タイムスタンプの取得
- クロスプラットフォーム対応スリープ

**必要な C++ バージョン**: C++17 以上

---

## データ構造

### `struct TimeInfo`

| フィールド | 型 | 説明 |
|---|---|---|
| `minutes` | `int` | 経過時間の「分」部分 |
| `seconds` | `int` | 経過時間の「秒」部分（0〜59） |
| `milliseconds` | `int` | 経過時間の「ミリ秒」部分（0〜999） |
| `delta_time_ms` | `double` | 前回の `UpdateFrameTime()` 呼び出しからの差分時間（ミリ秒） |
| `frequency_hz` | `int` | `delta_time_ms` から算出した更新周波数（Hz） |

---

## クラス: `TimeManager`

シングルトン的な静的クラス。`Initialize()` を一度呼び出してから各関数を使います。

### 静的メンバ関数

| 関数 | 戻り値 | 説明 |
|---|---|---|
| `Initialize()` | `void` | 内部タイマーをリセットして計測を開始する |
| `GetElapsedTimeSec()` | `double` | 起動からの経過時間（秒） |
| `GetElapsedTimeMs()` | `double` | 起動からの経過時間（ミリ秒） |
| `UpdateFrameTime()` | `void` | フレーム差分時間を更新する。メインループで毎フレーム呼ぶ |
| `MakeTimeInfo()` | `TimeInfo` | 現在の経過時間と差分時間をまとめた `TimeInfo` を返す |
| `GetLastFrameTimeMs()` | `double` | 前回 `UpdateFrameTime()` を呼んだ時点の経過時間（ミリ秒） |
| `GetLastDeltaTimeMs()` | `double` | 前回フレームとの差分時間（ミリ秒） |

---

## グローバル関数

`TimeManager` のラッパー関数として提供されます。

| 関数 | 戻り値 | 説明 |
|---|---|---|
| `GetCurrentTimeSec()` | `double` | 起動からの経過時間（秒） |
| `GetCurrentTimeMs()` | `double` | 起動からの経過時間（ミリ秒） |
| `UpdateFrameTime()` | `void` | フレーム差分時間を更新 |
| `GetTimeInfo()` | `TimeInfo` | 現在の `TimeInfo` を返す |
| `GetCurrentLocalTime()` | `struct tm` | 現在のローカル時刻（カレンダー時刻） |
| `GetCurrentUnixTimeMs()` | `long long` | Unix タイムスタンプ（ミリ秒） |
| `SleepMilliseconds(int ms)` | `void` | 指定ミリ秒だけスリープ |

---

## サンプルプログラム

### 基本：起動からの経過時間を取得する

```cpp
#include "UtilsTime.h"
#include <cstdio>

int main() {
    TimeManager::Initialize();

    printf("開始\n");
    SleepMilliseconds(500);
    printf("経過時間: %.3f 秒\n", GetCurrentTimeSec());
    printf("経過時間: %.1f ms\n", GetCurrentTimeMs());
    return 0;
}
```

---

### ループ：一定間隔でデルタタイムを計測する

メインループで `UpdateFrameTime()` を毎フレーム呼び出し、`GetTimeInfo()` でフレーム情報を取得します。

```cpp
#include "UtilsTime.h"
#include <cstdio>

int main() {
    TimeManager::Initialize();

    // 10 フレーム分ループして差分時間を表示する
    for (int frame = 0; frame < 10; ++frame) {
        UpdateFrameTime();           // 差分時間を更新（毎フレーム必須）
        TimeInfo info = GetTimeInfo();

        printf("frame %2d | %02dm %02ds %03dms | dt=%.3fms | %dHz\n",
            frame,
            info.minutes, info.seconds, info.milliseconds,
            info.delta_time_ms,
            info.frequency_hz);

        SleepMilliseconds(16);       // ~60fps を模擬
    }
    return 0;
}
```

出力例:
```
frame  0 | 00m 00s 000ms | dt=0.000ms | 0Hz
frame  1 | 00m 00s 016ms | dt=16.123ms | 61Hz
frame  2 | 00m 00s 032ms | dt=16.089ms | 62Hz
...
```

---

### ループ：指定秒数だけ動作し続けるタイマーループ

```cpp
#include "UtilsTime.h"
#include <cstdio>

int main() {
    TimeManager::Initialize();

    const double run_seconds = 3.0;   // 3 秒間ループする

    while (GetCurrentTimeSec() < run_seconds) {
        UpdateFrameTime();
        TimeInfo info = GetTimeInfo();

        printf("経過 %02d:%02d.%03d  dt=%.2fms\n",
            info.minutes, info.seconds, info.milliseconds,
            info.delta_time_ms);

        SleepMilliseconds(100);
    }

    printf("%.1f 秒経過。終了します。\n", GetCurrentTimeSec());
    return 0;
}
```

---

### ループ：処理時間を計測してログに出す

```cpp
#include "UtilsTime.h"
#include <cstdio>
#include <vector>
#include <numeric>

// 重い処理の代わりに数値を合計する例
static long long heavy_work(int n) {
    std::vector<long long> v(n);
    std::iota(v.begin(), v.end(), 0);
    return std::accumulate(v.begin(), v.end(), 0LL);
}

int main() {
    TimeManager::Initialize();

    const int iterations = 5;
    for (int i = 0; i < iterations; ++i) {
        double t_start = GetCurrentTimeMs();

        long long result = heavy_work(1000000);

        double t_end = GetCurrentTimeMs();
        printf("iter %d: result=%lld, 処理時間=%.3fms\n",
            i, result, t_end - t_start);
    }
    return 0;
}
```

---

### ローカル時刻・Unix タイムスタンプの取得

```cpp
#include "UtilsTime.h"
#include <cstdio>

int main() {
    struct tm local = GetCurrentLocalTime();
    printf("現在時刻: %04d-%02d-%02d %02d:%02d:%02d\n",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec);

    long long unix_ms = GetCurrentUnixTimeMs();
    printf("Unix タイムスタンプ (ms): %lld\n", unix_ms);
    return 0;
}
```

---

## 注意事項

- `TimeManager::Initialize()` を呼ぶ前に `GetCurrentTimeMs()` などを呼ぶとゼロ付近の値が返ります。  
  アプリ起動直後に必ず `Initialize()` を呼んでください。
- `UpdateFrameTime()` はメインループの**先頭**で一度だけ呼んでください。  
  同一フレーム内で複数回呼ぶと差分時間が正しく計算されません。
- `SleepMilliseconds` は OS スケジューラに依存するため、指定時間より実際の待機時間が長くなることがあります。
