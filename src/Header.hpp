#ifndef YOUTUBEDOWNLOADER_HPP_
#define YOUTUBEDOWNLOADER_HPP_

#include "widget.h"

class YDR {
	private:
		Widget *const ui;
		const string URL, outDir, ffdir;
		const uint8_t VAA;
		const char *const AC;
		const bool th;
		const bool ex;
	
	public:
		explicit YDR(Widget *const iui, const string iURL, const string ioutDir, const string iffdir, const uint8_t iVAA, const char *const iAC, const bool ith, const bool iex)
		: ui(iui), URL(iURL), outDir(ioutDir), ffdir(iffdir), VAA(iVAA), AC(iAC), th(ith), ex(iex) {};
		void Download() const;
		
		const static uint8_t Video = 0b0001;
		const static uint8_t Audio = 0b0010;
};

#endif