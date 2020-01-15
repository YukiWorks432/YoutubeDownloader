#ifndef _YOUTUBEDOWNLOADER_HPP_
#define _YOUTUBEDOWNLOADER_HPP_

#include "widget.h"

class YDR {
	public:
		explicit YDR(const string iURL, const string ioutDir, const uint8_t iVAA, const char *&iAC, const bool iex) : URL(iURL), outDir(ioutDir), VAA(iVAA), AC(iAC), ex(iex) {};
		void Download(const Widget * const widget) const;
		const static uint8_t Video = 0b0001;
		const static uint8_t Audio = 0b0010;

	private:
		const string URL, outDir;
		const uint8_t VAA;
		const char *&AC;
		const bool ex;
};

namespace cglob {
	string glob(const char *sp);
	static const string nfound = "0x0001"s;
};

namespace cwrap {
	bool CreateProcessW (	LPWSTR lpCommandLine,
							LPSTARTUPINFOW lpStartupInfo,
							LPPROCESS_INFORMATION lpProcessInformation);
	LPWSTR StdStr2Wchar(string &temp);
}

#endif