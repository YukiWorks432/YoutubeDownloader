#include <QtWidgets/QApplication>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <regex>
#include "widget.h"
#include "Header.hpp"
using std::cout; using std::endl; using std::to_string;

std::mutex mtx;
QString logs;

void updateLOG(Widget *ui) {
	QString ago;
	while (true) {
		if (ui->LOG->text() != logs) {
			ago = ui->LOG->text();
			ui->LOG->setText(ago + logs);
		}
	}
}

template<class T> std::vector<std::string> split(const std::string& s, const T& separator, bool ignore_empty = 0, bool split_empty = 0) {
  struct {
    auto len(const std::string&             s) { return s.length(); }
    auto len(const std::string::value_type* p) { return p ? std::char_traits<std::string::value_type>::length(p) : 0; }
    auto len(const std::string::value_type  c) { return c == std::string::value_type() ? 0 : 1; /*return 1;*/ }
  } util;
  
  if (s.empty()) { /// empty string ///
    if (!split_empty || util.len(separator)) return {""};
    return {};
  }
  
  auto v = std::vector<std::string>();
  auto n = static_cast<std::string::size_type>(util.len(separator));
  if (n == 0) {    /// empty separator ///
    if (!split_empty) return {s};
    for (auto&& c : s) v.emplace_back(1, c);
    return v;
  }
  
  auto p = std::string::size_type(0);
  while (1) {      /// split with separator ///
    auto pos = s.find(separator, p);
    if (pos == std::string::npos) {
      if (ignore_empty && p - n + 1 == s.size()) break;
      v.emplace_back(s.begin() + p, s.end());
      break;
    }
    if (!ignore_empty || p != pos)
      v.emplace_back(s.begin() + p, s.begin() + pos);
    p = pos + n;
  }
  return v;
}

void YDR::Download(const Widget *widget) const {
	if (VAA == 0) {
		widget->LOG->setText(Widget::tr("動画か音声か、\n一つは選択してください(複数可)"));
		return;
	}
	if (URL.substr(0, 8).find("https://") == string::npos) {
		widget->LOG->setText(Widget::tr("URLが https:// から始まっていません"));
		return;
	}
	if (outDir.length() <= 2) {
		widget->LOG->setText(Widget::tr("出力先ディレクトリが不正です"));
		return;
	}

	string cmd = "youtube-dl -e " + URL;
	auto fp = popen(cmd.c_str(), "r");
	if (fp == NULL) return;
	auto buf = new char[256];
	while (!feof(fp)) {
		fgets(buf, 256, fp);
	}
	
	auto title = QString::fromLocal8Bit(buf);
	mtx.lock();
	logs = title;
	mtx.unlock();

	auto buf1 = new char[256];
	cmd = "youtube-dl -F " + URL;
	fp = popen(cmd.c_str(), "r");
	if (fp == NULL) return;
	while (!feof(fp)) {
		fgets(buf1, 256, fp);
	}

	auto Formats = split(string(buf), "\n", true);
	std::vector<string> VF, AF;
	for (const auto &e : Formats) {
		std::smatch tempV, tempA;
		if (e.find("video only") != string::npos) if (std::regex_search(e, tempV, std::regex("[0-9]{2,3}"))) VF.push_back(tempV.str());
		if (e.find("audio only") != string::npos) if (std::regex_search(e, tempA, std::regex("[0-9]{2,3}"))) AF.push_back(tempA.str());
	}

	//cmd = string("youtube-dl --ffmpeg--location \"./ffmpeg/bin/ffmpeg.exe\" -f ")
	//	+ VF[-1] + string(" --output \"") + outDir + string("\\") + string(buf)
	//	+ string(".$(ext)s\" --no-check-certificate --no-part \"") + URL + string("\"");

	delete[] buf, buf1;
	widget->DLButton->setText("ダウンロード開始");
	if (ex) exit(0); //*/
}

int main(int argc, char **argv) {
    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
    widget->show();

	widget->thr_log = std::thread([&](){ updateLOG(widget); });

    // ループに入る
    return app.exec();
}