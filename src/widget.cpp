#include "widget.h"
using std::vector; using std::string; using std::to_string;
namespace fs = std::filesystem;

Widget::Widget(QWidget *parent) : QWidget(parent) {
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);

    // スロットは普通に呼び出せる
	VideoCheckBox->setChecked(true);
	ODEntry->setText(QString(fs::current_path().string().c_str()));

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
}

void Widget::ClipPaste(){
	URLEntry->setText(QApplication::clipboard()->text());
}

void Widget::SelectDir() {
	ODEntry->setText(QFileDialog::getExistingDirectory(this, tr("フォルダの選択")).toUtf8().constData());
}

void Widget::Download() {
	if (ExitCheckBox->checkState()) {
		std::exit(0);
	}
}