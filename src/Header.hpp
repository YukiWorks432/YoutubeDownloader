#ifndef _YOUTUBEDOWNLOADER_HPP_
#define _YOUTUBEDOWNLOADER_HPP_

#include "widget.h"

class YDR {
	private:
		Widget *const ui;
		const string URL, outDir;
		const uint8_t VAA;
		const char *const AC;
		const bool ex;
	
	public:
		explicit YDR(Widget *const iui, const string iURL, const string ioutDir, const uint8_t iVAA, const char *const iAC, const bool iex)
		: ui(iui), URL(iURL), outDir(ioutDir), VAA(iVAA), AC(iAC), ex(iex) {};
		void Download() const;
		
		const static uint8_t Video = 0b0001;
		const static uint8_t Audio = 0b0010;
};

namespace cglob {
	string glob(const char *sp);
	string glob(string sp);
	static const string nfound = "0x0001"s;
};

#endif