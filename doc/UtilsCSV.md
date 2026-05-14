# UtilsCSV.h — CSV 読み書きユーティリティ

## 概要

`include/UtilsCSV.h` はヘッダオンリーの CSV 入出力ユーティリティです。  
数値配列（`std::vector<T>`）を `index,value` 形式の CSV ファイルに保存・読み込みできます。
また、計測ログ向けに、列番号・ヘッダ名・値の取得元を明示した複数列 CSV のヘッダ行とデータ行を書き込めます。

- 任意の数値型をテンプレート引数で指定可能（`int`, `double`, `float` など）
- 保存時に既存ファイルを `.bak` にバックアップする安全な書き込み
- ヘッダ行のカスタマイズ、ヘッダ有無の切り替えに対応
- 計測ログ用に、時刻列・値配列の各要素・固定文字列を任意の列へ割り当て可能
- `UTILS_ENABLE_LOGGER` マクロで `UtilsLogger.h` のログ出力に統合可能

**必要な C++ バージョン**: C++17 以上（`<filesystem>` を使用）

---

## CSV フォーマット

```
id,<header>
0,<value0>
1,<value1>
2,<value2>
...
```

- 1 行目はヘッダ行（`id,<ヘッダ名>`）
- 2 行目以降は `index,value` の形式
- インデックスは 0 始まり

---

## 関数リファレンス

### `LoadCSVValue`

```cpp
template <typename T>
bool LoadCSVValue(
    const std::string& path,       // 読み込むファイルパス
    std::vector<T>& values,        // 読み込み結果を格納するベクター
    size_t expected_size,          // 期待する要素数
    bool header_exists = true      // ヘッダ行をスキップするか
);
```

| 引数 | 説明 |
|---|---|
| `path` | CSV ファイルのパス |
| `values` | 読み込み結果を受け取る `vector`。成功時に上書きされる |
| `expected_size` | 期待する要素数。全インデックスが揃わない場合は `false` を返す |
| `header_exists` | `true` の場合、最初の行をヘッダとしてスキップする（デフォルト `true`） |

**戻り値**: 成功時 `true`、ファイルが開けない・インデックス不足の場合 `false`

---

### `SaveCSVValue`

```cpp
template <typename T>
bool SaveCSVValue(
    const std::string& path,             // 保存先ファイルパス
    const std::vector<T>& values,        // 保存する値
    const std::string& header            // 値列のヘッダ名（空なら "value"）
);
```

| 引数 | 説明 |
|---|---|
| `path` | 出力先ファイルパス |
| `values` | 保存する値の `vector` |
| `header` | 2 列目（値列）のヘッダ名 |

**戻り値**: 成功時 `true`、書き込み失敗時 `false`

**バックアップ動作**:
1. まず `.tmp` ファイルに書き込む
2. 既存ファイルがあれば `.bak` にリネーム
3. `.tmp` を最終パスにリネーム

---

### `EscapeCSVField` (内部ヘルパー)

```cpp
static inline std::string EscapeCSVField(const std::string& field);
```

CSV フィールドに `,` `"` `\n` `\r` や前後スペースが含まれる場合に RFC 4180 準拠のクォートエスケープを行います。  
通常は直接呼び出す必要はありません。

---

### 計測ログ API

複数列の計測ログを、列番号・ヘッダ名・値の取得元を明示して出力できます。
`SaveCSVValue` の `id,value` 形式ではなく、時刻と複数センサ値を横並びで記録する用途に使います。

```cpp
std::vector<MeasurementLogColumn> columns = {
    MeasurementLogElapsedTimeColumn(0, "Time"),
    MeasurementLogValueColumn(1, "Fx", 0),
    MeasurementLogValueColumn(2, "Fy", 1),
    MeasurementLogValueColumn(3, "Fz", 2),
    MeasurementLogValueColumn(4, "Mx", 3),
    MeasurementLogValueColumn(5, "My", 4),
    MeasurementLogValueColumn(6, "Mz", 5),
};

std::ofstream log("result.csv");
WriteMeasurementLogHeader(log, columns);

std::vector<double> values = {1.0, 2.0, 3.0, 0.1, 0.2, 0.3};
WriteMeasurementLogRow(log, columns, 1234.0, values);
```

生成される CSV:

```csv
Time,Fx,Fy,Fz,Mx,My,Mz
1234,1,2,3,0.1,0.2,0.3
```

#### `MeasurementLogColumn`

```cpp
struct MeasurementLogColumn {
    size_t column_index;
    std::string header;
    MeasurementLogValueSource source;
    size_t value_index;
    std::string literal;
};
```

| フィールド | 説明 |
|---|---|
| `column_index` | CSV の何列目に出力するか。0 始まり |
| `header` | ヘッダ行に出力する列名 |
| `source` | 行データの取得元 |
| `value_index` | `MeasurementValue` の場合、値配列の何番目を使うか |
| `literal` | `Literal` の場合、固定文字列として出力する値 |

#### 補助関数

```cpp
MeasurementLogElapsedTimeColumn(column_index, header);
MeasurementLogValueColumn(column_index, header, value_index);
MeasurementLogLiteralColumn(column_index, header, literal);
```

#### 書き込み関数

