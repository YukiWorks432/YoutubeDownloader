#ifndef YOUTUBEDOWNLOADER_HPP_
#define YOUTUBEDOWNLOADER_HPP_

#include "widget.h"

class YDR {
	public:
		explicit YDR(Widget *const iui, const string iURL, const fs::path ioutDir, const string iffdir, const uint8_t iVAA, const string iAC, const bool ith, const bool iex)
		: ui(iui), URL(iURL), outDir(ioutDir), ffdir(iffdir), VAA(iVAA), AC(iAC), th(ith), ex(iex) {};
		void Download();
		inline void lock() noexcept
		{ ui->mtx->lock(); }
		inline void unlock() noexcept
		{ ui->mtx->unlock(); }
		const static uint8_t Video = 0b0001;
		const static uint8_t Audio = 0b0010;

	private:
		Widget *const ui;
		const string URL, ffdir;
		const fs::path outDir;
		const uint8_t VAA;
		const string AC;
		const bool th;
		const bool ex;
};

#endif