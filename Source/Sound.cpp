#include "PCH.h"
#include "Sound.h"
#include "ResourceLoader.h"

WavFile::WavFile()
{}

bool WavFile::Load(int resource)
{
	ResourceLoader resLoader(resource, L"WAVDATA");

	resLoader.Read(chunkId, sizeof(BYTE) * 4);
	resLoader.Read(&chunkSize, sizeof(UINT));
	resLoader.Read(format, sizeof(BYTE) * 4);
	resLoader.Read(subchunk1Id, sizeof(BYTE) * 4);
	resLoader.Read(&subchunk1Size, sizeof(uint32_t));
	resLoader.Read(&audioFormat, sizeof(uint16_t));
	resLoader.Read(&numChannels, sizeof(uint16_t));
	resLoader.Read(&sampleRate, sizeof(uint32_t));
	resLoader.Read(&byteRate, sizeof(uint32_t));
	resLoader.Read(&blockAlign, sizeof(uint16_t));
	resLoader.Read(&bitsPerSample, sizeof(uint16_t));
	resLoader.Read(subchunk2Id, sizeof(BYTE) * 4);
	resLoader.Read(&dataSize, sizeof(UINT));

	data.resize(dataSize);
	resLoader.Read(data.data(), dataSize);

	return true;
}

void WavFile::unLoad()
{
	data.clear();
}

SoundManager::SoundManager()
{}

SoundManager::~SoundManager()
{
	if (m_MasterVoice)
		m_MasterVoice->DestroyVoice();
}

HRESULT SoundManager::Init(int numChannels)
{
	HRESULT hr = CreateMasterVoice();
	if (SUCCEEDED(hr))
	{
		m_NumChannels = numChannels;
		SetWaveFormat();
		CreateChannels();
	}
	return hr;
}

void SoundManager::Play(WavFile& wavfile, float volume, float pitchratio)
{
	// Find an available Channel to play sound on
	int index = -1;
	for (int i = 0; i < m_NumChannels; i++)
	{
		if (!m_Channels[i].m_IsBusy)
		{
			index = i;
			m_Channels[i].m_IsBusy = true; // mark channel as in use
			break;
		}
	}
	if (index != -1)
	{
		XAUDIO2_BUFFER xAudio2buffer = {};
		xAudio2buffer.pContext = &m_Channels[index];
		xAudio2buffer.AudioBytes = wavfile.getAudioDataSize(); // size of data in bytes
		xAudio2buffer.pAudioData = wavfile.getAudioData(); // pointer to data
		xAudio2buffer.Flags = XAUDIO2_END_OF_STREAM;
		m_Channels[index].m_SourceVoice->SetVolume(volume);
		m_Channels[index].m_SourceVoice->SetFrequencyRatio(pitchratio);
		m_Channels[index].m_SourceVoice->SubmitSourceBuffer(&xAudio2buffer);
		m_Channels[index].m_SourceVoice->Start();
	}
}

HRESULT SoundManager::CreateMasterVoice()
{
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Error: Could not initialize COM system");
		MessageBoxW(NULL, L"Error: Could not initialize COM system", L"XAudio2 Error", MB_OK);
		return hr;
	}
	hr = XAudio2Create(&m_XAudio2Engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Error: Could not Create XAudio2 Engine");
		MessageBox(NULL, L"Error: Could not Create XAudio2 Engine", L"XAudio2 Error", MB_OK);
		return hr;
	}
	hr = m_XAudio2Engine->CreateMasteringVoice(&m_MasterVoice);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Error: Could not Create Master Voice");
		MessageBoxA(NULL, "Error: Could not Create Master Voice", "XAudio2 Error", MB_OK);
		return hr;
	}
	return hr;
}

void SoundManager::SetWaveFormat()
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nChannels = 2;
	m_WaveFormat.wBitsPerSample = 16;  // 16
	m_WaveFormat.nBlockAlign = (m_WaveFormat.nChannels * m_WaveFormat.wBitsPerSample) / 8;
	m_WaveFormat.nSamplesPerSec = 44100;  // 44100  24000
	m_WaveFormat.nAvgBytesPerSec = 176400;  // 176400  96000
}

void SoundManager::CreateChannels()
{
	m_Channels.resize(m_NumChannels);
	for (int i = 0; i < m_NumChannels; i++)
	{
		m_Channels[i].Init(m_XAudio2Engine.Get(), &m_WaveFormat);
	}
}

SoundManager::SourceChannel::SourceChannel() : m_SourceVoice(nullptr), m_IsBusy(false)
{}

SoundManager::SourceChannel::~SourceChannel()
{
	if (m_SourceVoice)
	{
		m_SourceVoice->Stop();
		m_SourceVoice->FlushSourceBuffers();
		m_SourceVoice->DestroyVoice();
	}
}

void SoundManager::SourceChannel::Init(IXAudio2* XAudio2Engine, WAVEFORMATEX* WaveFormat)
{
	HRESULT hr = XAudio2Engine->CreateSourceVoice(&m_SourceVoice, WaveFormat, 0, 2.0f, this);
}
