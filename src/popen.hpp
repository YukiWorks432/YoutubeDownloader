#ifndef POPEN_HPP_
#define POPEN_HPP_
#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <unordered_map>
#include <memory>
#include "strconv.h"
using std::string;using std::wstring;using std::cout;using std::endl;using std::to_string;
using namespace std::literals::string_literals;

namespace Popen{
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
		auto Er = ::GetLastError();
		cout << Er << endl;
		LPWSTR buf = nullptr;
		size_t size = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, Er,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPWSTR)&buf, 0, NULL
		);
		result = wide_to_utf8(wstring(buf, size));
		LocalFree(buf);
		return;
	}
	inline bool Popen(const string &command, string &result) noexcept{
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
			result = "Failed CreateProcessA() : "s + r;
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
}
#endif