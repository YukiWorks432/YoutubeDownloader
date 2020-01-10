/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include <mytextedit.h>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QFrame *URLFrame;
    QLabel *URLlabel;
    MYTextEdit *URLEntry;
    QFrame *DLFrame;
    QPushButton *DLButton;
    QCheckBox *ExitCheckBox;
    QFrame *VorA;
    QCheckBox *VideoCheckBox;
    QCheckBox *AudioCheckBox;
    QFrame *OutDirFrame;
    QLabel *ODLlabel;
    QTextEdit *ODEntry;
    QPushButton *SButton;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->setWindowModality(Qt::ApplicationModal);
        Widget->resize(640, 360);
        Widget->setMinimumSize(QSize(640, 360));
        Widget->setMaximumSize(QSize(640, 360));
        QFont font;
        font.setFamily(QString::fromUtf8("VDL \343\203\251\343\202\244\343\203\263\357\274\247 R"));
        font.setPointSize(14);
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
        font.setWeight(50);
        font.setStrikeOut(false);
        font.setKerning(true);
        font.setStyleStrategy(QFont::PreferAntialias);
        Widget->setFont(font);
        Widget->setCursor(QCursor(Qt::ArrowCursor));
        Widget->setMouseTracking(false);
        Widget->setTabletTracking(false);
        Widget->setContextMenuPolicy(Qt::NoContextMenu);
        Widget->setLayoutDirection(Qt::LeftToRight);
        URLFrame = new QFrame(Widget);
        URLFrame->setObjectName(QString::fromUtf8("URLFrame"));
        URLFrame->setGeometry(QRect(10, 10, 621, 81));
        URLFrame->setContextMenuPolicy(Qt::NoContextMenu);
        URLFrame->setFrameShape(QFrame::StyledPanel);
        URLFrame->setFrameShadow(QFrame::Raised);
        URLlabel = new QLabel(URLFrame);
        URLlabel->setObjectName(QString::fromUtf8("URLlabel"));
        URLlabel->setGeometry(QRect(10, 10, 51, 61));
        QFont font1;
        font1.setFamily(QString::fromUtf8("VDL \343\203\251\343\202\244\343\203\263\357\274\247 R"));
        URLlabel->setFont(font1);
        URLlabel->setContextMenuPolicy(Qt::NoContextMenu);
        URLEntry = new MYTextEdit(URLFrame);
        URLEntry->setObjectName(QString::fromUtf8("URLEntry"));
        URLEntry->setGeometry(QRect(90, 20, 521, 41));
        URLEntry->setContextMenuPolicy(Qt::NoContextMenu);
        DLFrame = new QFrame(Widget);
        DLFrame->setObjectName(QString::fromUtf8("DLFrame"));
        DLFrame->setGeometry(QRect(280, 270, 351, 80));
        DLFrame->setContextMenuPolicy(Qt::NoContextMenu);
        DLFrame->setFrameShape(QFrame::StyledPanel);
        DLFrame->setFrameShadow(QFrame::Raised);
        DLButton = new QPushButton(DLFrame);
        DLButton->setObjectName(QString::fromUtf8("DLButton"));
        DLButton->setGeometry(QRect(200, 20, 141, 41));
        QFont font2;
        font2.setFamily(QString::fromUtf8("VDL \343\203\251\343\202\244\343\203\263\357\274\247 R"));
        font2.setPointSize(10);
        DLButton->setFont(font2);
        DLButton->setContextMenuPolicy(Qt::NoContextMenu);
        ExitCheckBox = new QCheckBox(DLFrame);
        ExitCheckBox->setObjectName(QString::fromUtf8("ExitCheckBox"));
        ExitCheckBox->setGeometry(QRect(10, 20, 181, 41));
        QFont font3;
        font3.setPointSize(7);
        ExitCheckBox->setFont(font3);
        ExitCheckBox->setContextMenuPolicy(Qt::NoContextMenu);
        VorA = new QFrame(Widget);
        VorA->setObjectName(QString::fromUtf8("VorA"));
        VorA->setGeometry(QRect(10, 110, 621, 41));
        VorA->setContextMenuPolicy(Qt::NoContextMenu);
        VorA->setFrameShape(QFrame::StyledPanel);
        VorA->setFrameShadow(QFrame::Raised);
        VideoCheckBox = new QCheckBox(VorA);
        VideoCheckBox->setObjectName(QString::fromUtf8("VideoCheckBox"));
        VideoCheckBox->setGeometry(QRect(10, 0, 71, 41));
        QFont font4;
        font4.setPointSize(14);
        VideoCheckBox->setFont(font4);
        VideoCheckBox->setContextMenuPolicy(Qt::NoContextMenu);
        AudioCheckBox = new QCheckBox(VorA);
        AudioCheckBox->setObjectName(QString::fromUtf8("AudioCheckBox"));
        AudioCheckBox->setGeometry(QRect(540, 0, 71, 41));
        AudioCheckBox->setFont(font4);
        AudioCheckBox->setContextMenuPolicy(Qt::NoContextMenu);
        OutDirFrame = new QFrame(Widget);
        OutDirFrame->setObjectName(QString::fromUtf8("OutDirFrame"));
        OutDirFrame->setGeometry(QRect(10, 170, 621, 81));
        OutDirFrame->setContextMenuPolicy(Qt::NoContextMenu);
        OutDirFrame->setFrameShape(QFrame::StyledPanel);
        OutDirFrame->setFrameShadow(QFrame::Raised);
        ODLlabel = new QLabel(OutDirFrame);
        ODLlabel->setObjectName(QString::fromUtf8("ODLlabel"));
        ODLlabel->setGeometry(QRect(10, 10, 71, 61));
        ODLlabel->setFont(font1);
        ODLlabel->setContextMenuPolicy(Qt::NoContextMenu);
        ODEntry = new QTextEdit(OutDirFrame);
        ODEntry->setObjectName(QString::fromUtf8("ODEntry"));
        ODEntry->setGeometry(QRect(90, 20, 451, 41));
        QFont font5;
        font5.setPointSize(12);
        ODEntry->setFont(font5);
        ODEntry->setContextMenuPolicy(Qt::NoContextMenu);
        SButton = new QPushButton(OutDirFrame);
        SButton->setObjectName(QString::fromUtf8("SButton"));
        SButton->setGeometry(QRect(550, 20, 61, 41));
        SButton->setContextMenuPolicy(Qt::NoContextMenu);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "YoutubeDownloader", nullptr));
        URLlabel->setText(QApplication::translate("Widget", "URL", nullptr));
        DLButton->setText(QApplication::translate("Widget", "\343\203\200\343\202\246\343\203\263\343\203\255\343\203\274\343\203\211\351\226\213\345\247\213", nullptr));
        ExitCheckBox->setText(QApplication::translate("Widget", "\343\203\200\343\202\246\343\203\263\343\203\255\343\203\274\343\203\211\347\265\202\344\272\206\346\231\202\343\200\201\343\202\275\343\203\225\343\203\210\343\202\222\351\226\211\343\201\230\343\202\213", nullptr));
        VideoCheckBox->setText(QApplication::translate("Widget", "\345\213\225\347\224\273", nullptr));
        AudioCheckBox->setText(QApplication::translate("Widget", "\351\237\263\345\243\260", nullptr));
        ODLlabel->setText(QApplication::translate("Widget", "\345\207\272\345\212\233\345\205\210", nullptr));
        SButton->setText(QApplication::translate("Widget", "\351\201\270\346\212\236", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
