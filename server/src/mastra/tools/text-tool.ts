import { createTool } from '@mastra/core/tools';
import { z } from 'zod';

export const textTool = createTool({
  id: 'text-processor',
  description: 'Process and manipulate text in various ways',
  inputSchema: z.object({
    operation: z.enum(['count', 'translate', 'summarize', 'format', 'extract']).describe('Text operation to perform'),
    text: z.string().describe('Input text to process'),
    options: z.object({
      language: z.string().optional().describe('Target language for translation'),
      format: z.enum(['upper', 'lower', 'title', 'capitalize']).optional().describe('Text formatting option'),
      maxLength: z.number().optional().describe('Maximum length for summary'),
      pattern: z.string().optional().describe('Regex pattern for extraction'),
    }).optional(),
  }),
  outputSchema: z.object({
    result: z.string(),
    operation: z.string(),
    metadata: z.object({
      originalLength: z.number().optional(),
      processedLength: z.number().optional(),
      wordCount: z.number().optional(),
    }).optional(),
  }),
  execute: async ({ context }) => {
    const { operation, text, options = {} } = context;
    
    switch (operation) {
      case 'count':
        const wordCount = text.trim().split(/\s+/).length;
        const charCount = text.length;
        return {
          result: `文字数: ${charCount}, 単語数: ${wordCount}`,
          operation: 'count',
          metadata: {
            originalLength: charCount,
            wordCount,
          },
        };
      
      case 'translate':
        // Simulated translation - in production, use a translation API
        return {
          result: `[翻訳シミュレーション] ${text} → ${options.language || 'en'}`,
          operation: 'translate',
          metadata: {
            originalLength: text.length,
          },
        };
      
      case 'summarize':
        const maxLength = options.maxLength || 100;
        const summary = text.length > maxLength 
          ? text.substring(0, maxLength) + '...' 
          : text;
        return {
          result: summary,
          operation: 'summarize',
          metadata: {
            originalLength: text.length,
            processedLength: summary.length,
          },
        };
      
      case 'format':
        let formatted: string;
        switch (options.format) {
          case 'upper':
            formatted = text.toUpperCase();
            break;
          case 'lower':
            formatted = text.toLowerCase();
            break;
          case 'title':
            formatted = text.replace(/\w\S*/g, (txt) => 
              txt.charAt(0).toUpperCase() + txt.substr(1).toLowerCase());
            break;
          case 'capitalize':
            formatted = text.charAt(0).toUpperCase() + text.slice(1).toLowerCase();
            break;
          default:
            formatted = text;
        }
        return {
          result: formatted,
          operation: 'format',
          metadata: {
            originalLength: text.length,
            processedLength: formatted.length,
          },
        };
      
      case 'extract':
        if (!options.pattern) {
          throw new Error('抽出にはパターンが必要です');
        }
        try {
          const regex = new RegExp(options.pattern, 'g');
          const matches = Array.from(text.matchAll(regex), m => m[0]);
          return {
            result: matches.length > 0 ? matches.join(', ') : '一致するものが見つかりませんでした',
            operation: 'extract',
            metadata: {
              originalLength: text.length,
            },
          };
        } catch (error) {
          throw new Error(`正規表現エラー: ${error instanceof Error ? error.message : '不明なエラー'}`);
        }
      
      default:
        throw new Error('サポートされていない操作です');
    }
  },
});