import { createTool } from '@mastra/core/tools';
import { z } from 'zod';

export const searchTool = createTool({
  id: 'web-search',
  description: 'Search the web for information on any topic',
  inputSchema: z.object({
    query: z.string().describe('Search query'),
    maxResults: z.number().optional().default(5).describe('Maximum number of results to return'),
  }),
  outputSchema: z.object({
    results: z.array(z.object({
      title: z.string(),
      snippet: z.string(),
      url: z.string(),
    })),
    query: z.string(),
  }),
  execute: async ({ context }) => {
    // Simulated web search - in a real implementation, you would use a search API
    const mockResults = [
      {
        title: `検索結果: ${context.query} について`,
        snippet: `${context.query} に関する情報が見つかりました。詳細については実際の検索サービスをご利用ください。`,
        url: `https://example.com/search?q=${encodeURIComponent(context.query)}`,
      },
    ];

    return {
      results: mockResults.slice(0, context.maxResults || 5),
      query: context.query,
    };
  },
});