# PDF Editor

シンプルで洗練されたUIを備え、日常的なPDF編集タスクを直感的に実行できるGUIアプリケーションです。

## 🌟 機能

1. **画像PDF変換**: 複数の画像（JPG/PNG等）を読み込み、それぞれのサイズにぴったり合わせたPDFを生成します。
2. **PDF結合**: 選択された複数のPDFファイルをリストの順に結合して、1つのPDFにまとめます。
3. **ページ回転**: 指定したページ（単一、範囲、全体）を時計回りに回転して保存します（例：90度、180度、270度）。
4. **見開き化**: PDFの各ページを2ページずつ左右に配置し、横長の見開き仕様に結合・整形합니다。サイズの異なるページは自動でスケール調整されます。

## 🛠 技術スタック

- **言語**: C++17
- **UIフレームワーク**: Qt6
- **PDF処理**: 
  - PoDoFo (0.9.x) - 変換、結合、回転を利用
  - QPDF - 見開き化などを利用
- **ビルドツール**: CMake

## 🚀 ビルド方法 (Linux/Ubuntu)

### 前提条件
以下の依存パッケージのインストールが必要です。

```bash
sudo apt update
sudo apt install qt6-base-dev libpodofo-dev libqpdf-dev cmake build-essential
```

### ビルド実行

```bash
mkdir build
cd build
cmake ..
make
```

### アプリケーションの起動

```bash
./PDFEditor
```

## 📦 リリース

このリポジトリでは、`v`から始まるタグをPushすると、GitHub Actions経由で自動的にLinux向けの実行バイナリがビルドされ、Releaseページに公開されます。
