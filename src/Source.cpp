#include <QtWidgets/QApplication>
#include <iostream>
#include <sstream>
#include "widget.h"
#include "Header.hpp"
using std::cout; using std::endl; using std::to_string;

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

	std::stringbuf buf;
	std::streambuf *prev = std::cout.rdbuf(&buf);

	string cmd = "youtube-dl -e " + URL;
	system(cmd.c_str());
	
	string title = buf.str();
	widget->LOG->setText(Widget::tr(title.c_str()));

	// cmd = string("youtube -dl --ffmpeg--location \"./ffmpeg/bin/ffmpeg.exe\" --output \"") + outDir + string("\\");
	
	std::cout.rdbuf(prev);

	if (ex) exit(0);
}

int main(int argc, char **argv) {
    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
    widget->show();

    // ループに入る
    return app.exec();
}