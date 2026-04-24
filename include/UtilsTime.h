// UtilsTime.h - Time utility functions
#pragma once

#include <chrono>
#include <ctime>
#include <thread>

/// @brief 時間情報を保持する構造体
struct TimeInfo {
	int minutes; // 経過分数
	int seconds; // 経過秒数
	int milliseconds; // 経過ミリ秒数
	double delta_time_ms; // 前回からの差分時間（ミリ秒）
	int frequency_hz; // 更新周波数（Hz）
};

/// @brief 独立した時間管理クラス（クロスプラットフォーム対応）
class TimeManager {
private:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;

	static TimePoint start_time;
	static double last_frame_time_ms;
	static double last_delta_time_ms;

public:
	/// @brief タイマーの初期化
	static void Initialize() {
		start_time = Clock::now();
		last_frame_time_ms = 0.0;
		last_delta_time_ms = 0.0;
	}

	/// @brief 開始からの経過時間を秒単位で取得
	/// @return 経過時間（秒）
	static double GetElapsedTimeSec() {
		auto current = Clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current - start_time);
		return duration.count() / 1000000.0;
	}

	/// @brief 開始からの経過時間をミリ秒単位で取得
	/// @return 経過時間（ミリ秒）
	static double GetElapsedTimeMs() {
		auto current = Clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current - start_time);
		return duration.count() / 1000.0;
	}

	/// @brief フレーム更新時に差分時間を計算（定期呼び出しのみ）
	/// @note main.cppのメインループで毎フレーム呼び出す
	static void UpdateFrameTime() {
		double current_time = GetElapsedTimeMs();
		last_delta_time_ms = current_time - last_frame_time_ms;
		last_frame_time_ms = current_time;
	}

	/// @brief 現在の経過時間から時間情報を生成
	/// @return TimeInfo構造体
	/// @note delta_time_msはLastUpdateFrameTime()からの差分
	static TimeInfo MakeTimeInfo() {
		double elapsed = GetElapsedTimeMs();
		TimeInfo info;
		info.minutes = static_cast<int>(elapsed / 60000.0);
		info.seconds = static_cast<int>(elapsed / 1000.0) % 60;
		info.milliseconds = static_cast<int>(elapsed) % 1000;
		info.delta_time_ms = last_delta_time_ms;
		info.frequency_hz = (last_delta_time_ms > 0.0) ? static_cast<int>(1000.0 / last_delta_time_ms) : 0;
		return info;
	}

	/// @brief 前回の時間を取得
	static double GetLastFrameTimeMs() {
		return last_frame_time_ms;
	}

	/// @brief 前回の差分時間を取得
	static double GetLastDeltaTimeMs() {
		return last_delta_time_ms;
	}
};

inline TimeManager::TimePoint TimeManager::start_time = TimeManager::Clock::now();
inline double TimeManager::last_frame_time_ms = 0.0;
inline double TimeManager::last_delta_time_ms = 0.0;

/// @brief 現在の経過時間を秒単位で取得
/// @return 経過時間（秒）
/// @note TimeManagerを使用して計算
inline double GetCurrentTimeSec() {
	return TimeManager::GetElapsedTimeSec();
}

/// @brief 現在の経過時間をミリ秒単位で取得
/// @return 経過時間（ミリ秒）
/// @note TimeManagerを使用して計算
inline double GetCurrentTimeMs() {
	return TimeManager::GetElapsedTimeMs();
}

/// @brief フレーム時間を更新（メインループで毎フレーム呼び出す）
/// @note この関数で差分時間が計算される
inline void UpdateFrameTime() {
	TimeManager::UpdateFrameTime();
}

/// @brief 現在の時間情報を取得
/// @return TimeInfo構造体
/// @note delta_time_msはUpdateFrameTime()が前回呼ばれてからの差分
inline TimeInfo GetTimeInfo() {
	return TimeManager::MakeTimeInfo();
}

/// @brief 現在のローカル時刻を取得（カレンダー時刻）
/// @return struct tm with current local time
/// @note クロスプラットフォーム対応
inline struct tm GetCurrentLocalTime() {
	auto now = std::chrono::system_clock::now();
	auto time_t_val = std::chrono::system_clock::to_time_t(now);
	struct tm result;
#ifdef _WIN32
	localtime_s(&result, &time_t_val);
#else
	localtime_r(&time_t_val, &result);
#endif
	return result;
}

/// @brief 現在のUnix時刻をミリ秒単位で取得
/// @return Unix timestamp (milliseconds since epoch)
inline long long GetCurrentUnixTimeMs() {
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

/// @brief 指定されたミリ秒だけスリープ
/// @param milliseconds スリープ時間（ミリ秒）
/// @note クロスプラットフォーム対応
inline void SleepMilliseconds(int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
