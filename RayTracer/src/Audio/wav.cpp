#include "wav.h"
#include "WaveOut.h"
#include <stdio.h>
#pragma comment(lib, "winmm.lib")

void wav::setMutex(boost::mutex* _mutex)
{
    mutex=_mutex;
}


wav::wav(void)
{

}

DWORD WINAPI wav::AudioOutThreadProc(LPVOID lpParameter)
{
    printf("AudioOutThreadProc start.\n");
    MSG msg;
    while(GetMessage(&msg,0,0,0))
    {
        switch(msg.message )
        {
        case WOM_OPEN:
            printf("WOM_OPEN \n");
            break;
        case WOM_CLOSE:
            printf("WOM_CLOSE \n");
            break;
        case WOM_DONE:
            WAVEHDR* pwh=(WAVEHDR*)msg.lParam;
            waveOutUnprepareHeader((HWAVEOUT)msg.wParam,pwh,sizeof(WAVEHDR));
            delete []pwh->lpData;
            delete pwh;
            break;
        }
    }
    printf("AudioInThreadProc exit.\n");
    return msg.wParam;
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
            //printf("Format chunk found\n");

            tmpPtr = mPtr;
            memcpy(&mFmtChunk, tmpPtr, sizeof(mFmtChunk));
            
            tmpPtr += 2;
            tmpPtr += mFmtChunk.chunkDataSize/4;

            mPtr = tmpPtr;
            if (mPtr[0] == 0x61746164)        // little endian for "data"
            {
                //printf("Data chunk found\n");

                tmpPtr = mPtr;
                memcpy(&mDataChunk, tmpPtr, sizeof(mDataChunk));
                mPtr += 2;

                buffer = (short*) malloc(mDataChunk.chunkDataSize);
                memcpy(buffer, mPtr, mDataChunk.chunkDataSize);

                /*printf("sampleRate: %d\nnChannels: %d\nchunkDataSize: %d\n", 
                    mFmtChunk.sampleRate,
                    mFmtChunk.numChannels,
                    mDataChunk.chunkDataSize);
                */
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
void wav::openDevice()
{
    m_hAudioOut=CreateThread(0,0,AudioOutThreadProc,this,0,&m_dwAudioOutId);
    waveOutOpen(&hWaveOut,WAVE_MAPPER,&wf,m_dwAudioOutId,0,CALLBACK_THREAD);
}
void wav::closeDevice()
{
    waveOutClose(hWaveOut);
    PostThreadMessage(m_dwAudioOutId,WM_QUIT,0,0);
}
void wav::prepWave()
{
    wh.dwFlags = 0;
    wh.dwLoops = 1;
    waveOutPrepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
}
void wav::playWave(short* buffer,int length)
{
    char* p = new char[length];
    {
        printf("CopyMem\n");
        boost::lock_guard<boost::mutex> m_csLock(*mutex);
        CopyMemory(p,(char*)buffer,length);
    }
    
    LPWAVEHDR pwh=new WAVEHDR;
    ZeroMemory(pwh,sizeof(WAVEHDR));

    pwh->dwBufferLength=length;
    pwh->lpData=p;
    waveOutPrepareHeader(hWaveOut,pwh,sizeof(WAVEHDR));
    //wh.lpData = (char*)buffer;
    //wh.dwBufferLength = length;
    //printf("[[wav start %d\n",buffer);
    //waveOutWrite(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
    waveOutWrite(hWaveOut,pwh,sizeof(WAVEHDR));
    //do {}
    //while (!(wh.dwFlags & WHDR_DONE));
    //printf("]]wav end\n");
}
void wav::unprepWave()
{
    waveOutUnprepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
}