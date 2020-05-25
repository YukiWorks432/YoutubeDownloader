#ifndef MYFRAME_HPP_
#define MYFRAME_HPP_

#include <QtWidgets/QFrame>

class MYFrame : public QFrame {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        explicit MYFrame(QWidget *parent = (QWidget *)nullptr);
        void setOpacity(qreal value);
        qreal opacity();
};

#endif