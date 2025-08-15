# CLAUDE.md

このファイルは、このリポジトリでコードを操作する際のClaude Code (claude.ai/code) へのガイダンスを提供します。

## プロジェクト構成

これは天気サービスに特化したMastraベースのAIアプリケーションです。プロジェクトには2つの主要なディレクトリがあります：

- `/server/` - Mastraフレームワークを使用したNode.js/TypeScriptバックエンド
- `/device/` - 現在は空、将来のデバイス固有のコード用

## コマンド

**開発サーバー:**
```bash
cd server && npm run dev
```

**ビルド:**
```bash
cd server && npm run build
```

**本番環境での起動:**
```bash
cd server && npm run start
```

**テスト:**
現在テストは設定されていません。テストスクリプトはエラーを返します。

## アーキテクチャ概要

### Mastraフレームワーク
このプロジェクトでは、以下を提供するAIワークフローフレームワークであるMastraを使用しています：
- **Agents**: メモリとツールアクセスを持つAIエージェント
- **Tools**: エージェントが呼び出せる再利用可能な関数
- **Workflows**: エージェントとツールを組み合わせたマルチステッププロセス
- **Storage**: 永続化とメモリのためのLibSQL

### コアコンポーネント

**メイン設定** (`server/src/mastra/index.ts`):
- ワークフロー、エージェント、ストレージ、ログでMastraを初期化
- テレメトリストレージにインメモリLibSQLを使用
- Pinoロガーを設定

**Weather Agent** (`server/src/mastra/agents/weather-agent.ts`):
- Google Gemini 2.5 Proモデルを使用
- 天気情報とアクティビティプランニングを提供
- LibSQLファイルストレージを使用した永続メモリ
- 位置ベースの天気クエリに最適化された指示

**Weather Tool** (`server/src/mastra/tools/weather-tool.ts`):
- Open-Meteo APIから現在の天気データを取得
- 位置検索のためのジオコーディングを処理
- 構造化された天気データ（気温、湿度、風速など）を返す
- 40以上の天気コードのマッピングを含む

**Weather Workflow** (`server/src/mastra/workflows/weather-workflow.ts`):
- 2ステッププロセス：天気取得 → アクティビティプランニング
- 最初のステップで天気予報データを取得
- 2番目のステップでweather agentを使用してフォーマットされたアクティビティを提案
- 特定のフォーマット要件を持つ構造化されたアクティビティ推奨を返す

### 技術スタック
- **ランタイム**: Node.js 20.9.0+、ESモジュール
- **言語**: strictモードのTypeScript
- **AIフレームワーク**: Mastra Core v0.13.2
- **AIプロバイダー**: Google AI SDK with Gemini 2.5 Pro
- **データベース**: エージェントメモリとテレメトリのためのLibSQL
- **バリデーション**: 全体を通してZodスキーマ
- **APIs**: 天気データとジオコーディングのためのOpen-Meteo

### 主要パターン
- 全コンポーネントでZodスキーマによる入出力バリデーションを使用
- ツールとワークフローはモジュラーで構成可能
- エージェントメモリは会話を通じて永続化
- ワークフローはリアルタイム出力のためにレスポンスをストリーミング可能
- エラーハンドリングには位置バリデーションとAPI障害ケースを含む

## 開発メモ

- プロジェクトはESモジュール（`"type": "module"`）を使用
- TypeScript設定はbundlerモジュール解決でES2022をターゲット
- 現在リンティングやフォーマッティングツールは設定されていない
- `device/`ディレクトリは存在するが空で、将来の拡張を示唆