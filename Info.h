#pragma once

namespace snd
{
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
}
