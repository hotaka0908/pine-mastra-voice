# M5Stack CoreS3 Voice Recorder

M5Stack CoreS3からMastraサーバーに音声データ（WAV形式）をHTTP POSTで送信するプログラムです。

## 🎯 機能

- **音声録音**: M5Stack CoreS3の内蔵PDMマイクを使用
- **WAV変換**: 16kHz, 16bit, モノラル形式で変換
- **HTTP送信**: multipart/form-data形式でMastraサーバーに送信
- **WiFi接続**: 自動接続とステータス表示
- **ボタン制御**: ボタン押下で録音開始・停止
- **自動停止**: 5秒経過で自動停止

## 🔧 ハードウェア要件

- **M5Stack CoreS3**
- **WiFiネットワーク環境**
- **Mastraサーバーが動作するPC/サーバー**

## 📋 セットアップ手順

### 1. Arduino IDE環境構築

1. **Arduino IDEをインストール** (v1.8.19以降推奨)
2. **ESP32ボードマネージャーを追加**:
   - `ファイル` → `環境設定`
   - `追加のボードマネージャーのURL`に追加:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
3. **ESP32ボードをインストール**:
   - `ツール` → `ボード` → `ボードマネージャー`
   - "ESP32"を検索してインストール

### 2. M5Stack CoreS3ライブラリインストール

1. **ライブラリマネージャーを開く**:
   - `ツール` → `ライブラリを管理`
2. **必要なライブラリをインストール**:
   - `M5CoreS3` (最新版)
   - `WiFi` (ESP32標準)
   - `HTTPClient` (ESP32標準)
   - `SPIFFS` (ESP32標準)

### 3. 設定ファイルの編集

`config.h`ファイルを編集して環境に合わせて設定:

```cpp
// WiFi設定
#define WIFI_SSID "あなたのWiFi名"
#define WIFI_PASSWORD "あなたのWiFiパスワード"

// Mastraサーバー設定
#define SERVER_HOST "192.168.1.100"  // サーバーのIPアドレス
#define SERVER_PORT 4114
#define DEFAULT_AGENT "generalAgent"  // または "weatherAgent"
```

### 4. Mastraサーバーの準備

Mastraサーバーが起動していることを確認:

```bash
cd server
npm run dev
```

サーバーが `http://localhost:4114` で動作していることを確認。

### 5. プログラムのアップロード

1. **M5Stack CoreS3をUSBで接続**
2. **ボード設定**:
   - `ツール` → `ボード` → `ESP32 Arduino` → `M5Stack-CoreS3`
   - `ツール` → `シリアルポート` → 適切なポートを選択
3. **アップロード**:
   - `スケッチ` → `マイコンボードに書き込む`

## 🎮 使用方法

### 基本操作

1. **電源ON**: M5Stack CoreS3の電源をON
2. **WiFi接続待機**: 自動的にWiFiに接続
3. **録音開始**: **ボタンA**を押すと録音開始
4. **録音停止**: 
   - 再度**ボタンA**を押すか
   - 5秒経過で自動停止
5. **サーバー送信**: 自動的にMastraサーバーに送信
6. **結果確認**: 画面に送信結果が表示

### 画面表示

```
Voice Recorder
WiFi: Connected    (緑色 = 接続済み、赤色 = 切断)
Press A to Record  (待機中)
Recording...       (録音中、残り時間も表示)
Sending to server... (送信中)
Success!          (成功時、緑色)
```

### エラー時の対処

| エラー表示 | 原因 | 対処法 |
|-----------|------|--------|
| WiFi Failed! | WiFi接続失敗 | config.hのWiFi設定を確認 |
| No WiFi Connection! | WiFi切断状態で録音 | WiFi再接続を待つ |
| Memory Error! | メモリ不足 | M5Stackを再起動 |
| File Error! | SPIFFS書き込み失敗 | M5Stackを再起動 |
| Server Error! | サーバー応答エラー | サーバーの動作を確認 |
| Connection Failed! | ネットワーク接続失敗 | ネットワーク設定を確認 |

## 🔧 カスタマイズ

### エージェントの変更

`config.h`で使用するエージェントを変更:

```cpp
#define DEFAULT_AGENT "weatherAgent"  // 天気エージェント
// または
#define DEFAULT_AGENT "generalAgent"  // 汎用エージェント
```

### 録音時間の変更

```cpp
#define MAX_RECORD_TIME_SEC 10  // 最大10秒に変更
```

### サーバーURLの変更

```cpp
#define SERVER_HOST "192.168.1.200"  // 別のIPアドレス
#define SERVER_PORT 8080             // 別のポート
```

## 🐛 トラブルシューティング

### 1. 音声が認識されない

- **マイクの向き**: M5Stack CoreS3のマイク位置を確認
- **録音環境**: 静かな環境で明確に話す
- **距離**: デバイスから20cm以内で話す

### 2. WiFi接続失敗

- **SSID/パスワード**: config.hの設定を再確認
- **ネットワーク**: 2.4GHz WiFiネットワークを使用
- **距離**: WiFiルーターの近くでテスト

### 3. サーバー接続失敗

- **IPアドレス**: サーバーのIPアドレスを確認
- **ファイアウォール**: ポート4114が開いているか確認
- **サーバー起動**: Mastraサーバーが動作しているか確認

### 4. メモリエラー

- **電源リセット**: M5Stackを一度電源OFF/ON
- **録音時間短縮**: MAX_RECORD_TIME_SECを短く設定

## 📊 技術仕様

| 項目 | 仕様 |
|------|------|
| サンプリングレート | 16kHz |
| ビット深度 | 16bit |
| チャンネル | モノラル |
| 最大録音時間 | 5秒（設定可能） |
| 音声形式 | WAV |
| 通信プロトコル | HTTP POST |
| データ形式 | multipart/form-data |

## 📝 ライセンス

MIT License

## 🔗 関連リンク

- [M5Stack CoreS3 公式ドキュメント](https://docs.m5stack.com/en/core/CoreS3)
- [Mastra Framework](https://mastra.ai/)
- [Arduino IDE](https://www.arduino.cc/en/software)