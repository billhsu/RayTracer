#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")
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



char *readFileData(char *szFilename, long &dataLengthOut)
{
    FILE *fp = fopen(szFilename, "rb");
    long len;
    char *buffer;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = (char*) calloc(1, len+1);
    fread(buffer, 1, len, fp);
    fclose(fp);
    dataLengthOut = len;
    return buffer;
}

void parseWav(char *data)
{
    long *mPtr;
    long *tmpPtr;

    char *buffer;

    WAVEFORMATEX wf;
    volatile WAVEHDR wh;
    HWAVEOUT hWaveOut;

    fmtChunk mFmtChunk;
    wavChunk mDataChunk;

    mPtr = (long*)data;

    if ( mPtr[0] == 0x46464952) //  little endian check for 'RIFF'
    {
        mPtr += 3;
        if (mPtr[0] == 0x20746D66)  // little endian for "fmt "
        {
            printf("Format chunk found\n");

            tmpPtr = mPtr;
            memcpy(&mFmtChunk, tmpPtr, sizeof(mFmtChunk));
            tmpPtr += 2;
            tmpPtr += mFmtChunk.chunkDataSize/4;

            mPtr = tmpPtr;
            if (mPtr[0] == 0x61746164)        // little endian for "data"
            {
                printf("Data chunk found\n");

                tmpPtr = mPtr;
                memcpy(&mDataChunk, tmpPtr, sizeof(mDataChunk));
                mPtr += 2;

                buffer = (char*) malloc(mDataChunk.chunkDataSize);
                memcpy(buffer, mPtr, mDataChunk.chunkDataSize);

                printf("sampleRate: %d\n", mFmtChunk.sampleRate);

                wf.wFormatTag = mFmtChunk.compressionCode;
                wf.nChannels = mFmtChunk.numChannels;
                wf.nSamplesPerSec = mFmtChunk.sampleRate;
                wf.nAvgBytesPerSec = mFmtChunk.avgBytesPerSec;
                wf.nBlockAlign = mFmtChunk.blockAlign;
                wf.wBitsPerSample = mFmtChunk.significantBitsPerSample;
                wf.cbSize = mFmtChunk.extraFormatBytes;

                wh.lpData = buffer;
                wh.dwBufferLength = mDataChunk.chunkDataSize;
                wh.dwFlags = 0;
                wh.dwLoops = 0;

                waveOutOpen(&hWaveOut,WAVE_MAPPER,&wf,0,0,CALLBACK_NULL);
                waveOutPrepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
                waveOutWrite(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));

                do {}
                while (!(wh.dwFlags & WHDR_DONE));

                waveOutUnprepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
                waveOutClose(hWaveOut);

                free(buffer);
            }
            else printf("INvalid WAV\n");
        }
        

    }

    else
        printf("INvalid WAV\n");
}


int main()
{
    char *filename = "tada.wav";
    char *buffer;
    long fileSize;

    buffer = readFileData(filename, fileSize);
    parseWav(buffer);

    free(buffer);
    system("pause");
    return 0;
}