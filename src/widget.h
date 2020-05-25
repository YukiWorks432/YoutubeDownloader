#ifndef YDR_WIDGET_H_
#define YDR_WIDGET_H_

#include "ui_widget.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QGraphicsView>
#include <QtCore/QtCore>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtCore/QTextCodec>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QAbstractAnimation>
#include <QtGui/QtGui>
#include <QtGui/QClipboard>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#define NO_MIN_MAX
#include <windows.h>
#include <filesystem>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <regex>
#include <string>
#include <omp.h>
#include <unordered_map>
#include <fstream>
#include <picojson>
#include <chrono>
#include <wchar.h>
#include "cglob.hpp"
#include "strconv.hpp"
using std::cout; using std::wcout; using std::endl;
using std::array; using std::vector; using std::string; using std::wstring; using std::to_string;
using std::unordered_map;
namespace fs = std::filesystem;
using namespace std::literals::string_literals;

inline QString operator""_q(const char *str, size_t len)
{	return QString(str);	}
inline QString operator+(QString qstr, std::string str)
{	return qstr + QString::fromStdString(str);	}
inline string operator+(string str_, int n)
{	return str_ + to_string(n); }

inline void stop()
{	system("pause > nul");	}
inline std::vector<std::string> split(const std::string &str, const char &del) noexcept{
    int first = 0;
    int last = str.find_first_of(del);
 
    std::vector<std::string> result;
 
    while (first < str.size()) {
        std::string subStr(str, first, last - first);
 
        result.push_back(subStr);
 
        first = last + 1;
        last = str.find_first_of(del, first);
 
        if (last == std::string::npos) {
            last = str.size();
        }
    }
 
    return result;
}
inline void delList(string &s) noexcept{
	if (s.empty()) return;
	std::smatch t;
	if ((s.find("playlist") == string::npos) && ((s.find("?list=") != string::npos) || (s.find("&list=") != string::npos))) {
		s = s.substr(0, s.find("list=") - 1);
	}
}
inline void normalizeTitle(string &targetStr) noexcept{
    const char CR = '\r';
    const char LF = '\n';
	const char SS = '/';
	const char BS = '\\';
    string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF) {
			if(c == ' ' || c == '\t') { continue; }
			if (c == SS || c == BS) { destStr += '-'; continue; }
			destStr += c;
		}
    }
    targetStr = std::move(destStr);
}
inline void normalize(string &targetStr) noexcept{
    const char CR = '\r';
    const char LF = '\n';
    string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF) destStr += c;
    }
    targetStr = std::move(destStr);
}
inline string HexToRGBA(const string &Hex, const int &a) noexcept{
	string rgba = "rgba(";
	const int r = std::stoi(Hex.substr(1, 2), nullptr, 16);
	const int g = std::stoi(Hex.substr(3, 2), nullptr, 16);
	const int b = std::stoi(Hex.substr(5, 2), nullptr, 16);
	rgba += to_string(r) + ","s + to_string(g) + ","s + to_string(b) + ","s + to_string(a) + ")"s;
	return rgba;
}

inline string get_last_line(const string &s__) noexcept{
	std::size_t p = s__.find_last_of('\n');
	if (p == string::npos) 	return s__;
	else 					return s__.substr(p + 1);
}

