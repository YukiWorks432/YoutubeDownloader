#ifndef _MYTEXTEDIT_H_
#define _MYTEXTEDIT_H_

#include <QtWidgets/QTextEdit>
#include <QtGui/QMouseEvent>

class MYTextEdit : public QTextEdit {
	Q_OBJECT

	public:
		explicit MYTextEdit(QWidget *parent = nullptr);
		void keyPressEvent(QKeyEvent *event) {
			if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
				event->ignore();
			else
				QTextEdit::keyPressEvent(event);
        }

	signals:
		void rightClicked();
	
	private:
		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::RightButton) emit rightClicked();
			else QTextEdit::mousePressEvent(e);
		}
};

#endif