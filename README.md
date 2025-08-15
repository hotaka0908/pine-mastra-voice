# Pine - Mastra Voice Assistant

天気サービスに特化したMastraベースのAIアプリケーション with OpenAI Whisper音声認識統合

## 🎤 主な機能

- **音声認識**: OpenAI Whisperを使用した高精度なSTT（Speech-to-Text）
- **マルチエージェント**: 汎用エージェントと天気エージェント
- **リアルタイム処理**: 音声ファイル → 文字起こし → AIエージェント応答
- **多言語対応**: 日本語・英語音声認識

## 🚀 API エンドポイント

### 音声認識 + エージェント統合
```bash
POST /voice-to-agent
```
- 音声ファイルをアップロードして、指定エージェントからの応答を取得
- パラメータ: `audio` (音声ファイル), `agentName` (エージェント名)

### シンプル音声認識
```bash
POST /speech-to-text
```
- 音声ファイルをテキストに変換（認識結果のみ）
- パラメータ: `audio` (音声ファイル)

## 🤖 利用可能なエージェント

### 1. 汎用エージェント (`generalAgent`)
- **計算機能**: 数式の計算
- **日時処理**: 現在時刻、日付計算、比較
- **テキスト処理**: 文字数カウント、フォーマット、要約
- **検索機能**: ウェブ検索（モック実装）

### 2. 天気エージェント (`weatherAgent`)
- **天気情報**: OpenMeteo APIから現在の天気データを取得
- **位置検索**: ジオコーディング対応
- **アクティビティ提案**: 天気に基づいた活動提案

## 🛠️ 技術スタック

- **AIフレームワーク**: Mastra Core v0.13.2
- **音声認識**: OpenAI Whisper API
- **AIモデル**: Google Gemini 2.5 Pro
- **データベース**: LibSQL
- **ランタイム**: Node.js 20.9.0+
- **言語**: TypeScript (ES2022)

## ⚙️ セットアップ

### 1. 依存関係のインストール
```bash
cd server
npm install
```

### 2. 環境変数の設定
```bash
# server/.env
GOOGLE_GENERATIVE_AI_API_KEY=your_google_api_key
OPENAI_API_KEY=your_openai_api_key
```

### 3. 開発サーバーの起動
```bash
npm run dev
```

サーバーが `http://localhost:4114` で起動します。

## 📝 使用例

### cURLでテスト
```bash
# 音声認識のみ
curl -X POST http://localhost:4114/speech-to-text \
  -F "audio=@your_audio_file.wav"

# 音声 → エージェント応答
curl -X POST http://localhost:4114/voice-to-agent \
  -F "audio=@your_audio_file.wav" \
  -F "agentName=generalAgent"
```

### JavaScript/フロントエンド
```javascript
const formData = new FormData();
formData.append('audio', audioFile);
formData.append('agentName', 'generalAgent');

const response = await fetch('http://localhost:4114/voice-to-agent', {
  method: 'POST',
  body: formData
});

const result = await response.json();
console.log('音声認識:', result.transcript);
console.log('エージェント回答:', result.agentResponse);
```

## 🎯 対応音声形式

WAV, MP3, M4A, OGG, FLAC, WEBM, MPEG, MPGA

## 📁 プロジェクト構造

```
pine/
├── server/                 # Mastraバックエンド
│   ├── src/
│   │   └── mastra/
│   │       ├── agents/     # AIエージェント
│   │       ├── tools/      # ツール（計算、日時、テキスト処理等）
│   │       ├── workflows/  # ワークフロー
│   │       └── index.ts    # メイン設定 + カスタムAPIルート
│   ├── package.json
│   └── .env
├── device/                 # 将来のデバイス固有コード用
├── CLAUDE.md              # AI開発ガイド
└── README.md
```

## 🔧 開発情報

詳細な開発ガイドは [CLAUDE.md](./CLAUDE.md) を参照してください。

## 📄 ライセンス

MIT License