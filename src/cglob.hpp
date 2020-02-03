// This library is not available in clang.
// Use c++17 or higher.

#ifndef CPP_GLOB_
#define CPP_GLOB_

#include <filesystem>
#include <vector>

namespace cglob {
    namespace fs = std::filesystem;
    using std::string;
	using std::vector;

	inline bool glob(const string sp, vector<fs::path> &paths, const fs::path &dir = fs::current_path()) noexcept{
		if (!sp.empty()) {
			for (const auto &x : fs::recursive_directory_iterator(dir)) {
				fs::path p = x.path();
				if ((!p.empty()) && (p.string().find(sp) != string::npos)) 	paths.push_back(p);
			}
		} else {
			for (const auto &x : fs::recursive_directory_iterator(dir)) {
				fs::path p = x.path();
				if (!p.empty()) 	paths.push_back(p);
			}
		}
		return !paths.empty();
	}
}

#endif