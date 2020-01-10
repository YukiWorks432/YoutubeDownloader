#ifndef _YOUTUBEDOWNLOADER_HPP_
#define _YOUTUBEDOWNLOADER_HPP_

#include <string>
using std::string;

class YDR {
	public:
		explicit YDR(const string iURL, const string ioutDir, const uint8_t iVAA, const bool iex) : URL(iURL), outDir(ioutDir), VAA(iVAA), ex(iex) {};
		void Download() const;

	private:
		const string URL, outDir;
		const uint8_t VAA;
		const bool ex;
};

#endif