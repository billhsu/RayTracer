#include "wav.h"
#include <stdio.h>
#pragma comment(lib, "winmm.lib")

wav::wav(void)
{

}

short* wav::readWavFileData(char *szFilename, long &dataLengthOut)
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
    short* bufferParsed = parseWav(buffer);
    free(buffer);
    return bufferParsed;

}


short* wav::parseWav(char *data)
{
    long *mPtr;
    long *tmpPtr;

    short *buffer;

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

                buffer = (short*) malloc(mDataChunk.chunkDataSize);
                memcpy(buffer, mPtr, mDataChunk.chunkDataSize);

                printf("sampleRate: %d\nnChannels: %d\nchunkDataSize: %d\n", 
                    mFmtChunk.sampleRate,
                    mFmtChunk.numChannels,
                    mDataChunk.chunkDataSize);
                
                wf.wFormatTag = mFmtChunk.compressionCode;
                wf.nChannels = mFmtChunk.numChannels;
                wf.nSamplesPerSec = mFmtChunk.sampleRate;
                wf.nAvgBytesPerSec = mFmtChunk.avgBytesPerSec;
                wf.nBlockAlign = mFmtChunk.blockAlign;
                wf.wBitsPerSample = mFmtChunk.significantBitsPerSample;
                wf.cbSize = mFmtChunk.extraFormatBytes;

                return buffer;
            }
            else printf("INvalid WAV\n");
        }
        

    }

    else
        printf("INvalid WAV\n");
}

void wav::playWave(short* buffer,int length)
{
    wh.lpData = (char*)buffer;
    wh.dwBufferLength = length;
    wh.dwFlags = 0;
    wh.dwLoops = 0;
    printf("start\n");
    waveOutOpen(&hWaveOut,WAVE_MAPPER,&wf,0,0,CALLBACK_NULL);
    waveOutPrepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
    waveOutWrite(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));

    waveOutUnprepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
    waveOutClose(hWaveOut); 
}
