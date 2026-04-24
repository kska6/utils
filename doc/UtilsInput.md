# UtilsInput.h — キー入力ユーティリティ

## 概要

`include/UtilsInput.h` はヘッダオンリーのキーボード入力ユーティリティです。  
Windows と Unix 系 OS（Linux / macOS）の両方で動作します。

- **`KbHit()`** — キーが押されているかをノンブロッキングで確認
- **`GetCh()`** — 1 文字をブロッキングで読み取る（エコーなし）

すべての関数は `input_ns` 名前空間に配置されています。

**必要な C++ バージョン**: C++11 以上

---

## 名前空間: `input_ns`

### 関数リファレンス

| 関数 | 戻り値 | 説明 |
|---|---|---|
| `KbHit()` | `bool` | キーが押されていれば `true`、押されていなければ `false` を返す（ノンブロッキング） |
| `GetCh()` | `int` | 1 文字を読み取って文字コードを返す（ブロッキング）。エコーなし・行バッファなし |

---

## プラットフォーム別の動作

| OS | `KbHit()` の実装 | `GetCh()` の実装 |
|---|---|---|
| Windows | `_kbhit()` (conio.h) | `_getch()` (conio.h) |
| Unix 系 | `termios` でノンブロッキング読み取り | `termios` で raw モード読み取り |

Unix 系では `termios` の設定を一時的に変更し、読み取り後に元の設定へ戻します。

---

## サンプルプログラム

### 基本：キーが押されたら文字を表示する

```cpp
#include "UtilsInput.h"
#include <cstdio>

int main() {
    printf("何かキーを押してください（q で終了）\n");

    while (true) {
        if (input_ns::KbHit()) {
            int ch = input_ns::GetCh();
            printf("入力: %c (コード: %d)\n", ch, ch);
            if (ch == 'q' || ch == 'Q') {
                break;
            }
        }
    }

    printf("終了します\n");
    return 0;
}
```

---

### ループ：キー入力でメニューを操作する

```cpp
#include "UtilsInput.h"
#include <cstdio>

int main() {
    printf("=== メニュー ===\n");
    printf("[1] オプション A\n");
    printf("[2] オプション B\n");
    printf("[3] オプション C\n");
    printf("[q] 終了\n\n");
    printf("キーを押してください: ");

    bool running = true;
    while (running) {
        if (!input_ns::KbHit()) {
            continue;   // キー入力がなければ待機
        }

        int ch = input_ns::GetCh();
        switch (ch) {
        case '1':
            printf("\nオプション A を選択しました\n");
            break;
        case '2':
            printf("\nオプション B を選択しました\n");
            break;
        case '3':
            printf("\nオプション C を選択しました\n");
            break;
        case 'q':
        case 'Q':
            printf("\n終了します\n");
            running = false;
            break;
        default:
            printf("\n不明なキー: %c\n", ch);
            break;
        }

        if (running) {
            printf("キーを押してください: ");
        }
    }
    return 0;
}
```

---

### ループ：ゲームループ風のキー入力処理

`KbHit()` のノンブロッキング特性を利用して、他の処理をしながらキー入力を受け付けます。

```cpp
#include "UtilsInput.h"
#include "UtilsTime.h"
#include <cstdio>

int main() {
    TimeManager::Initialize();

    int counter = 0;
    bool running = true;

    printf("スペースでカウント、q で終了\n");

    while (running) {
        UpdateFrameTime();

        // キー入力はノンブロッキングで確認
        if (input_ns::KbHit()) {
            int ch = input_ns::GetCh();
            if (ch == ' ') {
                ++counter;
                printf("カウント: %d\n", counter);
            } else if (ch == 'q' || ch == 'Q') {
                running = false;
            }
        }

        // その他のゲームロジックをここで実行
        SleepMilliseconds(16);   // ~60fps
    }

    printf("最終カウント: %d\n", counter);
    return 0;
}
```

---

### ループ：Enter キーを待ってから次のステップへ進む

```cpp
#include "UtilsInput.h"
#include <cstdio>

static void wait_enter() {
    printf("Enter キーを押して続行...");
    // Enter (13: CR または 10: LF) が来るまで待つ
    while (true) {
        int ch = input_ns::GetCh();
        if (ch == '\r' || ch == '\n') break;
    }
    printf("\n");
}

int main() {
    const int steps = 3;
    for (int i = 1; i <= steps; ++i) {
        printf("--- ステップ %d ---\n", i);
        printf("処理中...\n");
        wait_enter();
    }
    printf("全ステップ完了\n");
    return 0;
}
```

---

## 注意事項

- Unix 系の `GetCh()` は内部で `termios` 設定を変更します。  
  シグナルで強制終了した場合、ターミナルの設定が元に戻らない可能性があります。
- `KbHit()` は**キー入力の有無を確認するだけ**で、文字を消費しません。  
  `KbHit()` が `true` を返した後は必ず `GetCh()` で文字を読み取ってください。  
  読み取らずに再度 `KbHit()` を呼ぶと `true` が返り続けます。
- `GetCh()` はブロッキング関数です。キー入力があるまで処理が止まります。  
  ノンブロッキングが必要な場合は `KbHit()` で確認してから呼んでください。
