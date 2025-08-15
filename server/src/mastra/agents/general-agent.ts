import { google } from '@ai-sdk/google';
import { Agent } from '@mastra/core/agent';
import { Memory } from '@mastra/memory';
import { LibSQLStore } from '@mastra/libsql';
import { searchTool } from '../tools/search-tool';
import { calculationTool } from '../tools/calculation-tool';
import { dateTimeTool } from '../tools/datetime-tool';
import { textTool } from '../tools/text-tool';

export const generalAgent = new Agent({
  name: 'General Assistant Agent',
  instructions: `
      あなたは様々なタスクをサポートする汎用AIアシスタントです。以下の機能を提供できます：

      ## 主な機能
      1. **情報検索**: web-searchツールを使用してインターネット上の情報を検索
      2. **数学計算**: calculatorツールで数式の計算を実行
      3. **日時処理**: datetime-toolで現在時刻取得、日付計算、比較などを実行
      4. **テキスト処理**: text-processorで文字数カウント、フォーマット、要約、抽出などを実行

      ## 対応する質問例
      - 「○○について調べて」→ 検索ツールを使用
      - 「2 + 3 * 4を計算して」→ 計算ツールを使用
      - 「今日の日付は？」→ 日時ツールを使用
      - 「このテキストの文字数は？」→ テキストツールを使用

      ## 応答ガイドライン
      - ユーザーの質問を理解し、適切なツールを選択して実行
      - 結果を分かりやすく日本語で説明
      - 必要に応じて複数のツールを組み合わせて使用
      - エラーが発生した場合は、理由を明確に説明し代替案を提示
      - ユーザーが具体的な指示をしない場合は、利用可能な機能を説明

      常に丁寧で親切な対応を心がけ、ユーザーのニーズに最適なサポートを提供してください。
  `,
  model: google('gemini-2.5-pro'),
  tools: { 
    searchTool, 
    calculationTool, 
    dateTimeTool, 
    textTool 
  },
  memory: new Memory({
    storage: new LibSQLStore({
      url: 'file:../mastra.db', // path is relative to the .mastra/output directory
    }),
  }),
});