#include "myframe.h"

MYFrame::MYFrame(QWidget *parent) : QFrame::QFrame(parent) {}

void MYFrame::setOpacity(qreal value) {
    d_ptr->m_opacity = value;
    emit opacityChanged();
};

qreal opacity()
{ return d_ptr->m_opacity; }