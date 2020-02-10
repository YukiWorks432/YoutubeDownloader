#include "widget.h"
#include "Header.hpp"

const string ACs[] = { "best"s, "wav"s, "flac"s, "aac"s, "mp3"s };
namespace my{
	inline bool check(const string &s) noexcept{
		return ((!s.empty()) && !(s == "#"));
	}

	inline std::string col (std::string c) {
		if (c[0] != '#') return ("#"s + c);
		return c;
	}

	inline void setStyle(Widget *ui) {
		using namespace std::chrono;
		
		std::ifstream fs("styles\\style.json", std::ios::in);
		if (fs.fail()) {
			ui->addLOG("style.jsonが見つかりませんでした"s);
			return;
		}
		const string json((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		
		fs.close();
		
		if (json.empty()) {
			ui->addLOG("style.jsonの中が空です"s);
			return;
		}

		picojson::value sv;
		const string err = picojson::parse(sv, json);
		
		if (!err.empty()) {
			ui->addLOG(err);
			return;
		}
		
		picojson::object& cfg_ = sv.get<picojson::object>();
		string mood = cfg_["mood"].get<string>();
		std::transform(mood.begin(), mood.end(), mood.begin(), ::tolower);
		if (mood == "default"s || mood == "deforuto"s || mood == "デフォルト"s || mood.empty()) {
			ui->LOG->setStyleSheet("background-color: #f0f0f0");
			ui->Frame->setStyleSheet("background-color: #eaeaea");
			ui->addLOG(mood.empty() ? "デフォルト"s : mood + "がセットされました"s);
			return;
		}
		picojson::object& cfg = cfg_[mood].get<picojson::object>();
		
		const auto wincolor 	= ui->wincolor 	= col(cfg["window"].get<string>());
		const auto fracolor 	= ui->fracolor 	= col(cfg["Frame"].get<string>());
		const auto textcolor 	= ui->textcolor = col(cfg["text"].get<string>());
		const auto textcolor_h 	= col(cfg["text_hover"].get<string>());
		if (!textcolor_h.empty()) {
			ui->dohover();
			ui->textcolor_h = textcolor_h;
			ui->addLOG("text_hover" + textcolor_h);
		}
		const auto textcolor_l	= col(cfg["text_link"].get<string>());
		if (!textcolor_l.empty()) {
			ui->textcolor_l = textcolor_l;
		}
		const auto selection 	= ui->selection = col(cfg["selection"].get<string>());
		const auto ST 			= ui->ST 		= col(cfg["selection-text"].get<string>());
		const auto border_w		= ui->border_w 	= cfg["boder_w"].get<string>();
		const auto border_c		= ui->border_c 	= col(cfg["boder_c"].get<string>());
		const auto &custom_ 	= cfg["custom"].get<picojson::array>();
		string custom;
		for (const auto &x : custom_) {
			custom += x.get<string>();
		}
		ui->custom = custom;
		
		string sheet = "QWidget {";
		if (check(wincolor)) 	sheet += "background-color: " 				+ wincolor 		+ ";";
		if (check(textcolor)) 	sheet += "color: " 							+ textcolor 	+ ";";
		if (check(selection)) 	sheet += "selection-background-color: " 	+ selection 	+ ";";
		if (check(ST)) 			sheet += "selection-color: " 				+ ST 			+ ";";
		sheet += "} ";
		if (check(border_w) && check(border_c)) sheet += "QPushButton, QComboBox {border: " + border_w + " solid " + border_c + ";}";
		if (!custom.empty()) sheet += custom;
		
		const int id = QFontDatabase::addApplicationFont("./fonts/migmix-1p-bold.ttf");
		const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
		const QFont ipagp(family);
		
		ui->setStyleSheet(sheet.c_str());

		if (check(fracolor)) ui->Frame->setStyleSheet(("background-color: " + fracolor).c_str());
		ui->addLOG(mood + " がセットされました");

	}
}

Widget::Widget(QWidget *parent) : QWidget(parent), mtx(new std::mutex), LOGt(new QTimer), timer(new QTimer) {
	using namespace std::chrono;
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint);
	setStyle(QStyleFactory::create("WindowsXP"));
	
	my::setStyle(this);

	addLOG("OpenMP最大スレッド数 : "s + to_string(omp_get_max_threads()));
	updateLOG();

    // スロットは普通に呼び出せる
	VCB->setChecked(true);
	ThCheckBox->setChecked(true);
	ODEntry->setText((fs::current_path() / fs::path("Download")).string().c_str());

	URLlabel->installEventFilter(this);
	URLEntry->installEventFilter(this);
	ODlabel->installEventFilter(this);
	ODEntry->installEventFilter(this);

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
	connect(URLEntry, SIGNAL(clickLink()), this, SLOT(goLink()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
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

void Widget::dohover() {
	this->hovers = true;
}

bool Widget::eventFilter(QObject *obj, QEvent *event) {
	const auto type = event->type();
	if 	(obj == URLEntry && !textcolor_l.empty() && hovering &&
		(URLEntry->toPlainText().toUtf8().toStdString().find("http://") == 0 || URLEntry->toPlainText().toUtf8().toStdString().find("https://") == 0)) {
		if (type == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->modifiers() & Qt::ControlModifier) {
			URLEntry->setStyleSheet("color:"_q + textcolor_l + ";text-decoration: underline solid "_q + textcolor_l);
			return true;
		}
		if (type == QEvent::KeyRelease) {
			URLEntry->setStyleSheet("color:"_q + textcolor);
		}
	}
	if (hovers) {
		if (type == QEvent::HoverEnter) {
			hovering = true;
			if (obj == URLEntry) {
				URLlabel->setStyleSheet("color:"_q + textcolor_h);
				URLEntry->setStyleSheet("color:"_q + textcolor_h);
				return true;
			}
			if (obj == ODEntry) {
				ODlabel->setStyleSheet("color:"_q + textcolor_h);
				ODEntry->setStyleSheet("color:"_q + textcolor_h);
				return true;
			}
		}
		if (type == QEvent::HoverLeave) {
			hovering = false;
			if (obj == URLEntry) {
				URLlabel->setStyleSheet("color:"_q + textcolor);
				URLEntry->setStyleSheet("color:"_q + textcolor);
				return true;
			}
			if (obj == ODEntry) {
				ODlabel->setStyleSheet("color:"_q + textcolor);
				ODEntry->setStyleSheet("color:"_q + textcolor);
				return true;
			}
		}
	}
	return false;
}

void Widget::goLink() {
	string url = URLEntry->toPlainText().toUtf8().toStdString();
	if (url.find("http://") != 0 && url.find("https://") != 0) return;
	string r;
	ShellExecuteA(nullptr, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	updateLOG();
}

void Widget::mousePressEvent(QMouseEvent *e) {
	if ((e->buttons() & Qt::LeftButton) && (e->pos().y() < Frame->height())) {
		isDrag = true;
		offset = e->pos();
	}
	QWidget::mousePressEvent(e);
}
void Widget::mouseReleaseEvent(QMouseEvent *e) {
	isDrag = false;
	QWidget::mouseReleaseEvent(e);
}
void Widget::mouseMoveEvent(QMouseEvent *e) {
	if (isDrag) {
		this->move(mapToParent(e->pos() - offset));
	} else QWidget::mouseMoveEvent(e);
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
	string s = wide_to_utf8(ws);
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
void Widget::addLOG(QString qs) {
	string s = qs.toStdString();
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
	const auto dir = QFileDialog::getExistingDirectory(this, tr("フォルダの選択")).toUtf8().constData();
	if (string(dir).empty()) return;
	ODEntry->setText(dir);
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