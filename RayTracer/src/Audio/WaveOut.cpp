#include "WaveOut.h"
#pragma comment(lib, "winmm.lib")

DWORD WaveOut::s_dwInstance = 0;

DWORD WINAPI WaveOut::AudioOutThreadProc(LPVOID lpParameter)
{
    printf("AudioOutThreadProc start.\n");
    WaveOut *pWaveIn;
    pWaveIn = (WaveOut *)lpParameter;

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
            pWaveIn->BufferSub ();
            delete []pwh->lpData;
            delete pwh;
            break;
        }
    }
    printf("AudioInThreadProc exit.\n");
    return msg.wParam;
}

WaveOut::WaveOut():
m_wChannel(1),
    m_dwSample (WOSA_POOR),
    m_wBit(16)
{
    m_hOut = 0;

    m_hAudioOut = 0;
    m_dwAudioOutId = 0;
    m_iBufferNum = 0;

    m_bThreadStart = FALSE;
    m_bDevOpen = FALSE;

    s_dwInstance ++;
}

WaveOut::~WaveOut()
{

}

BOOL WaveOut::StartThread()
{
    if (m_bThreadStart)
    {
        printf("WaveOut::StartThread: Wave out thread has run.\n");
        return FALSE;
    }

    m_hAudioOut=CreateThread(0,0,AudioOutThreadProc,this,0,&m_dwAudioOutId);
    if(!m_hAudioOut)
    {
        printf("WaveOut::StartThread: Strat wave out thread fail.\n");
        return FALSE;
    }
    m_bThreadStart = TRUE;
    return TRUE;
}
BOOL WaveOut::StopThread()
{
    if (!m_bThreadStart)
    {
        printf("WaveOut::StopThread: Wave out thread hasn't run.\n");
        return FALSE;
    }

    if(m_hAudioOut)
    {
        int t=50;
        DWORD ExitCode;
        BOOL bEnd=FALSE;
        PostThreadMessage(m_dwAudioOutId,WM_QUIT,0,0);
        while(t)
        {
            GetExitCodeThread(m_hAudioOut,&ExitCode);
            if(ExitCode!= STILL_ACTIVE)
            {
                bEnd=TRUE;
                break;
            }
            else
                Sleep(10);
            t--;
        }
        if(!bEnd)
        {
            TerminateThread(m_hAudioOut,0);
            printf("WaveOut::StopThread: TerminateThread wave out thread.\n");
        }
        m_hAudioOut=0;
    }
    m_bThreadStart = FALSE;
    return TRUE;
}

BOOL WaveOut::OpenDev()
{
    if (m_bDevOpen)
    {
        printf("WaveOut::OpenDev: Device has open.\n");
        return FALSE;
    }
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = m_wChannel;
    wfx.nSamplesPerSec = m_dwSample;
    wfx.nAvgBytesPerSec = m_wChannel * m_dwSample * m_wBit / 8;
    wfx.nBlockAlign = m_wBit * m_wChannel / 8;
    wfx.wBitsPerSample = m_wBit;
    wfx.cbSize = 0;

    m_mmr=waveOutOpen (0,WAVE_MAPPER,&wfx,0,0,WAVE_FORMAT_QUERY);
    if(m_mmr)
    {
        printf("WaveOut::OpenDev: waveOutOpen error.\n");
        return FALSE;
    }

    m_mmr=waveOutOpen(&m_hOut,WAVE_MAPPER,&wfx,m_dwAudioOutId,s_dwInstance,CALLBACK_THREAD);
    if(m_mmr)
    {
        printf("WaveOut::OpenDev: waveOutOpen error.\n");
        return FALSE;
    }
    m_bDevOpen = TRUE;
    m_iBufferNum = 0;
    return TRUE;
}

BOOL WaveOut::CloseDev()
{
    if (!m_bDevOpen)
    {
        printf("WaveOut::CloseDev: Device hasn't opened.\n");
        return FALSE;
    }

    if(!m_hOut)
    {
        printf("WaveOut::CloseDev: Device hasn't opened.\n");
        return FALSE;
    }

    m_mmr=waveOutClose(m_hOut);
    if(m_mmr)
    {
        printf("WaveOut::CloseDev: waveInClose error.\n");
        return FALSE;
    }
    m_hOut=0;
    m_bDevOpen = FALSE;
    return TRUE;
}

