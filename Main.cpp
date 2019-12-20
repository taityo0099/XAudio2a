#include<xaudio2.h>
#include<wrl.h>
#include<ks.h>
#include<ksmedia.h>
#include<vector>
#include"XAudio2/XAudio2.h"
#include<Windows.h>
#include"Info.h"
#include<string>
#include<vector>
#include<memory>

#pragma comment(lib,"XAudio2.lib")

//スピーカー一覧
const unsigned long spk[] = {
	KSAUDIO_SPEAKER_MONO,
	KSAUDIO_SPEAKER_STEREO,
	KSAUDIO_SPEAKER_STEREO | SPEAKER_LOW_FREQUENCY,
	KSAUDIO_SPEAKER_QUAD,
	0,
	KSAUDIO_SPEAKER_5POINT1,
	0,
	KSAUDIO_SPEAKER_7POINT1_SURROUND
};

#define Desctory(X) { if((X) != nullptr) (X)->DestroyVoice(); (X) = nullptr; }

struct Info
{
	//サンプリング周波数
	unsigned int sample;
	//量子化ビット数
	unsigned char bit;
	//チャンネル数
	unsigned char channel;

	//全初期化
	Info()
	{
		sample = bit = channel = 0;
	}

	//値で初期化
	Info(const unsigned int& s, const unsigned char& b, const unsigned char& c) {
		sample = s;
		bit = b;
		channel = c;
	}

	//
	Info(const Info& info)
	{
		(*this) = info;
	}

	void operator=(const Info& info)
	{
		sample = info.sample;
		bit = info.bit;
		channel = info.channel;
	}
};

 int Load(const std::string& fileName, std::shared_ptr<std::vector<float>>& wave, snd::Info& info)
{
	FILE* fp = nullptr;
	if (fopen_s(&fp, fileName.c_str(), "rb") != 0)
	{
		return -1;
	}

	//riffチャンク
	char chunckID[4];
	fread(&chunckID[0], sizeof(chunckID), 1, fp);
	unsigned long chunkSize = 0;
	fread(&chunkSize, sizeof(chunkSize), 1, fp);
	char formType[4];
	fread(&formType[0], sizeof(formType), 1, fp);

	//fmtチャンク
	fread(&chunckID[0], sizeof(chunckID), 1, fp);
	fread(&chunkSize, sizeof(chunkSize), 1, fp);
	short waveFormatType = 0;
	fread(&waveFormatType, sizeof(waveFormatType), 1, fp);
	short channel = 0;
	fread(&channel, sizeof(channel), 1, fp);
	unsigned long samplesPerSec = 0;
	fread(&samplesPerSec, sizeof(samplesPerSec), 1, fp);
	unsigned long bytesPerSec = 0;
	fread(&bytesPerSec, sizeof(bytesPerSec), 1, fp);
	short blockSize = 0;
	fread(&blockSize, sizeof(blockSize), 1, fp);
	short bitsPerSample;
	fread(&bitsPerSample, sizeof(bitsPerSample), 1, fp);

	std::string id;
	unsigned long size = 0;
	do
	{
		fseek(fp, size, SEEK_CUR);
		fread(&chunckID[0], sizeof(chunckID), 1, fp);
		id.assign(&chunckID[0], sizeof(chunckID));
		fread(&size, sizeof(size), 1, fp);

	} while (id != "data");

	info.sample = samplesPerSec;
	info.bit = bitsPerSample;
	info.channel = channel;
	wave = std::make_shared<std::vector<float>>(size / (info.bit / 8));

	for (float& i : *wave)
	{
		//16bit限定
		short tmp = 0;
		fread(&tmp, sizeof(tmp), 1, fp);
		i = float(tmp) / float(0xffff / 2);
	}

	fclose(fp);
	return 0;
}
int main()
{
	auto hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	_ASSERT(hr == S_OK);

	bool play = false;
	bool key = false;

	

	{
		//オーディオ
		Microsoft::WRL::ComPtr<IXAudio2>audio = nullptr;
		{
			hr = XAudio2Create(&audio);
			_ASSERT(hr == S_OK);
		}

		//マスタリング
		IXAudio2MasteringVoice* mastering = nullptr;
		{
			hr = audio->CreateMasteringVoice(&mastering);
			_ASSERT(hr == S_OK);
		}

		//ソースボイス
		IXAudio2SourceVoice* voice = nullptr;
		{
			Info info;

			//波形データの情報
			WAVEFORMATEXTENSIBLE fmt{};
			fmt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
			fmt.Format.nSamplesPerSec = sample;
			fmt.Format.wBitsPerSample = bit;
			fmt.Format.nChannels = channel;
			fmt.Format.nBlockAlign = fmt.Format.nChannels * fmt.Format.wBitsPerSample / 8;
			fmt.Format.nAvgBytesPerSec = fmt.Format.nSamplesPerSec * fmt.Format.nBlockAlign;
			fmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			fmt.dwChannelMask = spk[fmt.Format.nChannels - 1];
			fmt.Samples.wValidBitsPerSample = fmt.Format.wBitsPerSample;
			fmt.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
			hr = audio->CreateSourceVoice(&voice, (WAVEFORMATEX*)&fmt, 0, 1.0f, nullptr);
			_ASSERT(hr == S_OK);
		}

		//波形をバッファに乗せる
		{
			XAUDIO2_BUFFER buffer{};
			//buffer.AudioBytes = sizeof();
			//buffer.pAudioData = (unsigned char*)();
			hr = voice->SubmitSourceBuffer(&buffer);
			_ASSERT(hr == S_OK);
		}
		//再生
		{
			hr = voice->Start();
			_ASSERT(hr == S_OK);
		}

		//停止
		{
			hr = voice->Stop();
			_ASSERT(hr == S_OK);
		}
		Desctory(voice);
		Desctory(mastering);

	}
	CoUninitialize();
	return 0;
}
