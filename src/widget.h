#ifndef _EXSAMPLE_WIDGET_H_
#define _EXSAMPLE_WIDGET_H_

#include "ui_widget.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtGui/QClipboard>
#define NO_MIN_MAX
#include <windows.h>
#include <filesystem>

class Widget : public QWidget, public Ui::Widget {
    Q_OBJECT

	public:
		// コンストラクタ
		// 引数のQWidgetのポインタを渡すとそのウィジェットを親とする
		Widget(QWidget *parent = nullptr);

	// シグナルは宣言だけで定義はしない
	// 自前で定義するとMOCが生成するのと被ってしまう
	signals:

	// slotsで宣言するとスロットとしても使える
	private slots:
		void SelectDir();
		void Download();
		void ClipPaste();

};

#endif // _EXAMPLE_WIDGET_H_