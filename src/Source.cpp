#include "Header.hpp"

void YDR::Download() {
	using namespace Popen;
	uint errs = 0;
	string title;
	string code;
	string VFs, AFs;
	vector<string> VFv, AFv;
	// タイトル、フォーマットの取得
	#pragma omp parallel sections shared(errs)
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
			if (URL.find("playlist") == string::npos) {
				string buf;
				const string cmd = "youtube-dl -e "s + URL;
				if (!Popen::Popen(cmd, buf)) {
					std::lock_guard lock(*(ui->mtx));
					ui->addLOG("タイトル取得失敗"s);
					++errs;
				}
				title = QString::fromLocal8Bit(buf.c_str()).toStdString();
				normalizeTitle(title);
				std::lock_guard lock(*(ui->mtx));
				ui->addLOG(title);
			} else {
				string buf;
				const string cmd = "youtube-dl -e"s + URL;
				if (!Popen::Popen(cmd, buf)) {
					std::lock_guard lock(*(ui->mtx));
					ui->addLOG("タイトル取得失敗");
					++errs;
				}
				for (const auto &t : split(buf, '\n')) {
					lock();
					ui->addLOG(t);
					unlock();
				}
			}
		}
		// ID取得
		#pragma omp section
		{
			const string cmd = "youtube-dl --get-id " + URL;
			if (!Popen::Popen(cmd, code)) {
				std::lock_guard lock(*(ui->mtx));
				ui->addLOG("ID取得失敗");
				++errs;
			}
			normalizeTitle(code);
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
				ui->addLOG(buf);
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
			cmd 			= "youtube-dl --prefer-ffmpeg --ffmpeg-location \"" + ffdir + "\" -f " + VFs
							+ " --no-check-certificate --no-part --write-info-json";
			if (th) cmd 	+= " --write-thumbnail";
			cmd				+= " --output \"" + outDir.string() + "\\Videos\\" + code + ".%(ext)s\" \"" + URL + "\"";
		}
		if (VAA & Audio) {
			cmd1 			= "youtube-dl --prefer-ffmpeg --ffmpeg-location \"" + ffdir + "\" -f " + AFs
							+ " --no-check-certificate --no-part -x --write-info-json";
			if (th) cmd1 	+= " --write-thumbnail";
			cmd1 			+= " --output \"" + outDir.string() + "\\Audios\\" + code + ".%(ext)s\" \"" + URL + "\"";
		}
	}

	SetLastError(0);

	// ダウンロード
	lock();
	ui->addLOG("ダウンロード開始");
	unlock();
	#pragma omp parallel sections shared(cmd, cmd1, ui)
	{
		// 動画ダウンロード
		#pragma omp section
		{
			if (!cmd.empty()) {
				STARTUPINFOA si = {sizeof(STARTUPINFO)};
				PROCESS_INFORMATION pi = {};
				DWORD ExCo;
				BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(utf8_to_sjis(cmd).c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
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
				BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(utf8_to_sjis(cmd1).c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
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

	if (errs) {
		std::lock_guard lock(*(ui->mtx));
		ui->addLOG("ダウンロード関数実行失敗");
		ui->dled();
		return;
	}

	lock();
	ui->addLOG("エンコード開始");
	unlock();

	#pragma omp parallel sections shared(ui)
	{
		#pragma omp section
		{
			// 動画エンコード
			if (VAA & Video) {
				const auto cpath = outDir / fs::u8path("Videos");
				vector<fs::path> mov(1);
				cglob::glob(".mp4", mov, cpath);
				cglob::glob(".webm", mov, cpath);
				string newfile;
				for (const auto &x : mov) {
					if (x.empty()) continue;
					string jsonpass = (outDir / fs::path("Videos") / x.stem()).string() + ".info.json";
					newfile = (outDir / fs::path("Videos") / x.stem()).string() + "_enc.mp4";
					string cmd2 = ffdir + " -y -loglevel "s + (dmode == Popen::Debug ? "info"s : "quiet"s) + " -i \"" + x.string()
								+ "\" -strict -2 -c:a copy -c:v libx264 ";
					if (vmode == 1) cmd2 += "-vbr " + std::to_string(vbr);
					else 			cmd2 += "-crf " + std::to_string(crf);
					cmd2 += " \"" + newfile + "\"";
					string r;
					bool ret = !Popen::Popen(utf8_to_sjis(cmd2), r, dmode);
					if (r.find("Debug") == string::npos && !(ret || r.empty())) {
						lock();
						ui->addLOG("Command : ");
						ui->addLOG(cmd2);
						ui->addLOG(r);
						unlock();
						++errs;
						break;
					}

					// 元データの削除
					if (!leave || !!errs)	{
						std::error_code ec;
						fs::remove(x, ec);
						if (ec.value() != 0) {
							std::lock_guard lock(*(ui->mtx));
							ui->addLOGasis("Raw file remove error code : " + std::to_string(ec.value()) + "\n" + ec.message());
							++errs;
						}
						ec.clear();
						fs::remove(jsonpass, ec);
						if (ec.value() != 0) {
							std::lock_guard lock(*(ui->mtx));
							ui->addLOGasis("Raw file remove error code : " + std::to_string(ec.value()) + "\n" + ec.message());
							++errs;
						}
					}
				}
			}
		}
		#pragma omp section
		{
			// 音声エンコード
			if ((VAA & Audio) && (AC != ACs::best)) {
				const auto cpath = outDir / fs::u8path("Audios");
				vector<fs::path> wavs(1);
				cglob::glob(".opus", wavs, cpath);
				cglob::glob(".wav", wavs, cpath);
				cglob::glob(".webm", wavs, cpath);
				unordered_map<string, fs::path> thumbpass(1);
				if (th) {
					for (const auto &x : wavs) {
						vector<fs::path> tmp;
						if (cglob::glob(x.stem().generic_string() + ".jpg", tmp, cpath)) {
							thumbpass.emplace(x.stem().generic_string(), tmp[0]);
						}
					}
				}
				string acodec;
				string format;
				string exp;
				switch (AC) {
					case ACs::wav:
						acodec = "-c:a pcm_s16le";
						exp = "wav";
						break;
					case ACs::flac:
						acodec = "-c:a flac -compression_level 12";
						exp = "flac";
						break;
					case ACs::alac:
						acodec = "-c:a alac";
						exp = "m4a";
						break;
					case ACs::aac:
						acodec = "-b:a 256k -aac_coder twoloop -c:a aac -strict experimental";
						exp = "aac";
						break;
					case ACs::mp3:
						acodec = "-b:a " + to_string(bt * 2) + "k -c:a libmp3lame -strict unofficial -id3v2_version 3";
						exp = "mp3";
						break;
				}
				string newfile;
				for (const auto &x : wavs) {
					if (x.empty()) continue;
					newfile = (outDir / fs::path("Audios") / x.stem()).string() + "." + exp;

					string jsonpass = (outDir / fs::path("Audios") / x.stem()).string() + ".info.json";
					std::ifstream fs(utf8_to_sjis(jsonpass), std::ios::in);
					string artist, title, date, comment;
					if (!fs.fail()) {
						const string json = string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
						fs.close();
						if (json.empty()) continue;

						picojson::value sv;
						const string err = picojson::parse(sv, json);

						if (!err.empty()) {
							std::lock_guard lock(*(ui->mtx));
							ui->addLOG(err);
						}
						picojson::object &cfg_ = sv.get<picojson::object>();

						auto artist_ 	= cfg_["uploader"].get<string>();
						auto title_ 	= cfg_["title"].get<string>();
						auto date_ 		= cfg_["upload_date"].get<string>();
						auto comment_	= cfg_["description"].get<string>();

						int rb 	 = decode_unicode_escape_to_utf8(artist_, artist);
						rb 		+= decode_unicode_escape_to_utf8(title_, title);
						rb 		+= decode_unicode_escape_to_utf8(date_, date);
						rb 		+= decode_unicode_escape_to_utf8(comment_, comment);

						lock();
						ui->addLOGasis(
							artist + "\n" + title + "\n" + date + "\n" + comment
						);
						unlock();
						normalize(comment);
					} else {
						std::lock_guard lock(*(ui->mtx));
						ui->addLOG("メタデータ情報が見つかりませんでした");
						ui->addLOG(jsonpass);
					}

					string cmd2 = ffdir + " -y -loglevel "s + (dmode == Popen::Debug ? "info"s : "quiet"s) + " -vn -i \""s + x.string();
					if (th && !thumbpass[x.stem().generic_string()].empty()) {
						cmd2 	+= "\" -i \"" + thumbpass[x.stem().generic_string()].string();
					}
					cmd2 		+= "\" -disposition attached_pic -ar 44100 -ac 2 " + acodec
								+ " -metadata \"title\"=\"" + title + "\" -metadata \"artist\"=\"" + artist + "\" -metadata \"date\"=\"" 
								+ date + "\" -metadata \"comment\"=\"" + comment + "\" "
								+ "\"" + newfile + "\"";
					string r;
					bool ret = !Popen::Popen(utf8_to_sjis(cmd2), r, dmode);
					if (r.find("Debug") == string::npos && !(ret || r.empty())) {
						lock();
						ui->addLOG("Command : ");
						ui->addLOG(cmd2);
						ui->addLOG(r);
						unlock();
						++errs;
						break;
					}

					// 元データの削除
					if (!leave || !!errs)	{
						std::error_code ec;
						fs::remove(x, ec);
						if (ec.value() != 0) {
							std::lock_guard lock(*(ui->mtx));
							ui->addLOGasis("Raw file remove error code : " + std::to_string(ec.value()) + "\n" + ec.message());
							++errs;
						}
						ec.clear();
						fs::remove(jsonpass, ec);
						if (ec.value() != 0) {
							std::lock_guard lock(*(ui->mtx));
							ui->addLOGasis("Raw file remove error code : " + std::to_string(ec.value()) + "\n" + ec.message());
							++errs;
						}
					}
				}
			}
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
					fs::path np = outDir / fs::u8path(title.c_str() + x.extension().string());
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
					fs::path np = outDir / fs::u8path(title.c_str() + x.extension().string());
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
				if (ec.value() != 0) {
					std::lock_guard<std::mutex> lock(*(ui->mtx));
					ui->addLOG("Audios fs::remove_all("s + dd.string() + ", ec) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
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

	lock();
	ui->addLOG("ダウンロード終了");
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

		auto oeff = new QGraphicsOpacityEffect(frame[i]);
		frame[i]->setGraphicsEffect(oeff);
		auto anime1 =  new QPropertyAnimation(oeff, "opacity");
		anime1->setTargetObject(oeff);
		anime1->setDuration(ms);
		anime1->setStartValue(0.0);
		anime1->setEndValue(1.0);
		anime1->setEasingCurve(QEasingCurve::OutQuint);
		
		anime->start(QAbstractAnimation::DeleteWhenStopped);
		anime1->start(QAbstractAnimation::DeleteWhenStopped);
	}
}

unsigned int fontnum = 0;

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXW *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType , LPARAM lParam) {
	auto p = reinterpret_cast<wstring *>(lParam);
    if (fontnum >= 1024) return 1;
	p[fontnum] = wstring(lpelfe->elfFullName);
	++fontnum;
	return 1;
}

class Fonts {
	public:
		std::array<wstring, 1024> FontNames;
		bool searchFont(const wstring &name) {
            for (unsigned int i = 0; i < fontnum; ++i) {
                const wstring font = FontNames[i];
				if(font.find(name) != wstring::npos) {  
					return true;
				}
			}
			return false;
		}
		Fonts() {
            FontNames.fill(L"NaN\0");
			LOGFONTW lf;
			HDC hdc = GetDC(0);
			lf.lfFaceName[0] = 0;
			lf.lfPitchAndFamily = DEFAULT_PITCH;
			lf.lfCharSet = SHIFTJIS_CHARSET; 
			EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)EnumFontFamExProc, reinterpret_cast<LPARAM>(this->FontNames.data()), 0);
			ReleaseDC(0, hdc);
		}
};

inline void CreateConsole(void) {
    FILE* fp;
    AllocConsole();
    freopen_s(&fp, "CONOUT$", "w", stdout); /* 標準出力(stdout)を新しいコンソールに向ける */
    freopen_s(&fp, "CONOUT$", "w", stderr); /* 標準エラー出力(stderr)を新しいコンソールに向ける */
}

int main(int argc, char **argv) {
	setvbuf(stdout, nullptr, _IOFBF, size_t(1024 * 1024));
	SetConsoleOutputCP(CP_UTF8);

	try {
		// mainの最初でQApplicationを作っておく
		QApplication app(argc, argv);
		
		Fonts font; QFont uifont;
		if (font.searchFont(L"MigMix 1P Bold")) {
			uifont = QFont("MigMix 1P Bold");
		} else {
			uifont = QFont("Meiryo UI Bold");
		}
		app.setFont(uifont);

		// 自作のWidgetクラスを生成、表示
		Widget *widget = new Widget;
		widget->show();
		opening(widget);

		// ループに入る
		return app.exec();
	} catch (std::runtime_error ex) {
		MessageBox(NULL, ex.what(), "エラーが発生しました", MB_OK | MB_ICONWARNING);
		return 0;
	} catch (std::exception ex) {
		MessageBox(NULL, ex.what(), "エラーが発生しました", MB_OK | MB_ICONWARNING);
		return 0;
	} catch (...) {
		MessageBox(NULL, "原因不明の起動エラーです", "エラーが発生しました", MB_OK | MB_ICONWARNING);
		return 0;
	}
}