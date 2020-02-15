#include "Header.hpp"

void YDR::Download() {
	using namespace Popen;
	uint errs = 0;
	string VFs, AFs;
	string title;
	// タイトル、フォーマットの取得
	#pragma omp parallel sections
	{
		// VideosディレクトリとAudiosディレクトリの作成
		#pragma omp section
		{
			std::error_code ec;
			const auto p = outDir / fs::u8path("Videos");
			if (VAA & Video) fs::create_directory(p, ec);
			if (ec) {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("error! : "s + ec.value() + " : "s + ec.message());
				ui->addLOG("outDir : "s + p.string());
				++errs;
			}
			ec.clear();
			const auto p1 = outDir / fs::u8path("Audios");
			if (VAA & Audio) fs::create_directory(p1, ec);
			if (ec) {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("error! : "s + ec.value() + " : "s + ec.message());
				ui->addLOG("outDir : "s + p1.string());
				++errs;
			}
		}
		// タイトル取得
		#pragma omp section
		{
			string buf;
			const string cmd = "youtube-dl -e "s + URL;
			if (!Popen::Popen(cmd, buf)) {
				std::lock_guard lock(*(ui->mtx));
				ui->addLOG("タイトル取得失敗"s);
				++errs;
			}
			std::lock_guard lock(*(ui->mtx));
			title = QString::fromLocal8Bit(buf.c_str()).toStdString();
			normalizeTitle(title);
			ui->addLOG(title);
			title = utf8_to_sjis(title);
		}
		// フォーマット取得
		#pragma omp section
		{
			string buf;
			string cmd = "youtube-dl -F --youtube-skip-dash-manifest "s + URL;
			if (!Popen::Popen(cmd, buf)) {
				std::lock_guard lock(*(ui->mtx));
				ui->addLOG("フォーマット取得失敗"s);
				ui->addLOG(QString::fromLocal8Bit(buf.c_str()).toStdString());
				++errs;
			}
			for (auto &tmp : split(buf, '\n')) {
				std::smatch tempV, tempA;
				if (tmp.find("best") == string::npos) {
					if (tmp.find("video only") != string::npos) VFs = tmp.substr(0, 4);
					if (tmp.find("audio only") != string::npos) AFs = tmp.substr(0, 4);
					if (std::regex_search(VFs, tempV, std::regex("[0-9]{2,3}"))) VFs = tempV.str();
					if (std::regex_search(AFs, tempA, std::regex("[0-9]{2,3}"))) AFs = tempA.str();
				}
			}
			if (std::regex_match(VFs, std::regex("[0-9]{2,3}")) && std::regex_match(AFs, std::regex("[0-9]{2,3}"))) {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("フォーマット取得"s);
			} else {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("フォーマット取得失敗 at : Format no match"s);
				ui->dled();
				errs++;
			}
		}
	}

	if (errs) {
		std::lock_guard lock(*(ui->mtx));
		ui->addLOG("ダウンロード関数実行失敗");
		ui->dled();
		return;
	}
	
	string cmd;
	string cmd1;
	// コマンドの作成
	{
		if (VAA & Video) {
			cmd 			= "youtube-dl --ffmpeg-location \"" + ffdir + "\" -f " + VFs + " --no-check-certificate --no-part --add-metadata";
			if (th) cmd 	+= " --write-thumbnail";
			cmd				+= " --output \"" + outDir.string() + "\\Videos\\%(title)s.%(ext)s\" \"" + URL + "\"";
		}
		if (VAA & Audio) {
			string af = (AC == "alac"s) ? "" : "--audio-format "+ AC + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" ";
			cmd1 			= "youtube-dl --ffmpeg-location \"" + ffdir + "\" -f " + AFs + " --no-check-certificate --no-part -x " + af + "--add-metadata ";
			if (th) cmd1 	+= " --write-thumbnail";
			cmd1 			+= " --output \"" + outDir.string() + "\\Audios\\%(title)s.%(ext)s\" \"" + URL + "\"";
		}
	}

	SetLastError(0);

	// ダウンロード
	lock();
	ui->addLOG("ダウンロード開始");
	unlock();
	#pragma omp parallel shared(cmd, cmd1, ui)
	{
		#pragma omp sections
		{
			// 動画ダウンロード
			#pragma omp section
			{
				if (!cmd.empty()) {
					STARTUPINFOA si = {sizeof(STARTUPINFO)};
					PROCESS_INFORMATION pi = {};
					DWORD ExCo;
					BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(cmd.c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
					if (!res) {
						wstring r;
						GLEM(r);
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("動画ダウンロードプロセス起動失敗"s);
						ui->addLOG(r);
						errs++;
					} else {
						ExCo = WaitForSingleObject(pi.hProcess, INFINITE);
						if (ExCo == WAIT_FAILED) {
							wstring r;
							GLEM(r);
							std::lock_guard<std::mutex> lock(*(ui->mtx));
							ui->addLOG("動画ダウンロードプロセス実行失敗 : "s);
							ui->addLOG(r);
							errs++;
						} else {
							std::lock_guard<std::mutex> lock(*(ui->mtx));
							ui->addLOG("動画ダウンロードプロセス完了"s);
						}
					}
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
			}
			// 音声ダウンロード
			#pragma omp section
			{
				if (!cmd1.empty()) {
					STARTUPINFOA si = {sizeof(STARTUPINFO)};
					PROCESS_INFORMATION pi = {};
					DWORD ExCo;
					BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(cmd1.c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
					if (!res) {
						wstring r;
						GLEM(r);
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("音声ダウンロードプロセス起動失敗"s);
						ui->addLOG(r);
						errs++;
					} else {
						ExCo = WaitForSingleObject(pi.hProcess, INFINITE);
						if (ExCo == WAIT_FAILED) {
							wstring r;
							GLEM(r);
							std::lock_guard<std::mutex> lock(*(ui->mtx));
							ui->addLOG("音声ダウンロードプロセス実行失敗"s);
							ui->addLOG(r);
							errs++;
						} else {
							std::lock_guard<std::mutex> lock(*(ui->mtx));
							ui->addLOG("音声ダウンロードプロセス完了"s);
						}
					}
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
			}
		}
	}

	if (errs) {
		std::lock_guard lock(*(ui->mtx));
		ui->addLOG("ダウンロード関数実行失敗");
		ui->dled();
		return;
	}

	// エンコード
	if (AC == "alac"s) {
		lock();
		ui->addLOG("エンコード開始");
		unlock();
		vector<fs::path> wavs;
		cglob::glob(".opus", wavs, outDir / fs::u8path("Audios"));
		cglob::glob(".wav", wavs, outDir/ fs::u8path("Audios"));
		for (const auto &x : wavs) {
			string cmd2 = ffdir + " -i \"" + utf8_to_sjis(x.string()) + "\" -acodec alac \"" 
						+ utf8_to_sjis((outDir / fs::path("Audios") / x.stem()).string()) + ".m4a\"";
			string r;

			if (!Popen::Popen(cmd2, r, Popen::Flags::NoOutput)) {
				++errs;
			}
			lock();
			ui->addLOG(r);
			unlock();

			fs::remove(x);
		}
	}

	if (errs) {
		std::lock_guard lock(*(ui->mtx));
		ui->dled();
		return;
	}
	
	// 一時ファイルから外へ
	#pragma omp parallel sections
	{
		// Videos の中身を外へ
		#pragma omp section
		{
			vector<fs::path> ps;
			if (cglob::glob("\\Videos\\"s, ps, outDir)) {
				for (const auto& x : ps) {
					std::error_code ec;
					if (x.empty()) {
						std::lock_guard lock(*(ui->mtx));
						ui->addLOG("Empty"s);
						continue;
					}
					fs::path np = outDir / x.filename();
					fs::rename(x, np, ec);
					if (ec.value() != 0 && ec.value() != 17) {
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("Videos fs::rename("s + x.string() + ", "s + np.string() + ", ec) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
						ui->dled();
						errs++;
					}
				}
				std::error_code ec;
				fs::path dd = outDir / fs::path("Videos"s);
				fs::remove_all(dd, ec);
				if (ec) {
					std::lock_guard<std::mutex> lock(*(ui->mtx));
					ui->addLOG("Videos fs::remove_all("s + dd.string() + ", ec) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
					ui->dled();
					errs++;
				}
			}
		}
		// Audios の中身を外へ
		#pragma omp section
		{
			vector<fs::path> ps;
			if (cglob::glob("\\Audios\\"s, ps, outDir)) {
				for (const auto& x : ps) {
					std::error_code ec;
					if (x.empty()) {
						lock();
						ui->addLOG("Empty"s);
						unlock();
						continue;
					}
					fs::path np = outDir / x.filename();
					fs::rename(x, np, ec);
					if (ec.value() != 0 && ec.value() != 17) {
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("Audios fs::rename("s + x.string() + ", "s + np.string() + ", ce) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
						ui->dled();
						errs++;
					}
				}
				std::error_code ec;
				fs::path dd = outDir / fs::path("Audios"s);
				fs::remove_all(dd, ec);
				if (ec) {
					std::lock_guard<std::mutex> lock(*(ui->mtx));
					ui->addLOG("Audios fs::rename("s + dd.string() + ", ec) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
					ui->dled();
					errs++;
				}
			}
		}
	}

	if (errs) {
		std::lock_guard lock(*(ui->mtx));
		ui->addLOG("ダウンロード関数実行失敗");
		ui->dled();
		return;
	}

	ui->addLOG("ダウンロード終了");

	lock();
	ui->dled();
	unlock();

	if (ex) exit(0);
	return;
}

inline void opening(Widget *ui) {
	std::array<QFrame *, 6> frame = {
		ui->URLFrame, ui->VorA, ui->ACCs, ui->OutDirFrame, ui->LOGFrame, ui->DLFrame,
	};

	int ms = 1000;
	for (int i = 0; i < 6; ++i) {
		auto anime = new QPropertyAnimation(frame[i], "geometry");
		anime->setDuration(ms);
		QRect rec = frame[i]->geometry();
		anime->setStartValue(QRect(rec.x(), rec.y() + 100 * (i + 1), rec.width() , rec.height()));
		anime->setEndValue(rec);
		anime->setEasingCurve(QEasingCurve::OutQuint);

		auto oeff = new QGraphicsOpacityEffect(ui);
		frame[i]->setGraphicsEffect(oeff);
		auto anime1 =  new QPropertyAnimation(ui, "opacity");
		anime1->setTargetObject(oeff);
		anime1->setDuration(ms);
		anime1->setStartValue(0.0);
		anime1->setEndValue(1.0);
		anime1->setEasingCurve(QEasingCurve::OutQuint);
		
		anime->start(QAbstractAnimation::DeleteWhenStopped);
		anime1->start(QAbstractAnimation::DeleteWhenStopped);
	}
}

int main(int argc, char **argv) {
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, size_t(2560));

    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
    widget->show();
	opening(widget);

    // ループに入る
    return app.exec();
}