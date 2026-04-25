// UtilsLogger.h
#pragma once

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "UtilsTime.h"

/// @brief ログレベルの定義
enum class LogLevel {
	LL_ERROR = 0,
	LL_WARNING = 1,
	LL_INFO = 2,
	LL_DEBUG = 3,
	LL_DEBUG_VERBOSE = 4
};

/// @brief ロギングシステムの中央管理クラス
///
/// スレッドセーフなロギング機能を提供。ログレベルによるフィルタリング、
/// コンソールとファイルへの出力、タイムスタンプ付与などをサポート。
///
/// @note シングルトンパターンで実装
class Logger {
public:
	static Logger& GetInstance() {
		static Logger instance;
		return instance;
	}

	static void Log(LogLevel level, const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(level, format, args);
		va_end(args);
	}

	static void Error(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_ERROR, format, args, true);
		va_end(args);
	}

	static void Warning(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_WARNING, format, args);
		va_end(args);
	}

	static void Info(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_INFO, format, args);
		va_end(args);
	}

	static void Debug(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_DEBUG, format, args);
		va_end(args);
	}

	static void DebugVerbose(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_DEBUG_VERBOSE, format, args);
		va_end(args);
	}

	static void ForceWarning(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_WARNING, format, args, true);
		va_end(args);
	}

	static void ForceInfo(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_INFO, format, args, true);
		va_end(args);
	}

	static void ForceDebug(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_DEBUG, format, args, true);
		va_end(args);
	}

	static void ForceDebugVerbose(const char* format, ...) {
		va_list args;
		va_start(args, format);
		GetInstance().LogV(LogLevel::LL_DEBUG_VERBOSE, format, args, true);
		va_end(args);
	}

	static void BlankLine() {
		GetInstance().OutputBlankLine(false);
	}

	static void ForceBlankLine() {
		GetInstance().OutputBlankLine(true);
	}

	static void TimePrint(const TimeInfo& timeInfo) {
		Debug("%02dm%02ds%03dms dt %.3fms / %dHz",
			timeInfo.minutes, timeInfo.seconds, timeInfo.milliseconds,
			timeInfo.delta_time_ms, timeInfo.frequency_hz);
	}

	static void SetLogLevel(LogLevel console_level, LogLevel file_level) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().console_level = console_level;
		GetInstance().file_level = file_level;
	}

	static LogLevel GetLogLevel() {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		return GetInstance().console_level;
	}

	static bool SetLogFile(const std::string& path) {
		const std::string normalized = GetInstance().NormalizeLogPath(path);
		return GetInstance().SetLogFileImpl(normalized);
	}

	/// @brief ISO8601タイムスタンプ付きファイル名でログファイルを設定
	/// @param basePath ベースパス（拡張子がある場合は直前に挿入）。空なら "logs/utils" を基準に生成。
	static bool SetLogFileTimestamped(const std::string& basePath = std::string("logs/utils")) {
		const std::string normalized_base = GetInstance().NormalizeLogPath(basePath);
		const std::string path = GetInstance().MakeTimestampedLogPath(normalized_base);
		return GetInstance().SetLogFileImpl(path);
	}

	static void CloseLogFile() {
		GetInstance().CloseLogFileImpl();
	}

	static void SetTimestampEnabled(bool enabled) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().show_timestamp = enabled;
	}

	static void SetLevelTagEnabled(bool enabled) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().show_level_tag = enabled;
	}

	static void SetColorEnabled(bool enabled) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().color_enabled = enabled;
	}

	static void SetBufferingEnabled(bool enabled) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().buffering_enabled = enabled;
	}

	static void FlushBuffer() {
		GetInstance().FlushBufferImpl();
	}

	static void ClearBuffer() {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().log_buffer.clear();
	}

	static long long GetLastOutputTimeMs() {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		return GetInstance().last_output_time_ms;
	}

	static long long GetLastAnyOutputTimeMs() {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		return GetInstance().last_any_output_time_ms;
	}

	static void SetFlushIntervalMs(int interval_ms) {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		GetInstance().flush_interval_ms = (interval_ms < 0 ? 0 : interval_ms);
	}

	static int GetFlushIntervalMs() {
		std::lock_guard<std::mutex> lock(GetInstance().log_mutex);
		return GetInstance().flush_interval_ms;
	}

	static void FlushBufferIfDue(long long current_time_ms) {
		GetInstance().FlushBufferIfDueImpl(current_time_ms);
	}

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(Logger&&) = delete;

