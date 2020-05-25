#include "widget.h"
#include "Header.hpp"

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
		const string json = string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
		
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
	connect(diaButton, SIGNAL(clicked()), this, SLOT(openDialog()));
	connect(Mp3Qua, SIGNAL(valueChanged(int)), this, SLOT(updateQuaNum(int)));
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
		QMessageBox qmb(this);
		qmb.setText("ソフトを終了しますか?");
		qmb.setWindowTitle("修了確認");
		qmb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		qmb.setButtonText(QMessageBox::Yes, "ダウンロードを待って終了");
		qmb.setButtonText(QMessageBox::No, "ダウンロードを待たずに終了");
		qmb.setButtonText(QMessageBox::Cancel, "キャンセル");
		int ret = qmb.exec();
		if (ret == QMessageBox::Cancel) {
			e->ignore();
		} else if (ret == QMessageBox::No) {
			exit(0);
		} else {
			timer->stop();
			LOGt->stop();
			if (thr_dl.joinable()) thr_dl.join();
			QWidget::closeEvent(e);
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
		}
		if (type == QEvent::HoverLeave) {
			ACCombo->setStyleSheet("");
		}
	}
	if (type == QEvent::WhatsThisClicked) {
		if (obj != ACCombo) {
			ACCombo->setStyleSheet("");
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
	if (isOpen && (e->pos().x() > UiSelect->width()) && (e->pos().y() > Frame->height())) closeDialogAnime();
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
	normalize(s);
	if (logs.empty()) {
		logs = s;
		return;
	}
	string ltmp = get_last_line(logs);
	if (ltmp.find(s) == string::npos) {
		logs += "\n" + s;
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
void Widget::addLOG(const char* cs) {
	addLOG(string(cs));
}
void Widget::addLOGasis(const string s) {
	logs += "\n" + s;
}
void Widget::addLOGasis(const wstring ws) {
	string s = wide_to_utf8(ws);
	logs += "\n" + s;
}
void Widget::addLOGasis(const QString qs) {
	string s = qs.toStdString();
	logs += "\n" + s;	
}
void Widget::addLOGasis(const char* cs) {
	string s = string(cs);
	logs += "\n" + s;
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

void Widget::openDialogAnime() {
	std::array<QFrame *, 6> frame = {
		URLFrame, VorA, ACCs, OutDirFrame, LOGFrame, DLFrame,
	};
	auto oda = new QPropertyAnimation(UiSelect, "geometry");
	oda->setDuration(1000);
	QRect rec = UiSelect->geometry();
	oda->setStartValue(rec);
	oda->setEndValue(QRect(rec.x() + 360, rec.y(), rec.width(), rec.height()));
	oda->setEasingCurve(QEasingCurve::InOutCirc);
	oda->start(QAbstractAnimation::DeleteWhenStopped);

	for (int i = 0; i < 6; ++i) {
		auto oeff = new QGraphicsOpacityEffect(frame[i]);
		frame[i]->setGraphicsEffect(oeff);
		auto fade = new QPropertyAnimation(frame[i], "opacity");
		fade->setTargetObject(oeff);
		fade->setDuration(1000);
		fade->setStartValue(1.00);
		fade->setEndValue(0.25);
		fade->setEasingCurve(QEasingCurve::InOutCirc);
		fade->start(QAbstractAnimation::DeleteWhenStopped);
	}
	
	isOpen = true;


}

void Widget::closeDialogAnime() {
	std::array<QFrame *, 6> frame = {
		URLFrame, VorA, ACCs, OutDirFrame, LOGFrame, DLFrame,
	};
	auto cda = new QPropertyAnimation(UiSelect, "geometry");
	cda->setDuration(1000);
	QRect rec = UiSelect->geometry();
	cda->setStartValue(rec);
	cda->setEndValue(QRect(rec.x() - 360, rec.y(), rec.width(), rec.height()));
	cda->setEasingCurve(QEasingCurve::InOutCirc);
	cda->start(QAbstractAnimation::DeleteWhenStopped);

	for (int i = 0; i < 6; ++i) {
		auto oeff = new QGraphicsOpacityEffect(frame[i]);
		frame[i]->setGraphicsEffect(oeff);
		auto fade = new QPropertyAnimation(frame[i], "opacity");
		fade->setTargetObject(oeff);
		fade->setDuration(1000);
		fade->setStartValue(0.25);
		fade->setEndValue(1.00);
		fade->setEasingCurve(QEasingCurve::InOutCirc);
		fade->start(QAbstractAnimation::DeleteWhenStopped);
	}
	isOpen = false;
}

void Widget::openDialog() {
	if (!isOpen) 	openDialogAnime();
	else			closeDialogAnime();
}

void Widget::updateQuaNum(int value) {
	qDebug() << QString("Call Update Quality Num");
	Mp3QuaNum->setText(QString::number(value) + QString("k"));
	Mp3QuaNum->update();
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

void dl	(Widget *const iui, const string iURL, const fs::path ioutDir, const string iffdir,
		const uint8_t iVAA, const int iAC, const unsigned int ibt, const bool ith, const bool ileave, const bool iex) noexcept{
	YDR proc(iui, iURL, ioutDir, iffdir, iVAA, iAC, ibt, ith, ileave, iex);
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
	if (!fs::exists(fs::path(od))) {
		LOG->setText("出力先ディレクトリが存在しません");
		return;
	}

	const uint8_t VAA = (VCB->checkState() == Qt::Checked ? YDR::Video : 0) | (ACB->checkState() == Qt::Checked ? YDR::Audio : 0);
	if (VAA == 0) {
		LOG->setText("動画か音声か、\n一つは選択してください(複数可)");
		return;
	}
	
	const int ac = ACCombo->currentIndex();
	const unsigned int bt = static_cast<unsigned int>(Mp3Qua->value());
	const bool th = ThCheckBox->checkState() == Qt::Checked;
	const bool lev = Leave->checkState() == Qt::Checked;
	const bool ex = ExitCheckBox->checkState() == Qt::Checked;
	DLButton->setText("ダウンロード中");
	timer->start(msec);
	LOGt->start(msec);

	thr_dl = std::thread(dl, this, url, fs::path(od), ffdir, VAA, ac, bt, th, lev, ex);
	return;
}