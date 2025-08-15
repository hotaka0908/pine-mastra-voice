import { createTool } from '@mastra/core/tools';
import { z } from 'zod';

export const dateTimeTool = createTool({
  id: 'datetime-tool',
  description: 'Get current date/time information or perform date calculations',
  inputSchema: z.object({
    operation: z.enum(['current', 'format', 'add', 'subtract', 'compare']).describe('Operation to perform'),
    date: z.string().optional().describe('ISO date string (for format, add, subtract, compare operations)'),
    amount: z.number().optional().describe('Amount to add/subtract'),
    unit: z.enum(['days', 'hours', 'minutes', 'months', 'years']).optional().describe('Unit for add/subtract operations'),
    format: z.string().optional().describe('Format string for date formatting'),
    compareDate: z.string().optional().describe('Date to compare with (for compare operation)'),
    timezone: z.string().optional().default('Asia/Tokyo').describe('Timezone (default: Asia/Tokyo)'),
  }),
  outputSchema: z.object({
    result: z.string(),
    operation: z.string(),
  }),
  execute: async ({ context }) => {
    const now = new Date();
    
    switch (context.operation) {
      case 'current':
        return {
          result: now.toLocaleString('ja-JP', { timeZone: context.timezone }),
          operation: 'current',
        };
      
      case 'format':
        if (!context.date) throw new Error('日付が指定されていません');
        const date = new Date(context.date);
        return {
          result: date.toLocaleString('ja-JP', { timeZone: context.timezone }),
          operation: 'format',
        };
      
      case 'add':
      case 'subtract':
        if (!context.date || !context.amount || !context.unit) {
          throw new Error('日付、数量、単位が必要です');
        }
        const baseDate = new Date(context.date);
        const multiplier = context.operation === 'add' ? 1 : -1;
        
        switch (context.unit) {
          case 'days':
            baseDate.setDate(baseDate.getDate() + (context.amount * multiplier));
            break;
          case 'hours':
            baseDate.setHours(baseDate.getHours() + (context.amount * multiplier));
            break;
          case 'minutes':
            baseDate.setMinutes(baseDate.getMinutes() + (context.amount * multiplier));
            break;
          case 'months':
            baseDate.setMonth(baseDate.getMonth() + (context.amount * multiplier));
            break;
          case 'years':
            baseDate.setFullYear(baseDate.getFullYear() + (context.amount * multiplier));
            break;
        }
        
        return {
          result: baseDate.toLocaleString('ja-JP', { timeZone: context.timezone }),
          operation: context.operation,
        };
      
      case 'compare':
        if (!context.date || !context.compareDate) {
          throw new Error('比較する2つの日付が必要です');
        }
        const date1 = new Date(context.date);
        const date2 = new Date(context.compareDate);
        const diffMs = date1.getTime() - date2.getTime();
        const diffDays = Math.floor(diffMs / (1000 * 60 * 60 * 24));
        
        return {
          result: `${Math.abs(diffDays)}日の差があります（${diffDays > 0 ? '前者が後' : '前者が先'}）`,
          operation: 'compare',
        };
      
      default:
        throw new Error('サポートされていない操作です');
    }
  },
});