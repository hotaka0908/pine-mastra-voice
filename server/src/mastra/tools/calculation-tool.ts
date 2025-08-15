import { createTool } from '@mastra/core/tools';
import { z } from 'zod';

export const calculationTool = createTool({
  id: 'calculator',
  description: 'Perform mathematical calculations',
  inputSchema: z.object({
    expression: z.string().describe('Mathematical expression to evaluate (e.g., "2 + 3 * 4")'),
  }),
  outputSchema: z.object({
    result: z.number(),
    expression: z.string(),
  }),
  execute: async ({ context }) => {
    try {
      // Simple expression evaluator - in production, use a proper math parser
      const sanitizedExpression = context.expression.replace(/[^0-9+\-*/.() ]/g, '');
      const result = Function(`"use strict"; return (${sanitizedExpression})`)();
      
      if (typeof result !== 'number' || isNaN(result)) {
        throw new Error('計算結果が数値ではありません');
      }

      return {
        result,
        expression: context.expression,
      };
    } catch (error) {
      throw new Error(`計算エラー: ${error instanceof Error ? error.message : '不明なエラー'}`);
    }
  },
});