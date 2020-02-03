#ifndef EXSAMPLE_WIDGET_H_
#define EXSAMPLE_WIDGET_H_

#include "ui_widget.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QtCore>
#include <QtCore/QProcess>
#include <QtCore/QThread>
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
#include "cglob.hpp"
using std::cout; using std::wcout; using std::endl;
using std::array; using std::vector; using std::string; using std::wstring; using std::to_string;
namespace fs = std::filesystem;
using namespace std::literals::string_literals;

inline void delList(string &s) {
	if (s.empty()) return;
	std::smatch t;
	if ((s.find("playlist") == string::npos) && ((s.find("?list=") != string::npos) || (s.find("&list=") != string::npos))) {
		s = s.substr(0, s.find("list=") - 1);
	}
}
inline std::string WStringToString(std::wstring oWString) {
    // wstring → SJIS
    int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, (char *)NULL, 0, NULL, NULL);
 
    // バッファの取得
    CHAR* cpMultiByte = new CHAR[iBufferSize];
 
    // wstring → SJIS
    WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte, iBufferSize, NULL, NULL );
 
    // stringの生成
    std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
 
    // バッファの破棄
    delete[] cpMultiByte;
 
    // 変換結果を返す
    return oRet;
}
inline void normalizeTitle(string &targetStr) {
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
inline string normalize(string &targetStr) {
    const char CR = '\r';
    const char LF = '\n';
    string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF) destStr += c;
    }
    return destStr;
}

class Widget : public QWidget, public Ui::Widget {
    Q_OBJECT

	public:
		// コンストラクタ
		// 引数のQWidgetのポインタを渡すとそのウィジェットを親とする
		explicit Widget(QWidget *parent = nullptr);
		~Widget();
		void closeEvent(QCloseEvent *e);
		void addLOG(string s);
		void addLOG(wstring ws);
		void addLOG(const char* s);
		bool dled();
		std::mutex* mtx;

	// オリジナルのsignalは自ら定義する必要がある
	signals:

	// slotsで宣言するとスロットとしても使える
	private slots:
		void SelectDir();
		void Download();
		void ClipPaste();
		void updateLOG();
		void DownloadEnd();

	private:
		std::thread thr_dl;
		QTimer* LOGt;
		QTimer* timer;
		const unsigned int msec = 100;
		std::atomic<bool> ENDDL;
		string logs;
};

#endif // EXAMPLE_WIDGET_H_