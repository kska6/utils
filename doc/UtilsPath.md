# UtilsPath.h

パス解決とディレクトリ作成を扱うヘッダです。

## 主な関数

### `GetExecutablePath()`

実行ファイルのフルパスを返します。取得できない場合はカレントディレクトリを返します。

### `GetExecutableDirectory()`

実行ファイルが置かれているディレクトリを返します。

### `PathExists(path)`

指定パスが存在するかを返します。

### `FindAncestorDirectoryContaining(start_path, marker_names)`

`start_path` から親ディレクトリを辿り、`marker_names` のいずれかを含む最初のディレクトリを返します。

```cpp
const auto projectRoot = FindAncestorDirectoryContaining(
    GetExecutableDirectory(),
    {".git", "ROBOT_TILE_SLIP.sln"});
```

### `EnsureDirectory(directory_path)`

指定ディレクトリを作成し、そのパスを返します。既に存在する場合もそのまま返します。

```cpp
const auto dataDir = EnsureDirectory(projectRoot / "data");
```
