// UtilsCSV.h - CSV load/save utility functions
#pragma once

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#ifdef UTILS_ENABLE_LOGGER
#include "UtilsLogger.h"
static inline void CSV_LOG_ERROR_IMPL(const char* fmt) {
	Logger::Error("%s", fmt);
}

template <typename... Args>
static inline void CSV_LOG_ERROR_IMPL(const char* fmt, Args... args) {
	Logger::Error(fmt, args...);
}

static inline void CSV_LOG_WARNING_IMPL(const char* fmt) {
	Logger::Warning("%s", fmt);
}

template <typename... Args>
static inline void CSV_LOG_WARNING_IMPL(const char* fmt, Args... args) {
	Logger::Warning(fmt, args...);
}

#define CSV_LOG_ERROR(...) CSV_LOG_ERROR_IMPL(__VA_ARGS__)
#define CSV_LOG_WARNING(...) CSV_LOG_WARNING_IMPL(__VA_ARGS__)
#else
template <typename... Args>
static inline void CSV_LOG_ERROR_IMPL(const char* fmt, Args... args) {
	std::fprintf(stderr, "Error: ");
	std::fprintf(stderr, fmt, args...);
	std::fprintf(stderr, "\n");
}

template <typename... Args>
static inline void CSV_LOG_WARNING_IMPL(const char* fmt, Args... args) {
	std::fprintf(stderr, "Warning: ");
	std::fprintf(stderr, fmt, args...);
	std::fprintf(stderr, "\n");
}

#define CSV_LOG_ERROR(...) CSV_LOG_ERROR_IMPL(__VA_ARGS__)
#define CSV_LOG_WARNING(...) CSV_LOG_WARNING_IMPL(__VA_ARGS__)
#endif

// CSV escaping helper (inline so templates can use it)
static inline std::string EscapeCSVField(const std::string& field)
{
	bool need_quote = false;
	for (char c : field) {
		if (c == '"' || c == ',' || c == '\n' || c == '\r') {
			need_quote = true;
			break;
		}
	}
	if (!field.empty() && (field.front() == ' ' || field.back() == ' ')) need_quote = true;

	if (!need_quote) return field;

	std::string out;
	out.reserve(field.size() + 2);
	out.push_back('"');
	for (char c : field) {
		if (c == '"') out.push_back('"');
		out.push_back(c);
	}
	out.push_back('"');
	return out;
}

// Generic CSV loader: supports different numeric types via template parameter T.
// CSV format expected: lines of `index,value` with optional header line when `header_exists`.
template <typename T>
bool LoadCSVValue(const std::string& path, std::vector<T>& values, size_t expected_size, bool header_exists = true)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		CSV_LOG_ERROR("Unable to open file %s for reading", path.c_str());
		return false;
	}

	std::string line;
	bool header_checked = false;
	std::vector<T> loaded_values(expected_size);
	std::vector<bool> value_set(expected_size, false);

	while (std::getline(file, line)) {
		if (line.empty()) continue;
		if (header_exists && !header_checked) {
			header_checked = true;
			continue;
		}

		std::istringstream ss(line);
		std::string token;

		if (!std::getline(ss, token, ',')) continue;
		size_t index = 0;
		try {
			index = std::stoul(token);
		} catch (...) {
			CSV_LOG_WARNING("cannot parse index '%s' in %s", token.c_str(), path.c_str());
			continue;
		}
		if (index >= expected_size) {
			CSV_LOG_WARNING("Index %zu out of range in file %s", index, path.c_str());
			continue;
		}

		if (!std::getline(ss, token, ',')) {
			if (!(ss >> token)) continue;
		}

		std::istringstream vs(token);
		T value;
		if (!(vs >> value)) {
			CSV_LOG_WARNING("cannot parse value '%s' in %s", token.c_str(), path.c_str());
			continue;
		}

		loaded_values[index] = value;
		value_set[index] = true;
	}

	if (std::any_of(value_set.begin(), value_set.end(), [](bool v) { return !v; })) {
		CSV_LOG_WARNING("Not all expected values were set in file %s", path.c_str());
		return false;
	}

	values = std::move(loaded_values);
	return true;
}

// Templated saver: writes numeric values (or anything streamable) as CSV index,value
template <typename T>
bool SaveCSVValue(const std::string& path, const std::vector<T>& values, const std::string& header)
{
	namespace fs = std::filesystem;
	fs::path p(path);
	fs::path tmp = p.string() + std::string(".tmp");
	fs::path bak = p.string() + std::string(".bak");
	std::error_code ec;

	{
		std::ofstream of(tmp, std::ofstream::trunc);
		if (!of.is_open()) {
			CSV_LOG_ERROR("Unable to open temporary file %s for writing", tmp.string().c_str());
			return false;
		}

		std::string hdr = header.empty() ? std::string("value") : header;
		of << "id," << EscapeCSVField(hdr) << "\n";
		for (size_t i = 0; i < values.size(); ++i) {
			of << i << "," << values[i] << "\n";
			if (!of) {
				CSV_LOG_ERROR("write failed while writing to %s", tmp.string().c_str());
				of.close();
				fs::remove(tmp, ec);
				return false;
			}
		}
		of.close();
		if (!of) {
			CSV_LOG_ERROR("closing temporary file %s failed", tmp.string().c_str());
			fs::remove(tmp, ec);
			return false;
		}
	}

	if (fs::exists(p)) {
		if (fs::exists(bak)) {
			fs::remove(bak, ec);
			if (ec) {
				CSV_LOG_WARNING("failed to remove existing backup %s (%s)", bak.string().c_str(), ec.message().c_str());
			}
		}
		fs::rename(p, bak, ec);
		if (ec) {
			if (!fs::copy_file(p, bak, fs::copy_options::overwrite_existing, ec)) {
				CSV_LOG_WARNING("failed to create backup %s (%s)", bak.string().c_str(), ec.message().c_str());
			} else {
				fs::remove(p, ec);
			}
		}
	}

	fs::rename(tmp, p, ec);
	if (ec) {
		if (!fs::copy_file(tmp, p, fs::copy_options::overwrite_existing, ec)) {
			CSV_LOG_ERROR("failed to replace target file %s with %s (%s)", p.string().c_str(), tmp.string().c_str(), ec.message().c_str());
			fs::remove(tmp, ec);
			return false;
		}
		fs::remove(tmp, ec);
	}

	return true;
}

#ifdef UTILS_ENABLE_LOGGER
#undef CSV_LOG_ERROR
#undef CSV_LOG_WARNING
#endif