private:
	Logger()
		: console_level(LogLevel::LL_INFO)
		, file_level(LogLevel::LL_DEBUG_VERBOSE)
		, log_file(nullptr)
		, show_timestamp(true)
		, show_level_tag(true)
		, color_enabled(true)
		, buffering_enabled(false)
		, last_output_time_ms(0)
		, last_any_output_time_ms(0)
		, flush_interval_ms(100)
	{}

	~Logger() {
		CloseLogFileImpl();
	}

	void LogV(LogLevel level, const char* format, va_list args, bool is_force = false) {
		std::lock_guard<std::mutex> lock(log_mutex);

		if (level > console_level && level > file_level) {
			return;
		}

		if (is_force || !buffering_enabled) {
			OutputLog(level, format, args, is_force);
			return;
		}

		va_list args_copy;
		va_copy(args_copy, args);

		char buffer[4096];
		vsnprintf(buffer, sizeof(buffer), format, args_copy);
		va_end(args_copy);

		log_buffer.push_back(std::make_pair(level, std::string(buffer)));
	}

	void OutputLog(LogLevel level, const char* format, va_list args, bool is_force = false) {
		const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		if (is_force) {
			last_any_output_time_ms = now_ms;
		} else {
			last_output_time_ms = now_ms;
			last_any_output_time_ms = now_ms;
		}

		struct tm timeinfo = {};
		if (show_timestamp) {
			timeinfo = GetCurrentLocalTime();
		}

		OutputLogConsole(level, show_timestamp ? &timeinfo : nullptr, format, args);

		va_list args_copy;
		va_copy(args_copy, args);
		OutputLogFile(level, show_timestamp ? &timeinfo : nullptr, format, args_copy);
		va_end(args_copy);
	}

	void FlushBufferImpl() {
		std::lock_guard<std::mutex> lock(log_mutex);

		for (const auto& entry : log_buffer) {
			LogLevel level = entry.first;
			const std::string& message = entry.second;
			const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			last_output_time_ms = now_ms;
			last_any_output_time_ms = now_ms;

			if (message.empty()) {
				OutputBlankLineImpl();
				continue;
			}

			struct tm timeinfo = {};
			if (show_timestamp) {
				timeinfo = GetCurrentLocalTime();
			}

			OutputLogConsoleString(level, show_timestamp ? &timeinfo : nullptr, message);
			OutputLogFileString(level, show_timestamp ? &timeinfo : nullptr, message);
		}

		log_buffer.clear();
	}

	void OutputLogConsole(LogLevel level, const struct tm* timeinfo, const char* format, va_list args) {
		if (level > console_level) return;
		if (timeinfo) {
			printf("%02d:%02d:%02d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		if (show_level_tag) {
			const char* level_tag = GetLevelTag(level);
			const char* color_code = GetColorCode(level);
			if (color_enabled) {
				printf("%s", color_code);
			}
			printf("[%s] ", level_tag);
			if (color_enabled) {
				printf("\033[0m");
			}
		}
		vprintf(format, args);
		printf("\r\n");
	}

	void OutputLogFile(LogLevel level, const struct tm* timeinfo, const char* format, va_list args) {
		if (!log_file || level > file_level) return;
		if (timeinfo) {
			fprintf(log_file, "%02d:%02d:%02d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		if (show_level_tag) {
			const char* level_tag = GetLevelTag(level);
			fprintf(log_file, "[%s] ", level_tag);
		}
		vfprintf(log_file, format, args);
		fprintf(log_file, "\n");
		fflush(log_file);
	}

	void OutputLogConsoleString(LogLevel level, const struct tm* timeinfo, const std::string& message) {
		if (level > console_level) return;
		if (timeinfo) {
			printf("%02d:%02d:%02d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		if (show_level_tag) {
			const char* level_tag = GetLevelTag(level);
			const char* color_code = GetColorCode(level);
			if (color_enabled) {
				printf("%s", color_code);
			}
			printf("[%s] ", level_tag);
			if (color_enabled) {
				printf("\033[0m");
			}
		}
		printf("%s\r\n", message.c_str());
	}

	void OutputLogFileString(LogLevel level, const struct tm* timeinfo, const std::string& message) {
		if (!log_file || level > file_level) return;
		if (timeinfo) {
			fprintf(log_file, "%02d:%02d:%02d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}
		if (show_level_tag) {
			const char* level_tag = GetLevelTag(level);
			fprintf(log_file, "[%s] ", level_tag);
		}
		fprintf(log_file, "%s\n", message.c_str());
		fflush(log_file);
	}

	void FlushBufferIfDueImpl(long long current_time_ms) {
		std::lock_guard<std::mutex> lock(log_mutex);
		if (!buffering_enabled) return;

		if (current_time_ms - last_output_time_ms > flush_interval_ms) {
			FlushBufferImplNoLock();
		} else {
			log_buffer.clear();
		}
	}

	void OutputBlankLine(bool is_force = false) {
		std::lock_guard<std::mutex> lock(log_mutex);

		if (is_force || !buffering_enabled) {
			OutputBlankLineImpl();
			const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			if (is_force) {
				last_any_output_time_ms = now_ms;
			} else {
				last_output_time_ms = now_ms;
				last_any_output_time_ms = now_ms;
			}
			return;
		}

		log_buffer.push_back(std::make_pair(LogLevel::LL_INFO, std::string()));
	}

	void OutputBlankLineImpl() {
		printf("\r\n");

		if (log_file) {
			fprintf(log_file, "\n");
			fflush(log_file);
		}
	}

	void FlushBufferImplNoLock() {
		for (const auto& entry : log_buffer) {
			LogLevel level = entry.first;
			const std::string& message = entry.second;
			const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			last_output_time_ms = now_ms;
			last_any_output_time_ms = now_ms;

			if (message.empty()) {
				OutputBlankLineImpl();
				continue;
			}

			struct tm timeinfo = {};
			if (show_timestamp) {
				timeinfo = GetCurrentLocalTime();
			}

			OutputLogConsoleString(level, show_timestamp ? &timeinfo : nullptr, message);
			OutputLogFileString(level, show_timestamp ? &timeinfo : nullptr, message);
		}

		log_buffer.clear();
	}

	bool SetLogFileImpl(const std::string& path) {
		std::lock_guard<std::mutex> lock(log_mutex);

		CloseLogFileImplNoLock();

		if (path.empty()) {
			return true;
		}

		try {
			std::filesystem::path p(path);
			if (p.has_parent_path()) {
				std::filesystem::create_directories(p.parent_path());
			}
		} catch (...) {
			// ディレクトリ作成に失敗しても、以後の fopen でエラーを返す
		}

#ifdef _WIN32
		errno_t err = fopen_s(&log_file, path.c_str(), "a");
		if (err != 0) {
			log_file = nullptr;
			return false;
		}
#else
		log_file = fopen(path.c_str(), "a");
		if (!log_file) {
			return false;
		}
#endif
		return true;
	}

	void CloseLogFileImpl() {
		std::lock_guard<std::mutex> lock(log_mutex);
		CloseLogFileImplNoLock();
	}

	void CloseLogFileImplNoLock() {
		if (log_file) {
			fclose(log_file);
			log_file = nullptr;
		}
	}

	std::string NormalizeLogPath(const std::string& path) const {
		return path;
	}

	std::string MakeTimestampedLogPath(const std::string& basePath) const {
		const auto now = std::chrono::system_clock::now();
		const std::time_t t = std::chrono::system_clock::to_time_t(now);
		struct tm tm = {};
#ifdef _WIN32
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif
		char stamp[32];
		std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tm);

		std::filesystem::path path(basePath);
		if (path.has_extension()) {
			const auto stem = path.stem().string();
			const auto ext = path.extension().string();
			path = path.parent_path() / (stem + "_" + stamp + ext);
		} else {
			path = path.string() + "_" + stamp + ".log";
		}
		return path.string();
	}

	static const char* GetLevelTag(LogLevel level) {
		switch (level) {
		case LogLevel::LL_ERROR: return "ERROR";
		case LogLevel::LL_WARNING: return "WARN";
		case LogLevel::LL_INFO: return "INFO";
		case LogLevel::LL_DEBUG: return "DEBUG";
		case LogLevel::LL_DEBUG_VERBOSE: return "TRACE";
		default: return "LOG";
		}
	}

	static const char* GetColorCode(LogLevel level) {
		switch (level) {
		case LogLevel::LL_ERROR: return "\033[31m";
		case LogLevel::LL_WARNING: return "\033[33m";
		case LogLevel::LL_INFO: return "\033[37m";
		case LogLevel::LL_DEBUG: return "\033[90m";
		case LogLevel::LL_DEBUG_VERBOSE: return "\033[90m";
		default: return "\033[0m";
		}
	}

	LogLevel console_level;
	LogLevel file_level;
	FILE* log_file;
	bool show_timestamp;
	bool show_level_tag;
	bool color_enabled;
	bool buffering_enabled;
	long long last_output_time_ms;
	long long last_any_output_time_ms;
	int flush_interval_ms;
	std::vector<std::pair<LogLevel, std::string>> log_buffer;
	std::mutex log_mutex;
};
