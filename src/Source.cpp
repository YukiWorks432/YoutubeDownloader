#include "Header.hpp"

inline void GLEM(wstring &result) noexcept{
	auto Er = GetLastError();
	cout << Er << endl;
	const ulong s = 2048;
	std::wstring buf(s, '\0');
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, Er,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR)buf.data(), static_cast<DWORD>(s), NULL
	);
	result = buf;
	return;
}
inline void GLEM(string &result) noexcept{
	auto Er = GetLastError();
	cout << Er << endl;
	const ulong s = 2048;
	std::wstring buf(s, '\0');
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, Er,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR)buf.data(), static_cast<DWORD>(s), NULL
	);
	result = WStringToString(buf);
	return;
}

inline bool Popen(const string command, string &result) noexcept{
	SetLastError(0);
	SECURITY_ATTRIBUTES sa;
	HANDLE read, write;
	string r;

	std::shared_ptr<char> cmd(new char[command.length() + 1], std::default_delete<char[]>());
	std::copy(command.begin(), command.end(), cmd.get());
	cmd.get()[command.length()] = '\0';

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&read, &write, &sa, 0)) {
		string r;
		GLEM(r);
		result = "Failed CreatePipe() : "s + r;
		return false;
	}

	STARTUPINFO si = {};
	si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = write;
    si.hStdError = write;

	if (si.hStdOutput == INVALID_HANDLE_VALUE || si.hStdError == INVALID_HANDLE_VALUE) {
		result = "Failed Get Handle"s;
		return false;
	}

	PROCESS_INFORMATION pi = {};
	if (!CreateProcessA(NULL, cmd.get(), NULL, NULL, TRUE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
		string r;
		GLEM(r);
		result = "Failed CreateProcessA()"s + r;
		return false;
	}

	HANDLE child = pi.hProcess;
	if (!CloseHandle(pi.hThread)) {
		string r;
		GLEM(r);
		result = "Failed CloseHandle(pi.hThread) : "s + r;
		return false;
	}
	if (WaitForSingleObject(child, INFINITE) != WAIT_OBJECT_0) {
		string r;
		GLEM(r);
		result = "Failed WaitForSingleObject : "s + r;
		return false;
	}
	if (!CloseHandle(write)) {
		string r;
		GLEM(r);
		result = "Failed CloseHandle : "s + r;
		return false;
	}

	std::array<char, 1280> buf;
	DWORD rlen = 0;
	while (ReadFile(read, buf.data(), buf.size(), &rlen, NULL)) {
		std::copy(buf.begin(), buf.begin() + rlen, std::back_inserter(result));
	}

	if (!CloseHandle(read)) {
		string r;
		GLEM(r);
		result = "Failed CloseHandle(read)"s + r;
		return false;
	}

	return true;
}

inline std::vector<std::string> split(std::string str, char del) noexcept{
    int first = 0;
    int last = str.find_first_of(del);
 
    std::vector<std::string> result;
 
    while (first < str.size()) {
        std::string subStr(str, first, last - first);
 
        result.push_back(subStr);
 
        first = last + 1;
        last = str.find_first_of(del, first);
 
        if (last == std::string::npos) {
            last = str.size();
        }
    }
 
    return result;
}

void YDR::Download() const {
	uint errs = 0;
	string VFs, AFs;
	// タイトル、フォーマットの取得
	#pragma omp parallel sections
	{
		// VideosディレクトリとAudiosディレクトリの作成
		#pragma omp section
		{
			std::error_code ec;
			if (VAA & Video) fs::create_directory(fs::path(outDir + "\\Videos"s), ec);
			if (ec) {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("error! : "s + ec.message());
				errs++;
			}
			ec.clear();
			if (VAA & Audio) fs::create_directory(fs::path(outDir + "\\Audios"s), ec);
			if (ec) {
				std::lock_guard<std::mutex> lock(*(ui->mtx));
				ui->addLOG("error! : "s + ec.message());
				errs++;
			}
		}
		// タイトル取得
		#pragma omp section
		{
			string buf;
			const string cmd = "youtube-dl -e "s + URL;
			if (!Popen(cmd, buf)) {
				std::lock_guard lock(*(ui->mtx));
				ui->addLOG("タイトル取得失敗"s);
				++errs;
			}
			std::lock_guard lock(*(ui->mtx));
			ui->addLOG(QString::fromLocal8Bit(buf.c_str()).toStdString());
		}
		// フォーマット取得
		#pragma omp section
		{
			string buf;
			string cmd = "youtube-dl -F --youtube-skip-dash-manifest "s + URL;
			if (!Popen(cmd, buf)) {
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

	if (errs) return;

	string cmd;
	string cmd1;
	// コマンドの作成
	{
		if (VAA & Video) {
			cmd 			= "youtube-dl --ffmpeg-location \""s + ffdir + "\" -f "s + VFs + " --no-check-certificate --no-part --add-metadata"s;
			if (th) cmd 	+= " --write-thumbnail --embed-thumbnail"s;
			cmd				+= " --output \""s + outDir + "\\Videos\\%(title)s.%(ext)s\" \""s + URL + "\""s;
		}
		if (VAA & Audio) {
			cmd1 			= "youtube-dl --ffmpeg-location \""s + ffdir + "\" -f "s + AFs + " --no-check-certificate --no-part"s
							+ " -x --audio-format "s + string(AC) + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" --add-metadata "s;
			if (th) cmd1 	+= " --write-thumbnail --embed-thumbnail"s;
			cmd1 			+= " --output \""s + outDir + "\\Audios\\%(title)s.%(ext)s\" \""s + URL + "\""s;
		}
	}

	SetLastError(0);

	// ダウンロード
	#pragma omp parallel sections
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
			if (cglob::glob("\\Videos\\"s, ps, fs::path(outDir))) {
				for (const auto& x : ps) {
					std::error_code ec;
					if (x.empty()) {
						std::lock_guard lock(*(ui->mtx));
						ui->addLOG("Empty"s);
						continue;
					}
					fs::path np = fs::path(outDir) / x.filename();
					fs::rename(x, np, ec);
					if (ec.value() != 0 && ec.value() != 17) {
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("Videos fs::rename("s + x.string() + ", "s + np.string() + ", ec) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
						ui->dled();
						errs++;
					}
				}
				std::error_code ec;
				fs::path dd = fs::path(outDir) / fs::path("Videos"s);
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
			if (cglob::glob("\\Audios\\"s, ps, fs::path(outDir))) {
				for (const auto& x : ps) {
					std::error_code ec;
					if (x.empty()) {
						ui->mtx->lock();
						ui->addLOG("Empty"s);
						ui->mtx->unlock();
						continue;
					}
					fs::path np = fs::path(outDir) / x.filename();
					fs::rename(x, np, ec);
					if (ec.value() != 0 && ec.value() != 17) {
						std::lock_guard<std::mutex> lock(*(ui->mtx));
						ui->addLOG("Audios fs::rename("s + x.string() + ", "s + np.string() + ", ce) error! : code = "s + to_string(ec.value()) + " : "s + ec.message());
						ui->dled();
						errs++;
					}
				}
				std::error_code ec;
				fs::path dd = fs::path(outDir) / fs::path("Audios"s);
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

	if (errs) return;

	ui->dled();
	if (ex) exit(0);
	return;
}

int main(int argc, char **argv) {
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, size_t(2560));

    // mainの最初でQApplicationを作っておく
    QApplication app(argc, argv);

    // 自作のWidgetクラスを生成、表示
    Widget *widget = new Widget;
    widget->show();

    // ループに入る
    return app.exec();
}