inline bool decode_unicode_escape_to_utf8(const std::string& src, std::string& dst) {
	std::string result("");
	result.resize(src.size());

	for (size_t i = 0; i < result.size(); ++i) {
		result[i] = '\0';
	}

	size_t result_index = 0;
	bool in_surrogate_pair = false;
	size_t surrogate_buffer = 0;
	for (size_t i = 0; i < src.size(); ++i) {
		if (i + 1 < src.size() && src[i] == '\\' && src[i+1] == 'u') {
			size_t octet = 0;
			{  // calculate octet
				const char* const hex = &src[i + 2];
				for (int j = 0; j < 4; ++j) {
					octet *= 16;
					if ('0' <= hex[j] && hex[j] <= '9') {
						octet += static_cast<int>(hex[j] - '0');
					} else if ('a' <= hex[j] && hex[j] <= 'f') {
						octet += static_cast<int>(hex[j] - 'a' + 10);
					} else if ('A' <= hex[j] && hex[j] <= 'F') {
						octet += static_cast<int>(hex[j] - 'A' + 10);
					} else {
						return false;
					}
				}
			}
			{  // fill up sequence
				char* const sequence = &result[result_index];
				if (in_surrogate_pair) {
					if (0xdc00 <= octet && octet <= 0xdfff) {
					// low surrogate pair
					const size_t joined = surrogate_buffer + (octet & 0x03ff) + 0x10000;
					sequence[0] = (static_cast<char>(joined >> 18)  &  0x3) | 0xf0;
					sequence[1] = (static_cast<char>(joined >> 12)  & 0x3f) | 0x80;
					sequence[2] = (static_cast<char>(joined >> 6)   & 0x3f) | 0x80;
					sequence[3] = (static_cast<char>(joined & 0xff) & 0x3f) | 0x80;
					result_index += 4;
					in_surrogate_pair = false;
					} else {
						return false;
					}
				} else if (octet < 0x7f) {
					sequence[0] = static_cast<char>(octet) & 0x7f;
					result_index += 1;
				} else if (octet < 0x7ff) {
					sequence[0] = (static_cast<char>(octet >> 6) & 0xdf) | 0xc0;
					sequence[1] = (static_cast<char>(octet)      & 0x3f) | 0x80;
					result_index += 2;
				} else if (0xdbff) {
					// high surrogate pair
					in_surrogate_pair = true;
					surrogate_buffer = (octet & 0x03ff) * 0x400;
				} else {
					sequence[0] = (static_cast<char>(octet >> 12) & 0x0f) | 0xe0;
					sequence[1] = (static_cast<char>(octet >> 6)  & 0x3f) | 0x80;
					sequence[2] = (static_cast<char>(octet)       & 0x3f) | 0x80;
					result_index += 3;
				}
			}
			i += 5;  // \\uXXXX is 6 bytes, so + 5 here, and + 1 in next loop
		} else {  // not unicode
			if (in_surrogate_pair) {
				return false;
			}
			result[result_index] = src[i];
			result_index += 1;
		}
		// next char
	}
	result.resize(result_index);
	dst.swap(result);
	return true;
}

struct pallet {
	string wincolor;
	string fracolor;
	string textcolor;
	string textcolor_h;
	string textcolor_l;
	string selection;
	string ST;
	string border_w;
	string border_c;
	string qbombobox_hover;
	string custom;
};

class Widget : public QWidget, public Ui::Widget {
    Q_OBJECT

	public:
		// コンストラクタ
		// 引数のQWidgetのポインタを渡すとそのウィジェットを親とする
		explicit Widget(QWidget *parent = nullptr);
		~Widget();
		void closeEvent(QCloseEvent *e);
		void mousePressEvent(QMouseEvent *e);
		void mouseReleaseEvent(QMouseEvent *e);
		void mouseMoveEvent(QMouseEvent *e);
		void addLOG(string s);
		void addLOG(wstring ws);
		void addLOG(QString qs);
		void addLOG(const char* cs);
		void addLOGasis(const string s);
		void addLOGasis(const wstring ws);
		void addLOGasis(const QString qs);
		void addLOGasis(const char* cs);
		void openDialogAnime();
		void closeDialogAnime();
		bool dled();
		std::mutex *const mtx;
		vector<string> moods;
		std::unordered_map<string, pallet*> pallets;

		inline void lock() noexcept
		{ mtx->lock(); }
		inline void unlock() noexcept
		{ mtx->unlock(); }
		inline string setmood(const string &m) noexcept
		{ return mood = m; }
		inline string getmood() noexcept
		{ return mood; }
		inline void setNowPallet(const pallet &p__) noexcept
		{ nowPallet = p__; }
		inline void setFFmpeg(const string &ffdir__) noexcept
		{ ffdir = ffdir__; }
		

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	// オリジナルのsignalは自ら定義する必要がある
	signals:

	// slotsで宣言するとスロットとしても使える
	private slots:
		void SelectDir();
		void Download();
		void ClipPaste();
		void updateLOG();
		void DownloadEnd();
		void goLink();
		void openDialog();
		void updateQuaNum(int value);

	private:
		pallet nowPallet;
		std::thread thr_dl;
		QTimer *const LOGt;
		QTimer *const timer;
		QPoint offset = QPoint(0, 0);
		std::string logs;
		string mood;
		string ffdir;
		std::atomic<bool> ENDDL = false;
		std::atomic<bool> isDrag = false;
		std::atomic<bool> hovering = false;
		std::atomic<bool> isOpen = false;
		const unsigned int msec = 10;
};

#endif // EXAMPLE_WIDGET_H_