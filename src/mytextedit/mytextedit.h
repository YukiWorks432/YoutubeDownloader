#ifndef _MYTEXTEDIT_H_
#define _MYTEXTEDIT_H_

#include <QtWidgets/QTextEdit>
#include <QtGui/QMouseEvent>

class MYTextEdit : public QTextEdit {
	Q_OBJECT

	public:
		explicit MYTextEdit(QWidget *parent = nullptr);

	signals:
		void rightClicked();
	
	private:
		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::RightButton) emit rightClicked();
		}
};

#endif