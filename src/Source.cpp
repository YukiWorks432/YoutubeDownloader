#include <QtWidgets/QApplication>
#include "widget.h"

int main(int argc, char **argv) {
    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
	// widget->setWindowFlags(Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
    widget->show();

    // ループに入る
    return app.exec();
}