// UtilsPath.h - Path utility functions
#pragma once

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

inline std::filesystem::path GetExecutablePath() {
#ifdef _WIN32
	char module_path[MAX_PATH] = {};
	const DWORD length = GetModuleFileNameA(nullptr, module_path, MAX_PATH);
	if (length == 0 || length == MAX_PATH) {
		return std::filesystem::current_path();
	}
	return std::filesystem::path(std::string(module_path, length));
#elif defined(__APPLE__)
	uint32_t size = 0;
	_NSGetExecutablePath(nullptr, &size);
	std::vector<char> buffer(size);
	if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
		return std::filesystem::current_path();
	}
	std::error_code ec;
	const std::filesystem::path resolved = std::filesystem::weakly_canonical(std::filesystem::path(buffer.data()), ec);
	return ec ? std::filesystem::path(buffer.data()) : resolved;
#else
	std::error_code ec;
	const std::filesystem::path resolved = std::filesystem::read_symlink("/proc/self/exe", ec);
	return ec ? std::filesystem::current_path() : resolved;
#endif
}

inline std::filesystem::path GetExecutableDirectory() {
	const std::filesystem::path executable_path = GetExecutablePath();
	if (executable_path.has_parent_path()) {
		return executable_path.parent_path();
	}
	return std::filesystem::current_path();
}

inline bool PathExists(const std::filesystem::path& path) {
	std::error_code ec;
	return std::filesystem::exists(path, ec);
}

inline std::filesystem::path FindAncestorDirectoryContaining(
	const std::filesystem::path& start_path,
	const std::vector<std::string>& marker_names) {
	std::filesystem::path current = start_path;
	std::error_code ec;
	if (std::filesystem::is_regular_file(current, ec)) {
		current = current.parent_path();
	}
	const std::filesystem::path fallback = current;

	while (!current.empty()) {
		for (const std::string& marker_name : marker_names) {
			if (PathExists(current / marker_name)) {
				return current;
			}
		}

		const std::filesystem::path parent = current.parent_path();
		if (parent == current) {
			break;
		}
		current = parent;
	}

	return fallback;
}

inline std::filesystem::path EnsureDirectory(const std::filesystem::path& directory_path) {
	std::error_code ec;
	std::filesystem::create_directories(directory_path, ec);
	return directory_path;
}
