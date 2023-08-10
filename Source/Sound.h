#pragma once
#include "PCH.h"

class WavFile
{
public:
	WavFile();
	bool Load(int resource);
	inline BYTE* getAudioData()
	{
		return data.data();
	}
	inline UINT getAudioDataSize()
	{
		return (UINT)data.size();
	}
private:
	void unLoad();
	unsigned char chunkId[4] = {};
	uint32_t chunkSize = 0;
	unsigned char format[4] = {};
	unsigned char subchunk1Id[4] = {};
	uint32_t subchunk1Size = 0;
	uint16_t audioFormat = 0;
	uint16_t numChannels = 0;
	uint32_t sampleRate = 0;
	uint32_t byteRate = 0;
	uint16_t blockAlign = 0;
	uint16_t bitsPerSample = 0;
	unsigned char subchunk2Id[4] = {};
	uint32_t dataSize = 0;
	std::vector<BYTE> data;
};

class SoundManager
{
public:
	SoundManager();
	~SoundManager();
	HRESULT Init(int numChannels);
	void Play(WavFile& wavfile, float volume, float pitchratio);
private:
	void SetWaveFormat();
	HRESULT CreateMasterVoice();
	void CreateChannels();
	class SourceChannel : public IXAudio2VoiceCallback
	{
	public:
		SourceChannel();
		~SourceChannel();
		void Init(IXAudio2* XAudio2Engine, WAVEFORMATEX* WaveFormat);
	protected:
		void STDMETHODCALLTYPE OnStreamEnd() noexcept override {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() noexcept override {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override {}
		void STDMETHODCALLTYPE OnBufferEnd(void* context) noexcept override { m_IsBusy = false; }
		void STDMETHODCALLTYPE OnBufferStart(void* context) noexcept override {}
		void STDMETHODCALLTYPE OnLoopEnd(void* context) noexcept override {}
		void STDMETHODCALLTYPE OnVoiceError(void* context, HRESULT error) noexcept override {}
	private:
		IXAudio2SourceVoice* m_SourceVoice;
		bool m_IsBusy;
		friend class SoundManager;
	};
	Microsoft::WRL::ComPtr<IXAudio2> m_XAudio2Engine;
	IXAudio2MasteringVoice* m_MasterVoice = nullptr;
	std::vector<SourceChannel> m_Channels;
	WAVEFORMATEX m_WaveFormat = {};
	int m_NumChannels = 0;
};