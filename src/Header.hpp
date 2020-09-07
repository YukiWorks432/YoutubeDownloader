#ifndef YOUTUBEDOWNLOADER_HPP_
#define YOUTUBEDOWNLOADER_HPP_

#include "widget.h"

class YDR {
	public:
		explicit YDR(	Widget *const iui, const string iURL, const fs::path ioutDir, const string iffdir,
						const uint8_t iVAA, const int iAC, const unsigned int ibt, const unsigned int ivbr,const unsigned int icrf,
						const unsigned int ivmode, const bool ith, const bool ileave, const bool iex, const int idmode)
		: ui(iui), URL(iURL), outDir(ioutDir), ffdir(iffdir), VAA(iVAA), AC(iAC), bt(ibt), vbr(ivbr), crf(icrf),
		  vmode(ivmode), th(ith), leave(ileave), ex(iex), dmode(idmode) {};
		void Download();
		inline void lock() noexcept
		{ ui->mtx->lock(); }
		inline void unlock() noexcept
		{ ui->mtx->unlock(); }
		static const uint8_t Video = 0b0001;
		static const uint8_t Audio = 0b0010;
		enum ACs {
			best 	=	0b0000,
			wav 	=	0b0001,
			flac 	=	0b0010,
			alac 	=	0b0011,
			aac		=	0b0100,
			mp3		=	0b0101,
		};

	private:
		Widget *const ui;
		const string URL, ffdir;
		const fs::path outDir;
		const uint8_t VAA;
		const int AC;
		const unsigned int bt, vbr, crf, vmode;
		const bool th;
		const bool leave;
		const bool ex;
		const int dmode;
};

#endif