BOOL WaveOut::StartPlay()
{
    BOOL bRet=FALSE;
    if (!StartThread())           
    {
        goto Exit;
    };
    if (!OpenDev())
    {
        goto Exit1;
    };
    bRet = TRUE;
    goto Exit;
Exit1:
    StopThread ();
Exit:
    return bRet;
}

BOOL WaveOut::StopPlay()
{
    CloseDev();
    StopThread ();

    return TRUE;
}

MMRESULT WaveOut::GetLastMMError()
{
    return m_mmr;
}

char* WaveOut::GetLastErrorString()
{
    char buffer[256];
    memset(buffer,0,256);
    waveOutGetErrorText(m_mmr,buffer,256);
    return buffer;
}


void WaveOut::SetChannel(WORD wChannel)
{
    m_wChannel = (m_wChannel == wChannel) ? 2:1;
}

void WaveOut::SetSample(DWORD dwSample)
{
    m_dwSample = dwSample;
}

void WaveOut::SetBit(WORD wBit)
{
    m_wBit = (wBit == 8) ? 8:16;
}

DWORD WaveOut::GetInstance()
{
    return s_dwInstance;
}

WORD WaveOut::GetBit()
{
    return m_wBit;
}

DWORD WaveOut::GetSample()
{
    return m_dwSample;
}

WORD WaveOut::GetChannel()
{
    return m_wChannel;
}

BOOL WaveOut::Play(char* buf,UINT uSize)
{
    if (!m_bDevOpen)
    {
        printf("WaveOut::Play: Device hasn't been open.\n");
        return FALSE;
    }
	if (GetBufferNum () > PLAY_DELAY)
	{
	//	printf("WaveOut::Play: Delay too big pass over.\n");
		return TRUE;
	}
	char* p;
	LPWAVEHDR pwh=new WAVEHDR;
	if(!pwh)
	{
        printf("WaveOut::Play: alloc WAVEHDR memoyr error.\n");
        return FALSE;
    }

    p=new char[uSize];
    if(!p)
    {
        printf("WaveOut::Play: alloc data memoyr error.\n");
        return FALSE;
    }

    CopyMemory(p,buf,uSize);
    ZeroMemory(pwh,sizeof(WAVEHDR));
    pwh->dwBufferLength=uSize;
    pwh->lpData=p;
    m_mmr=waveOutPrepareHeader(m_hOut,pwh,sizeof(WAVEHDR));
    if (m_mmr)
    {
        printf("WaveOut::CloseDev: waveOutPrepareHeader error.\n");
        return FALSE;
    }

    m_mmr=waveOutWrite(m_hOut,pwh,sizeof(WAVEHDR));
    if (m_mmr)
    {
        printf("WaveOut::CloseDev: waveOutWrite error.\n");
        return FALSE;
    }
    m_iBufferNum ++;
    return TRUE;
}

void WaveOut::BufferAdd()
{
    boost::lock_guard<boost::mutex> m_csLock(mutex);
    m_iBufferNum ++;
}

void WaveOut::BufferSub()
{
    boost::lock_guard<boost::mutex> m_csLock(mutex);
    m_iBufferNum --;
}

int WaveOut::GetBufferNum()
{
    int iTemp;
    boost::lock_guard<boost::mutex> m_csLock(mutex);
    iTemp = m_iBufferNum;
    return iTemp;

}

BOOL WaveOut::SetFormatByFile(std::string file)
{
#pragma pack(push, 1)
    struct FileHeader
    {
        char cFlagFiff[4];
        unsigned __int32 iFileLen;
        char cFlagWave[4];
        char cFlagFmt[4];
        char cResove[4];
        unsigned __int16 cFormat;
        unsigned __int16 cChannel;
        unsigned __int32 cSample;
        unsigned __int32 cBytePerSec;
        unsigned __int16 cByteprocess;
        unsigned __int16 cBit;
        char cFlagDat[4];
        unsigned __int32 iLenData;
    };
#pragma pack(pop)
    FILE *fp = fopen(file.c_str(), "rb");

    struct FileHeader head;
    fread((void *)&head,1,sizeof (head),fp);
    fclose(fp);

    this->SetChannel (head.cChannel);
    this->SetSample (head.cSample);
    this->SetBit (head.cBit);
    printf("%d\n%d\n%d\n",head.cChannel,head.cSample,head.cBit);
    //system("pause");
    return TRUE;
}
