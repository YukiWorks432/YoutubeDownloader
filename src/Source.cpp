#include "Header.hpp"

std::mutex mtx;
QString logs;

inline void updateLOG(Widget *ui) {
	QString text;
	while (true) {
		mtx.lock();
		auto c = ui->LOG->text().toStdString().find(logs.toStdString().c_str());
		if (c == string::npos) {
			text = ui->LOG->text() + logs;
			ui->LOG->setText(text);
		}
		mtx.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

inline void normalize(std::string &targetStr) {
    const char CR = '\r';
    const char LF = '\n';
	const char SS = '/';
	const char BS = '\\';
    std::string destStr;
    for (const auto c : targetStr) {
        if (c != CR && c != LF) {
			if (c == SS || c == BS) destStr += '-';
			else 					destStr += c;
		}
    }
    targetStr = std::move(destStr);
}

bool cwrap::CreateProcessW(	LPWSTR lpCommandLine,
							LPSTARTUPINFOW lpStartupInfo,
							LPPROCESS_INFORMATION lpProcessInformation) {
	return CreateProcessW(NULL, lpCommandLine, NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, lpStartupInfo, lpProcessInformation);
}

LPWSTR cwrap::StdStr2Wchar(string &temp) {
	int n;
	n = MultiByteToWideChar( CP_ACP, 0, temp.c_str(), temp.size(), NULL, 0 );
	LPWSTR p = new WCHAR[ n + 1 ];
	n = MultiByteToWideChar( CP_ACP, 0, temp.c_str(), temp.size(), p, n );
	*( p + n ) = '\0';
	return p;
}

string cglob::glob(const char *sp) {
	for (const auto &x : fs::recursive_directory_iterator(fs::current_path())) {
		string s = x.path().string();
		if (s.find(sp) != string::npos) {
			return s;
		}
	}
	return cglob::nfound;
}

void YDR::Download(const Widget * const widget) const {
	if (VAA == 0) {
		widget->LOG->setText(Widget::tr("動画か音声か、\n一つは選択してください(複数可)"));
		return;
	}
	if (URL.substr(0, 8).find("https://") == string::npos) {
		widget->LOG->setText(Widget::tr("URLが https:// から始まっていません"));
		return;
	}
	if (outDir.length() <= 2) {
		widget->LOG->setText(Widget::tr("出力先ディレクトリが不正です"));
		return;
	}

	string cmd = "youtube-dl -e "s + URL;
	auto fp = popen(cmd.c_str(), "r");
	if (fp == NULL) return;
	auto buf = new char[128];
	while (!feof(fp)) {
		fgets(buf, 128, fp);
	}
	
	auto title = QString::fromLocal8Bit(buf);
	string titles = buf;
	normalize(titles);
	mtx.lock();
	logs = title;
	mtx.unlock();

	auto buf1 = new char[128];
	cmd = "youtube-dl -F --youtube-skip-dash-manifest "s + URL;
	auto fp1 = popen(cmd.c_str(), "r");
	string VFs, AFs;
	if (fp1 == NULL) return;
	{
		string tmp;
		while (!feof(fp1)) {
			fgets(buf1, 128, fp1);
			std::smatch tempV, tempA;
			if ((tmp = string(buf1)).find("video only") != string::npos) VFs = buf1;
			if (tmp.find("audio only") != string::npos) AFs = buf1;
			VFs = VFs.substr(0, 3);
			AFs = AFs.substr(0, 3);
			if (std::regex_search(VFs, tempV, std::regex("[0-9]{2,3}"))) VFs = tempV.str();
			if (std::regex_search(AFs, tempA, std::regex("[0-9]{2,3}"))) AFs = tempA.str();
		}
	}

	mtx.lock();
	if (std::regex_match(VFs, std::regex("[0-9]{2,3}")) && std::regex_match(AFs, std::regex("[0-9]{2,3}"))) {
		logs = QString("フォーマット取得成功 ");
	} else {
		logs = QString("フォーマット取得失敗");
		delete[] buf, buf1;
		widget->DLButton->setText("ダウンロード開始");
		mtx.unlock();
		return;
	}
	mtx.unlock();

	string ffdir = cglob::glob("\\bin\\ffmpeg.exe");

	string cmd1 = "exit"s;
	cmd = "youtube-dl --ffmpeg-location \""s + ffdir + "\" -q -f "s;
	if (VAA & Video) { 
		if (VAA & Audio) {
			cmd1 = cmd;
			cmd1 += AFs + " -x --audio-format "s + string(AC) + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" "s;
			cmd1 += " --output \""s + outDir + "\\"s + titles + ".%(ext)s\" --no-check-certificate --no-part \""s + URL + "\""s;
		}
		cmd += VFs;
	}
	if (!(VAA & Video)) {
		cmd += AFs + " -x --audio-format "s + string(AC) + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" "s;
	}
	cmd += " --output \""s + outDir + "\\"s + titles + ".%(ext)s\" --no-check-certificate --no-part \""s + URL + "\""s;

	#pragma omp parallel sections
	{
		#pragma omp section
		{
			system(cmd.c_str());
		}
		#pragma omp section
		{
			system(cmd1.c_str());
		}
	}

	delete[] buf, buf1;
	widget->DLButton->setText("ダウンロード開始");
	if (ex) exit(0); //*/
}

int main(int argc, char **argv) {
    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
    widget->show();

	widget->thr_log = std::thread([&](){ updateLOG(widget); });

    // ループに入る
    return app.exec();
}