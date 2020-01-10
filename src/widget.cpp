#include "widget.h"
#include "Header.hpp"
using std::vector; using std::string; using std::to_string;
namespace fs = std::filesystem;

Widget::Widget(QWidget *parent) : QWidget(parent) {
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);

    // スロットは普通に呼び出せる
	VCB->setChecked(true);
	ODEntry->setText(QString(fs::current_path().string().c_str()));
	LOG->setText(QString(""));

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
}

void Widget::ClipPaste(){
	string clip = QApplication::clipboard()->text().toStdString();
	clip = clip.find("playlist") == string::npos ? clip.substr(0, clip.find("?list=")) : clip;
	URLEntry->setText(clip.c_str());
}

void Widget::SelectDir() {
	ODEntry->setText(QFileDialog::getExistingDirectory(this, tr("フォルダの選択")).toUtf8().constData());
}

void Widget::Download() {
	DLButton->setText(tr("ダウンロード中"));
	using Qt::CheckState::Checked;
	YDR proc(URLEntry->toPlainText().toUtf8().constData(), ODEntry->toPlainText().toUtf8().constData(), (VCB->checkState() == Checked) << 1 + (ACB->checkState() == Checked), ExitCheckBox->checkState() == Checked);
	proc.Download(this);
	DLButton->setText(tr("ダウンロード開始"));
}