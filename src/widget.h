#ifndef EXSAMPLE_WIDGET_H_
#define EXSAMPLE_WIDGET_H_

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
#include <QtCore/QPropertyAnimation>
#include <QtCore/QAbstractAnimation>
#include <QtGui/QtGui>
#include <QtGui/QClipboard>
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
#include "cglob.hpp"
using std::cout; using std::wcout; using std::endl;
using std::array; using std::vector; using std::string; using std::wstring; using std::to_string;
namespace fs = std::filesystem;
using namespace std::literals::string_literals;

inline QString operator""_q(const char *str, size_t len)
{	return QString(str);	}
inline QString operator+(QString qstr, std::string str)
{	return qstr + QString::fromStdString(str);	}
inline string operator+(string str_, int n)
{	return str_ + to_string(n); }

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
			if (c == SS || c == BS) destStr += '-';
			else 					destStr += c;
		}
    }
    targetStr = std::move(destStr);
}
inline string normalize(string &targetStr) noexcept{
    const char CR = '\r';
    const char LF = '\n';
    string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF) destStr += c;
    }
    return destStr;
}
inline string HexToRGBA(const string &Hex, const int &a) noexcept{
	string rgba = "rgba(";
	const int r = std::stoi(Hex.substr(1, 2), nullptr, 16);
	const int g = std::stoi(Hex.substr(3, 2), nullptr, 16);
	const int b = std::stoi(Hex.substr(5, 2), nullptr, 16);
	rgba += to_string(r) + ","s + to_string(g) + ","s + to_string(b) + ","s + to_string(a) + ")"s;
	return rgba;
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
		void addLOG(const char* s);
		bool dled();
		std::mutex *const mtx;
		vector<string> moods;
		std::unordered_map<string, pallet*> pallets;

		inline string setmood(const string &m) noexcept
		{ return mood = m; }
		inline string getmood() noexcept
		{ return mood; }
		inline void setNowPallet(const pallet &p__) noexcept
		{ nowPallet = p__; }

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

	private:
		pallet nowPallet;
		std::thread thr_dl;
		QTimer *const LOGt;
		QTimer *const timer;
		QPoint offset = QPoint(0, 0);
		string logs;
		string mood;
		std::atomic<bool> ENDDL = false;
		std::atomic<bool> isDrag = false;
		std::atomic<bool> hovering = false;
		const unsigned int msec = 10;
};

#endif // EXAMPLE_WIDGET_H_