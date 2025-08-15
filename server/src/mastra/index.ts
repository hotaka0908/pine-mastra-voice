
import { Mastra } from '@mastra/core/mastra';
import { PinoLogger } from '@mastra/loggers';
import { LibSQLStore } from '@mastra/libsql';
import { registerApiRoute } from '@mastra/core/server';
import { OpenAIVoice } from '@mastra/voice-openai';
import { weatherWorkflow } from './workflows/weather-workflow';
import { weatherAgent } from './agents/weather-agent';
import { generalAgent } from './agents/general-agent';
import { Readable } from 'stream';

// Initialize OpenAI Voice for Whisper STT (if API key is available)
let voice: OpenAIVoice | null = null;

if (process.env.OPENAI_API_KEY) {
  voice = new OpenAIVoice({
    listeningModel: {
      name: "whisper-1",
      apiKey: process.env.OPENAI_API_KEY,
    },
  });
}

// Mock STT function for demo purposes when no API key is available
const mockSpeechToText = async (audioBuffer: Buffer, filename: string): Promise<string> => {
  // Simulate processing time
  await new Promise(resolve => setTimeout(resolve, 1000));
  
  const fileSize = audioBuffer.length;
  const extension = filename.split('.').pop()?.toLowerCase() || 'unknown';
  
  return `[モック音声認識] ${extension}ファイル（${fileSize}バイト）を受信しました。実際の音声認識にはOPENAI_API_KEYの設定が必要です。`;
};

export const mastra = new Mastra({
  workflows: { weatherWorkflow },
  agents: { weatherAgent, generalAgent },
  storage: new LibSQLStore({
    // stores telemetry, evals, ... into memory storage, if it needs to persist, change to file:../mastra.db
    url: ":memory:",
  }),
  logger: new PinoLogger({
    name: 'Mastra',
    level: 'info',
  }),
  server: {
    apiRoutes: [
      // Custom endpoint for speech-to-text with agent interaction
      registerApiRoute("/voice-to-agent", {
        method: "POST",
        handler: async (c) => {
          try {
            const mastra = c.get("mastra");
            
            // Get form data from request
            const body = await c.req.formData();
            const audioFile = body.get('audio') as File;
            const agentName = body.get('agentName') as string || 'generalAgent';
            
            if (!audioFile) {
              return c.json({ error: 'Audio file is required' }, 400);
            }

            // Convert File to buffer
            const arrayBuffer = await audioFile.arrayBuffer();
            const buffer = Buffer.from(arrayBuffer);

            // Perform speech-to-text conversion
            let transcript: string;
            if (voice) {
              const audioStream = Readable.from(buffer);
              transcript = await voice.listen(audioStream, {
                filetype: audioFile.name.split('.').pop() as any || "wav",
              });
            } else {
              transcript = await mockSpeechToText(buffer, audioFile.name);
            }

            if (!transcript) {
              return c.json({ error: 'Failed to transcribe audio' }, 500);
            }

            // Get the specified agent
            const agent = mastra.getAgent(agentName);
            if (!agent) {
              return c.json({ error: `Agent '${agentName}' not found` }, 404);
            }

            // Send transcribed text to agent
            const response = await agent.generate([
              {
                role: 'user',
                content: transcript,
              }
            ]);

            return c.json({
              transcript,
              agentResponse: response.text,
              agentName,
            });

          } catch (error) {
            console.error('Voice-to-agent error:', error);
            return c.json({ 
              error: 'Internal server error',
              details: error instanceof Error ? error.message : 'Unknown error'
            }, 500);
          }
        },
      }),

      // Simple speech-to-text endpoint (transcript only)
      registerApiRoute("/speech-to-text", {
        method: "POST",
        handler: async (c) => {
          try {
            // Get form data from request
            const body = await c.req.formData();
            const audioFile = body.get('audio') as File;
            
            if (!audioFile) {
              return c.json({ error: 'Audio file is required' }, 400);
            }

            // Convert File to buffer
            const arrayBuffer = await audioFile.arrayBuffer();
            const buffer = Buffer.from(arrayBuffer);

            // Perform speech-to-text conversion
            let transcript: string;
            if (voice) {
              const audioStream = Readable.from(buffer);
              transcript = await voice.listen(audioStream, {
                filetype: audioFile.name.split('.').pop() as any || "wav",
              });
            } else {
              transcript = await mockSpeechToText(buffer, audioFile.name);
            }

            return c.json({ transcript });

          } catch (error) {
            console.error('Speech-to-text error:', error);
            return c.json({ 
              error: 'Failed to transcribe audio',
              details: error instanceof Error ? error.message : 'Unknown error'
            }, 500);
          }
        },
      }),
    ],
  },
});
