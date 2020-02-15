#include "widget.h"
#include "Header.hpp"

const string ACs[] = { "best"s, "wav"s, "flac"s, "alac"s, "aac"s, "mp3"s };
namespace my{
	inline std::string col (std::string c) noexcept{
		if (c[0] != '#') return ("#"s + c);
		return c;
	}


	QDebug operator<<(QDebug deb, const std::string &s) {
		return (deb << s.c_str());
	}
	QDebug operator<<(QDebug deb, const pallet &p) {
		deb 	<< "wincolor\t: " 		<< p.wincolor		<< '\n'
				<< "fracolor\t: " 		<< p.fracolor		<< '\n'
				<< "textcolor\t: " 		<< p.textcolor		<< '\n'
				<< "textcolor_h\t: " 	<< p.textcolor_h	<< '\n'
				<< "textcolor_l\t: "	<< p.textcolor_l	<< '\n'
				<< "selection\t: " 		<< p.selection		<< '\n'
				<< "selection-text\t: "	<< p.ST				<< '\n'
				<< "border_w\t: " 		<< p.border_w		<< '\n'
				<< "berder_c\t: " 		<< p.border_c		<< '\n'
				<< "custom\t: " 		<< p.custom 		<< '\n';
	return deb;
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

		const auto &moods = cfg_["moods"].get<picojson::array>();

		for (auto &x : moods) {
			string name = x.get<string>();
			
			if (name == "デフォルト" || name == "deforuto" || name == "default") {
				ui->moods.push_back(name);
				continue;
			}
			ui->moods.push_back(name);

			auto &cc = cfg_[name].get<picojson::object>();
			pallet *pal = new pallet();
			pal->wincolor 	 = col(cc["window"].get<string>());
			pal->fracolor 	 = col(cc["frame"].get<string>());
			pal->textcolor 	 = col(cc["text"].get<string>());
			pal->textcolor_h = col(cc["text-hover"].get<string>());
			pal->textcolor_l = col(cc["text-link"].get<string>());
			pal->selection   = col(cc["selection"].get<string>());
			pal->ST 		 = col(cc["selection-text"].get<string>());
			pal->border_w 	 = cc["border-w"].get<string>();
			pal->border_c 	 = col(cc["border-c"].get<string>());
			pal->qbombobox_hover = cc["qcombobox-hover"].get<string>();
			const auto &c_ 	 = cc["custom"].get<picojson::array>();
			for (const auto &i : c_) {
				pal->custom += i.get<string>();
			}

			ui->pallets.insert({name, pal});
		}

		const int id = QFontDatabase::addApplicationFont("./fonts/migmix-1p-bold.ttf");
		const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
		const QFont ipagp(family);

		string mood = ui->setmood(cfg_["mood"].get<string>());
		std::transform(mood.begin(), mood.end(), mood.begin(), ::tolower);
		if (mood == "default"s || mood == "deforuto"s || mood == "デフォルト"s || mood.empty()) {
			ui->LOG->setStyleSheet("background-color: #f0f0f0");
			ui->Frame->setStyleSheet("background-color: #eaeaea");
			ui->diaButton->setStyleSheet("QPushButton#diaButton{border-image:url(./styles/icons/dialog.png);}"
										"QPushButton:hover#diaButton{border-image:url(./styles/icons/dialog_h.png);}");
			ui->addLOG(mood.empty() ? "デフォルト"s : mood + "がセットされました"s);
			return;
		}

		const auto pal = *(ui->pallets.at(mood));
		ui->setNowPallet(pal);

		string sheet = "QWidget {";
		if (!pal.wincolor.empty())		sheet += "background-color: " 			+ pal.wincolor 	+ ";";
		if (!pal.textcolor.empty()) 	sheet += "color: " 						+ pal.textcolor 	+ ";";
		if (!pal.selection.empty()) 	sheet += "selection-background-color: " + pal.selection 	+ ";";
		if (!pal.ST.empty()) 			sheet += "selection-color: " 			+ pal.ST 			+ ";";
		sheet += "} ";
		if (!pal.border_w.empty() && !pal.border_c.empty())
			sheet += "QPushButton, QComboBox {border: " + pal.border_w + " solid " + pal.border_c + ";}";
		sheet += pal.custom;
		
		ui->setStyleSheet(sheet.c_str());
		if (!pal.fracolor.empty()) ui->Frame->setStyleSheet(("background-color: " + pal.fracolor).c_str());
	
		ui->addLOG(ui->getmood() + " がセットされました");
	}
}

