#ifndef WAVFILE_H
#define WAVFILE_H
#include <windows.h>
#include <mmsystem.h>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

class wav{
    public:

        wav(void);
        short *readWavFileData(char *szFilename, long &dataLengthOut);
        void openDevice();
        void prepWave();
        void playWave(short* buffer,int length);
        void unprepWave();
        void closeDevice();
        void setMutex(boost::mutex* _mutex);
    private:
        struct wavFileHeader
        {
            long chunkId;           //"RIFF" (0x52,0x49,0x46,0x46)
            long chunkSize;         // (fileSize - 8)  - could also be thought of as bytes of data in file following this field (bytesRemaining)
            long riffType;          // "WAVE" (0x57415645)
        };

        struct fmtChunk
        {
            long chunkId;                       // "fmt " - (0x666D7420)
            long chunkDataSize;                 // 16 + extra format bytes
            short compressionCode;              // 1 - 65535
            short numChannels;                  // 1 - 65535
            long sampleRate;                    // 1 - 0xFFFFFFFF
            long avgBytesPerSec;                // 1 - 0xFFFFFFFF
            short blockAlign;                   // 1 - 65535
            short significantBitsPerSample;     // 2 - 65535
            short extraFormatBytes;             // 0 - 65535
        };

        struct wavChunk
        {
            long chunkId;
            long chunkDataSize;
        };

        short* parseWav(char *data);
    public:
        WAVEFORMATEX wf;
        volatile WAVEHDR wh;
        HWAVEOUT hWaveOut;
        boost::mutex* mutex;
    protected:
        static DWORD WINAPI AudioOutThreadProc(LPVOID lpParameter);
        HANDLE m_hAudioOut;
        DWORD m_dwAudioOutId;
};
#endif
