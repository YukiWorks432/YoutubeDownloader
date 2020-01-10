#include <QtWidgets/QApplication>
#include <iostream>
#include "widget.h"
#include "Header.hpp"
using std::cout; using std::endl; using std::to_string;

void YDR::Download() const {
	string cmd("youtube-dl -e" + URL);
	string title = to_string(system(cmd.c_str()));
	cout << title << endl;
	// cmd = string("youtube -dl --ffmpeg--location \"./ffmpeg/bin/ffmpeg.exe\" --output \"") + outDir + string("\\");
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