Widget::Widget(QWidget *parent) : QWidget(parent), mtx(new std::mutex), LOGt(new QTimer), timer(new QTimer) {
	using namespace std::chrono;
	auto t = high_resolution_clock::now();
    // setupUiは、UICが生成したクラスに実装されている関数
    // これを呼ぶことでウィジェット内の要素の確保や初期値の設定などをDesignerで設定した値通りの状態にするための処理が行われる
    setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint);
	setStyle(QStyleFactory::create("WindowsXP"));
	
	closeButton->setStyleSheet("QPushButton#closeButton{border-image:url(./styles/icons/close.png);}QPushButton:hover#closeButton{border-image:url(./styles/icons/close1.png);}");
	my::setStyle(this);
	
	vector<fs::path> p;
	if (!cglob::glob("\\bin\\ffmpeg.exe", p)) {
		LOG->setText("ffmpegにアクセスできませんでした。アクセス権限を確認してください。");
		return;
	}
	ffdir = utf8_to_sjis(p[0].generic_string());

	addLOG("FFmpeg : "s + p[0].generic_string());
	addLOG("OpenMP最大スレッド数 : "s + to_string(omp_get_max_threads()));
	updateLOG();

    // スロットは普通に呼び出せる
	VCB->setChecked(true);
	ThCheckBox->setChecked(true);
	ODEntry->setText((fs::current_path() / fs::path("Download")).string().c_str());

	URLEntry->installEventFilter(this);
	ODEntry->installEventFilter(this);
	ACCombo->installEventFilter(this);

    // シグナルとスロットを接続
	connect(URLEntry, SIGNAL(rightClicked()), this, SLOT(ClipPaste()));
	connect(URLEntry, SIGNAL(clickLink()), this, SLOT(goLink()));
    connect(SButton, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(DLButton, SIGNAL(clicked()), this, SLOT(Download()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(LOGt, SIGNAL(timeout()), this, SLOT(updateLOG()));
	connect(timer, SIGNAL(timeout()), this, SLOT(DownloadEnd()));
	auto d = to_string(duration_cast<duration<double>>(high_resolution_clock::now() - t).count());
	addLOG("Start Up Time : "s + d + "s"s);
	updateLOG();
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

bool Widget::eventFilter(QObject *obj, QEvent *event) {
	const auto type = event->type();
	if 	(obj == URLEntry && !nowPallet.textcolor_l.empty() && hovering &&
		(URLEntry->toPlainText().toUtf8().toStdString().find("http://") == 0 || URLEntry->toPlainText().toUtf8().toStdString().find("https://") == 0)) {
		if (type == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->modifiers() & Qt::ControlModifier) {
			URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor_l + ";text-decoration: underline solid "_q + nowPallet.textcolor_l);
			return true;
		}
		if (type == QEvent::KeyRelease) {
			if (hovering) 	URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor_h);
			else 			URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor);
		}
	}
	if (!nowPallet.textcolor_h.empty()) {
		if (type == QEvent::HoverEnter) {
			hovering = true;
			if (obj == URLEntry) {
				if (static_cast<QKeyEvent*>(event)->modifiers() & Qt::ControlModifier) {
					URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor_l);
					return true;
				}
				URLlabel->setStyleSheet("color:"_q + nowPallet.textcolor_h);
				URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor_h);
				return true;
			}
			if (obj == ODEntry) {
				ODlabel->setStyleSheet("color:"_q + nowPallet.textcolor_h);
				ODEntry->setStyleSheet("color:"_q + nowPallet.textcolor_h);
				return true;
			}
		}
		if (type == QEvent::HoverLeave) {
			hovering = false;
			if (obj == URLEntry) {
				URLlabel->setStyleSheet("color:"_q + nowPallet.textcolor);
				URLEntry->setStyleSheet("color:"_q + nowPallet.textcolor);
				return true;
			}
			if (obj == ODEntry) {
				ODlabel->setStyleSheet("color:"_q + nowPallet.textcolor);
				ODEntry->setStyleSheet("color:"_q + nowPallet.textcolor);
				return true;
			}
		}
	}
	if (obj == ACCombo && !nowPallet.qbombobox_hover.empty()) {
		if (type == QEvent::HoverEnter)	{
			ACCombo->setStyleSheet(nowPallet.qbombobox_hover.c_str());
			updateLOG();
		}
		if (type == QEvent::HoverLeave) {
			ACCombo->setStyleSheet("");
			updateLOG();
		}
	}
	return false;
}

void Widget::goLink() {
	string url = URLEntry->toPlainText().toUtf8().toStdString();
	if (url.find("http://") != 0 && url.find("https://") != 0) return;
	ShellExecuteA((HWND)this->winId(), "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
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
		logs = normalize(s);
		return;
	}
	string ltmp = get_last_line(logs);
	if (ltmp.find(s) == string::npos) {
		logs += "\n"s + normalize(s);
	}
}
void Widget::addLOG(wstring ws) {
	string s = wide_to_utf8(ws);
	addLOG(s);
}
void Widget::addLOG(QString qs) {
	string s = qs.toStdString();
	addLOG(s);
}
void Widget::addLOG(const char* s) {
	addLOG(string(s));
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
	ODEntry->setText(QString::fromUtf8(dir));
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

void dl(Widget *const iui, const string iURL, const fs::path ioutDir, const string iffdir, const uint8_t iVAA, const string iAC, const bool ith, const bool iex) noexcept{
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

	string od = ODEntry->toPlainText().toStdString();
	if (od.empty()) {
		LOG->setText("空白を埋めてください");
		return;
	}
	if (od.length() <= 2) {
		LOG->setText("出力先ディレクトリが不正です");
		return;
	}

	const uint8_t VAA = (VCB->checkState() == Qt::Checked ? YDR::Video : 0) | (ACB->checkState() == Qt::Checked ? YDR::Audio : 0);
	if (VAA == 0) {
		LOG->setText("動画か音声か、\n一つは選択してください(複数可)");
		return;
	}
	
	const string ac = ACs[ACCombo->currentIndex()];

	const bool th = ThCheckBox->checkState() == Qt::Checked;
	
	const bool ex = ExitCheckBox->checkState() == Qt::Checked;
	DLButton->setText("ダウンロード中");
	timer->start(msec);
	LOGt->start(msec);

	thr_dl = std::thread(dl, this, url, fs::path(od), ffdir, VAA, ac, th, ex);
	return;
}