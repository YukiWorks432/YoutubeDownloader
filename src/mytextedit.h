#ifndef MYTEXTEDIT_H_
#define MYTEXTEDIT_H_

#include <QtWidgets/QTextEdit>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <iostream>
#include <string>
#include <array>
#define NO_MIN_MAX
#include <windows.h>
#include "popen.hpp"

using std::string; using std::wstring; using std::cout; using std::endl; 
using namespace std::literals::string_literals;

class MYTextEdit : public QTextEdit {
	Q_OBJECT

	public:
		MYTextEdit(QWidget *parent = (QWidget *)nullptr);
		void keyPressEvent(QKeyEvent *e) {
			if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
				e->ignore();
				return;
			}
			QTextEdit::keyPressEvent(e);
        }

	signals:
		void rightClicked();
		void clickLink();
	
	private:
		void mousePressEvent(QMouseEvent *e) {
			if ((e->buttons() & Qt::LeftButton) && (e->modifiers() & Qt::ControlModifier)) {
				emit clickLink();
			}
			if (e->buttons() & Qt::RightButton) {
				emit rightClicked();
			}
			QTextEdit::mousePressEvent(e);
		}
};

#endif