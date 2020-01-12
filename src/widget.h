#ifndef _EXSAMPLE_WIDGET_H_
#define _EXSAMPLE_WIDGET_H_

#include "ui_widget.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QtCore>
#include <QtCore/QProcess>
#include <QtGui/QtGui>
#include <QtGui/QClipboard>
#include <thread>
#include <mutex>
#define NO_MIN_MAX
#include <windows.h>
#include <filesystem>

class Widget : public QWidget, public Ui::Widget {
    Q_OBJECT

	public:
		// コンストラクタ
		// 引数のQWidgetのポインタを渡すとそのウィジェットを親とする
		explicit Widget(QWidget *parent = nullptr);
		~Widget();
		std::thread thr_dl;
		std::thread thr_log;

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