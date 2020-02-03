#include "widget.h"
#include "Header.hpp"

const string ACs[] = { "best"s, "wav"s, "flac"s, "aac"s, "mp3"s };

Widget::Widget(QWidget *parent) : QWidget(parent) {
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);
	setStyle(QStyleFactory::create("WindowsXP"));
	const int id = QFontDatabase::addApplicationFont("./fonts/migmix-1p-bold.ttf");
	const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
	const QFont ipagp(family);

    // スロットは普通に呼び出せる
	VCB->setChecked(true);
	ThCheckBox->setChecked(true);
	ODEntry->setText(QString((fs::current_path().string() + "\\Download"s).c_str()));

	addLOG("OpenMP最大スレッド数 : "s + to_string(omp_get_max_threads()));

	mtx = new std::mutex;
	LOGt = new QTimer;
	timer = new QTimer;
	ENDDL = false;

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
	connect(LOGt, SIGNAL(timeout()), this, SLOT(updateLOG()));
	connect(timer, SIGNAL(timeout()), this, SLOT(DownloadEnd()));
}

Widget::~Widget() {
	thr_dl.join();
}

void Widget::closeEvent(QCloseEvent *e) {
	if (DLButton->text() != tr("ダウンロード開始")) {
		const auto ret = QMessageBox::question(this, tr("終了確認"), tr("終了しますか？\n(ダウンロードの終了を待ちます)"));
		if (ret == QMessageBox::No) {
			e->ignore();
		} else {
			timer->stop();
			LOGt->stop();
			if (thr_dl.joinable()) thr_dl.join();
		}
	}
}

void Widget::addLOG(string s) {
	if (logs.empty()) {
		logs += normalize(s);
		return;
	}
	string ltmp;
	logs.copy(ltmp.data(), logs.rfind('\n') != string::npos ? logs.rfind('\n') : 0, logs.length());
	if (ltmp.find(s) == string::npos) {
		logs += "\n"s + normalize(s);
	}
}
void Widget::addLOG(wstring ws) {
	string s = WStringToString(ws);
	if (logs.empty()) {
		logs += normalize(s);
		return;
	}
	string ltmp;
	logs.copy(ltmp.data(), logs.rfind('\n') != string::npos ? logs.rfind('\n') : 0, logs.length());
	if (ltmp.find(s) == string::npos) {
		logs += "\n"s + normalize(s);
	}
}
void Widget::addLOG(const char* s) {
	string text(s);
	if (logs.empty()) {
		logs += normalize(text);
		return;
	}
	string ltmp;
	logs.copy(ltmp.data(), logs.rfind('\n') != string::npos ? logs.rfind('\n') : 0, logs.length());
	if (ltmp.find(text) == string::npos) {
		logs += "\n"s + normalize(text);
	}
}

void Widget::updateLOG() {
	LOG->setText(QString::fromStdString(logs));
	QTextCursor c = LOG->textCursor();
	c.movePosition(QTextCursor::End);
	LOG->setTextCursor(c);
	LOG->update();
}

void Widget::ClipPaste() {
	string clip = QApplication::clipboard()->text().toStdString();
	delList(clip);
	URLEntry->setText(clip.c_str());
}

void Widget::SelectDir() {
	ODEntry->setText(QFileDialog::getExistingDirectory(this, tr("フォルダの選択")).toUtf8().constData());
}

bool Widget::dled() {
	ENDDL = true;
	return ENDDL.load();
}

void Widget::DownloadEnd() {
	if (!ENDDL.load()) return;
	DLButton->setText("ダウンロード開始");
	LOGt->stop();
	timer->stop();
	updateLOG();
}

void dl(Widget *const iui, const string iURL, const string ioutDir, const string iffdir, const uint8_t iVAA, const char *const iAC, const bool ith, const bool iex) noexcept{
	YDR proc(iui, iURL, ioutDir, iffdir, iVAA, iAC, ith, iex);
	proc.Download();
	return;
}

void Widget::Download() {
	if (QString("ダウンロード中") == DLButton->text()) return;
	if (thr_dl.joinable()) thr_dl.join();

	ENDDL = false;

	string url = URLEntry->toPlainText().toUtf8().toStdString();
	if (url.empty()) {
		LOG->setText("空白を埋めてください");
		return;
	}
	if (url.find("https://") == string::npos) {
		LOG->setText("URLが https:// から始まっていません");
		return;
	}
	delList(url);
	URLEntry->setText(url.c_str());

	const string od = ODEntry->toPlainText().toUtf8().toStdString();
	if (od.empty()) {
		LOG->setText("空白を埋めてください");
		return;
	}
	if (od.length() <= 2) {
		LOG->setText("出力先ディレクトリが不正です");
		return;
	}

	vector<fs::path> p;
	if (!cglob::glob("\\bin\\ffmpeg.exe", p)) {
		LOG->setText("ffmpegにアクセスできませんでした。アクセス権限を確認してください。");
		return;
	}
	const string ffdir = p[0].string();

	const uint8_t VAA = (VCB->checkState() == Qt::Checked ? YDR::Video : 0) | (ACB->checkState() == Qt::Checked ? YDR::Audio : 0);
	if (VAA == 0) {
		LOG->setText("動画か音声か、\n一つは選択してください(複数可)");
		return;
	}
	
	const char *ac = ACs[ACCombo->currentIndex()].c_str();

	const bool th = ThCheckBox->checkState() == Qt::Checked;
	
	const bool ex = ExitCheckBox->checkState() == Qt::Checked;
	DLButton->setText("ダウンロード中");
	timer->start(msec);
	LOGt->start(msec);

	thr_dl = std::thread(dl, this, url, od, ffdir, VAA, ac, th, ex);
	return;
}