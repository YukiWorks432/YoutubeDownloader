#include "Header.hpp"

namespace cglob {
	string glob(const char *sp) {
		for (const auto &x : fs::recursive_directory_iterator(fs::current_path())) {
			string s = x.path().string();
			if (s.find(sp) != string::npos) {
				return s;
			}
		}
		return nfound;
	}
	string glob(string sp) {
		for (const auto &x : fs::recursive_directory_iterator(fs::current_path())) {
			string s = x.path().string();
			if (s.find(sp) != string::npos) {
				return s;
			}
		}
		return nfound;
	}
}

void GLEM() {
	auto Er = GetLastError();
	cout << Er << endl;
	LPTSTR buf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, Er,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		buf, 0, NULL
	);
	cout << buf << endl;
	LocalFree(buf);
	return;
}

void YDR::Download() const {
	if (VAA && Video) fs::create_directory(fs::path(outDir + "\\Videos"s));
	if (VAA && Audio) fs::create_directory(fs::path(outDir + "\\Audios"s));

	string cmd = "youtube-dl -e "s + URL;
	FILE *const fp = popen(cmd.c_str(), "r");
	if (fp == NULL) {
		ui->mtx->lock();
		ui->addLOG("タイトル取得失敗 at : fp == NULL"s);
		ui->dled();
		ui->mtx->unlock();
		return;
	}
	std::array<char, 128> buf;
	while (!feof(fp)) {
		fgets(buf.data(), 128, fp);
	}
	
	{
		string tmp = QString::fromLocal8Bit(buf.data()).toStdString();
		normalizeTitle(tmp);
		ui->mtx->lock();
		ui->addLOG(tmp);
		ui->mtx->unlock();
	}
	string titles = QString::fromLocal8Bit(buf.data()).toLocal8Bit().toStdString();
	normalizeTitle(titles);

	std::array<char, 128> buf1;
	cmd = "youtube-dl -F --youtube-skip-dash-manifest "s + URL;
	FILE *const fp1 = popen(cmd.c_str(), "r");
	string VFs, AFs;
	if (fp1 == NULL) {
		ui->mtx->lock();
		ui->addLOG("フォーマット取得失敗 at : fp1 == NULL"s);
		ui->dled();
		ui->mtx->unlock();
		return;
	}
	string tmp;
	while (!feof(fp1)) {
		fgets(buf1.data(), 128, fp1);
		std::smatch tempV, tempA;
		tmp = string(buf1.data());
		if (tmp.find("best") == string::npos) {
			if (tmp.find("video only") != string::npos) VFs = tmp;
			if (tmp.find("audio only") != string::npos) AFs = tmp;
			VFs = VFs.substr(0, 4);
			AFs = AFs.substr(0, 4);
			if (std::regex_search(VFs, tempV, std::regex("[0-9]{2,3}"))) VFs = tempV.str();
			if (std::regex_search(AFs, tempA, std::regex("[0-9]{2,3}"))) AFs = tempA.str();
		}
	}
	if (std::regex_match(VFs, std::regex("[0-9]{2,3}")) && std::regex_match(AFs, std::regex("[0-9]{2,3}"))) {
		ui->mtx->lock();
		ui->addLOG("フォーマット取得"s);
		ui->mtx->unlock();
	} else {
		ui->mtx->lock();
		ui->addLOG("フォーマット取得失敗 at : Format no match"s);
		ui->dled();
		ui->mtx->unlock();
		return;
	}

	string ffdir = cglob::glob("\\bin\\ffmpeg.exe");
	if (ffdir == cglob::nfound) {
		ui->mtx->lock();
		ui->addLOG("ffmpeg存在確認失敗"s);
		ui->addLOG("アクセス権限を確認してください");
		ui->mtx->unlock();
		return;
	}
	ui->mtx->lock();
	ui->addLOG("ffmpeg 存在確認"s);
	ui->addLOG(ffdir);
	ui->mtx->unlock();

	string cmd1 = "exit"s;
	cmd = "youtube-dl --ffmpeg-location \""s + ffdir + "\" -f "s;
	if (VAA & Video) { 
		if (VAA & Audio) {
			cmd1 = cmd;
			cmd1 += AFs + " -x --audio-format "s + string(AC) + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" "s;
			cmd1 += " --output \""s + outDir + "\\Audios\\"s + titles + ".%(ext)s\" --no-check-certificate --no-part \""s + URL + "\""s;
		}
		cmd += VFs;
		cmd += " --output \""s + outDir + "\\Videos\\"s + titles + ".%(ext)s\" --no-check-certificate --no-part \""s + URL + "\""s;
	}
	if (!(VAA & Video)) {
		cmd += AFs + " -x --audio-format "s + string(AC) + " --audio-quality 0 --postprocessor-args \"-compression_level 12\" "s;
		cmd += " --output \""s + outDir + "\\Audio\\"s + titles + ".%(ext)s\" --no-check-certificate --no-part \""s + URL + "\""s;
	}

	SetLastError(0);

	#pragma omp parallel sections
	{
		#pragma omp section
		{
			HANDLE read, write;
			STARTUPINFOA si = {sizeof(STARTUPINFO)};
			PROCESS_INFORMATION pi = {};
			DWORD ExCo;
			auto wcmd = const_cast<wchar_t *>(utf8_to_wide(cmd).c_str());
			BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(cmd.c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
			if (!res) {
				GLEM();
				ui->mtx->lock();
				ui->addLOG("動画ダウンロードプロセス起動失敗\n"s);
				ui->dled();
				ui->mtx->unlock();
			} else {
				ExCo = WaitForSingleObject(pi.hProcess, INFINITE);
				if (ExCo == WAIT_FAILED) {
					GLEM();
					ui->mtx->lock();
					ui->addLOG("動画ダウンロードプロセス実行失敗 : "s);
					ui->dled();
					ui->mtx->unlock();
				} else {
					ui->mtx->lock();
					ui->addLOG("動画ダウンロードプロセス完了"s);
					ui->mtx->unlock();
				}
			}
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			/*
			DWORD len;
			if (PeekNamedPipe(read, NULL, 0, NULL, &len, NULL)) {
				const uint size = 1024;
				std::array<char, size> buf2;
				if (ReadFile(read, buf2.data(), size - 1, &len, NULL)) {
					cout << buf2.data() << endl;
				} else {
					cout << "Failed ReadFile." << endl;
					wstring wbuf;
					if (GLEM(wbuf)) {
						wcout << wbuf << endl;
					} else {
						cout << "Failed Get Last Error." << endl;
					}
				}
			} else {
				cout << "Faild PeekNamedPipe" << endl;
				wstring wbuf;
				if (GLEM(wbuf)) {
					wcout << wbuf << endl;
				} else {
					cout << "Failed Get Last Error." << endl;
				}
			} //*/
		}
		#pragma omp section
		{
			STARTUPINFOA si = {sizeof(STARTUPINFO)};
			PROCESS_INFORMATION pi = {};
			DWORD ExCo;
			wchar_t* wcmd = const_cast<wchar_t*>(utf8_to_wide(cmd1).c_str());
			BOOL res = CreateProcessA(NULL, const_cast<LPSTR>(cmd1.c_str()),  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
			if (!res) {
				GLEM();
				ui->mtx->lock();
				ui->addLOG("音声ダウンロードプロセス起動失敗"s);
				ui->dled();
				ui->mtx->unlock();
			} else {
				ExCo = WaitForSingleObject(pi.hProcess, INFINITE);
				if (ExCo == WAIT_FAILED) {
					GLEM();
					ui->mtx->lock();
					ui->addLOG("音声ダウンロードプロセス実行失敗"s);
					ui->dled();
					ui->mtx->unlock();
				} else {
					ui->mtx->lock();
					ui->addLOG("音声ダウンロードプロセス完了"s);
					ui->mtx->unlock();
				}
			}
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	ui->mtx->lock();
	ui->dled();
	ui->mtx->unlock();
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