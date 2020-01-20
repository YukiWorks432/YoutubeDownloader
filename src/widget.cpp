#include "widget.h"
#include "Header.hpp"

Widget::Widget(QWidget *parent) : QWidget(parent) {
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);

    // スロットは普通に呼び出せる
	VCB->setChecked(true);
	ODEntry->setText(QString(fs::current_path().string().c_str()));

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
}

Widget::~Widget() {
	thr_dl.join();
	thr_log.join();
}

void Widget::ClipPaste() {
	string clip = QApplication::clipboard()->text().toStdString();
	clip = clip.find("playlist") == string::npos ? clip.substr(0, clip.find("list=") - 1) : clip;
	URLEntry->setText(clip.c_str());
}

void Widget::SelectDir() {
	ODEntry->setText(QFileDialog::getExistingDirectory(this, tr("フォルダの選択")).toUtf8().constData());
}

const char *ACs[] = { "best", "wav", "flac", "aac", "mp3" };

void Widget::Download() {
	if (tr("ダウンロード中") == DLButton->text()) return;
	if (thr_dl.joinable()) thr_dl.join();
	DLButton->setText(tr("ダウンロード中"));
	const int ACI = ACCombo->currentIndex();
	const char *&ac = ACs[ACI];
	thr_dl = std::thread([&]() {
		YDR proc(
			URLEntry->toPlainText().toUtf8().toStdString(),
			ODEntry->toPlainText().toUtf8().toStdString(),
			(VCB->checkState() == Qt::Checked ? YDR::Video : 0) | (ACB->checkState() == Qt::Checked ? YDR::Audio : 0),
			ac,
			ExitCheckBox->checkState() == Qt::Checked);
		proc.Download(this);	
	});
}