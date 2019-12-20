#pragma once
#include "Info.h"
#include <string>
#include <vector>
#include <memory>


namespace wav
{
	// 読み込み
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
}