```cpp
bool WriteMeasurementLogHeader(
    std::ostream& output,
    const std::vector<MeasurementLogColumn>& columns
);

template <typename T>
bool WriteMeasurementLogRow(
    std::ostream& output,
    const std::vector<MeasurementLogColumn>& columns,
    T elapsed_time,
    const std::vector<T>& values
);
```

列番号が重複している場合や、`value_index` が値配列の範囲外の場合は `false` を返します。

---

## Logger との統合

`UtilsCSV.h` を読み込む前に `UTILS_ENABLE_LOGGER` を定義すると、  
エラー・警告メッセージが `Logger::Error` / `Logger::Warning` 経由で出力されます。

```cpp
#define UTILS_ENABLE_LOGGER
#include "UtilsLogger.h"
#include "UtilsCSV.h"
```

定義しない場合は `stderr` に直接出力されます。

---

## サンプルプログラム

### 基本：保存して読み込む

```cpp
#include "UtilsCSV.h"
#include <vector>
#include <cstdio>

int main() {
    // int の配列を保存する
    std::vector<int> data = {10, 20, 30, 40, 50};
    if (SaveCSVValue("data.csv", data, "score")) {
        printf("保存成功\n");
    }

    // 読み込む
    std::vector<int> loaded;
    if (LoadCSVValue("data.csv", loaded, data.size())) {
        for (size_t i = 0; i < loaded.size(); ++i) {
            printf("loaded[%zu] = %d\n", i, loaded[i]);
        }
    }
    return 0;
}
```

生成される `data.csv`:
```
id,score
0,10
1,20
2,30
3,40
4,50
```

---

### ループ：複数ファイルを順番に保存する

ループ内でファイル名を変えながら複数の CSV を出力する例です。

```cpp
#include "UtilsCSV.h"
#include <vector>
#include <string>
#include <cstdio>

int main() {
    const int num_files = 3;

    for (int f = 0; f < num_files; ++f) {
        std::string filename = "output_" + std::to_string(f) + ".csv";

        // ファイルごとに異なる値を生成する
        std::vector<double> values(5);
        for (size_t i = 0; i < values.size(); ++i) {
            values[i] = static_cast<double>(f * 10 + i) * 0.1;
        }

        if (SaveCSVValue(filename, values, "value")) {
            printf("%s を保存しました\n", filename.c_str());
        } else {
            printf("%s の保存に失敗しました\n", filename.c_str());
        }
    }
    return 0;
}
```

---

### ループ：全ファイルを読み込んで集計する

```cpp
#include "UtilsCSV.h"
#include <vector>
#include <string>
#include <cstdio>
#include <numeric>

int main() {
    const int num_files = 3;
    const size_t expected = 5;

    double grand_total = 0.0;

    for (int f = 0; f < num_files; ++f) {
        std::string filename = "output_" + std::to_string(f) + ".csv";

        std::vector<double> values;
        if (!LoadCSVValue(filename, values, expected)) {
            printf("%s の読み込みに失敗しました\n", filename.c_str());
            continue;
        }

        double total = std::accumulate(values.begin(), values.end(), 0.0);
        printf("%s: 合計=%.2f\n", filename.c_str(), total);
        grand_total += total;
    }

    printf("全ファイル合計: %.2f\n", grand_total);
    return 0;
}
```

---

### ループ：float 配列を保存・読み込みする（型指定）

テンプレート引数で任意の数値型を使えます。

```cpp
#include "UtilsCSV.h"
#include <vector>
#include <cstdio>

int main() {
    // float 型の配列を保存する
    std::vector<float> sensors(8);
    for (size_t i = 0; i < sensors.size(); ++i) {
        sensors[i] = static_cast<float>(i) * 1.5f;
    }
    SaveCSVValue("sensors.csv", sensors, "sensor_value");

    // float 型として読み込む
    std::vector<float> loaded;
    if (LoadCSVValue("sensors.csv", loaded, sensors.size())) {
        for (size_t i = 0; i < loaded.size(); ++i) {
            printf("sensor[%zu] = %.2f\n", i, loaded[i]);
        }
    }
    return 0;
}
```

---

### ループ：ヘッダなし CSV を読み込む

外部ツールが生成したヘッダなし CSV を読み込む場合は `header_exists = false` を指定します。

```cpp
// ヘッダなし CSV の内容例 (raw.csv):
// 0,100
// 1,200
// 2,300

#include "UtilsCSV.h"
#include <vector>
#include <cstdio>

int main() {
    std::vector<int> values;
    if (LoadCSVValue("raw.csv", values, 3, /*header_exists=*/false)) {
        for (size_t i = 0; i < values.size(); ++i) {
            printf("values[%zu] = %d\n", i, values[i]);
        }
    }
    return 0;
}
```

---

## 注意事項

- `expected_size` で指定した要素数のインデックスがすべて揃っていない場合、`LoadCSVValue` は `false` を返し `values` は変更されません。
- `SaveCSVValue` はアトミックに書き込みます（`.tmp` → rename）。書き込み中の停電などでも元のファイルは保護されます。
- 既存の `.bak` ファイルは上書きされます。
- ファイルパスのディレクトリが存在しない場合、`SaveCSVValue` は失敗します。あらかじめディレクトリを作成してください。
