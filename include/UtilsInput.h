// UtilsInput.h
#pragma once

#ifdef _WIN32
#include <conio.h>
#else
#include <cstdio>
#include <cerrno>
#include <termios.h>
#include <unistd.h>
#endif

namespace input_ns {

#ifndef _WIN32
	namespace detail {
		inline int& PeekedChar() {
			static int ch = -1;
			return ch;
		}
	}
#endif

	inline int GetCh() {
#ifdef _WIN32
		return _getch();
#else
		if (detail::PeekedChar() != -1) {
			int ch = detail::PeekedChar();
			detail::PeekedChar() = -1;
			return ch;
		}

		unsigned char buf = 0;
		termios old{}, newt{};
		const int fd = STDIN_FILENO;
		const int original_errno = errno;
		int failure_errno = 0;
		bool have_old = false;
		int result = -1;

		auto set_failure_errno = [&](int err) {
			if (failure_errno == 0) {
				failure_errno = err;
			}
		};

		fflush(stdout);

		if (tcgetattr(fd, &old) < 0)
		{
			set_failure_errno(errno);
			goto restore_errno;
		}
		have_old = true;

		newt = old;
		newt.c_lflag &= ~(ICANON | ECHO);
		newt.c_cc[VMIN] = 1;
		newt.c_cc[VTIME] = 0;

		if (tcsetattr(fd, TCSANOW, &newt) < 0)
		{
			set_failure_errno(errno);
			goto restore_termios;
		}

		if (read(fd, &buf, 1) < 0)
		{
			set_failure_errno(errno);
			goto restore_termios;
		}

		result = static_cast<int>(buf);

	restore_termios:
		if (have_old && tcsetattr(fd, TCSADRAIN, &old) < 0)
			set_failure_errno(errno);

	restore_errno:
		errno = (failure_errno != 0 ? failure_errno : original_errno);
		return result;
#endif
	}

	inline bool KbHit() {
#ifdef _WIN32
		return _kbhit() != 0;
#else
		if (detail::PeekedChar() != -1)
			return true;

		termios old{}, newt{};
		unsigned char ch = 0;
		int nread = 0;
		const int fd = STDIN_FILENO;

		if (tcgetattr(fd, &old) < 0)
			return false;

		newt = old;
		newt.c_lflag &= ~(ICANON | ECHO);
		newt.c_cc[VMIN] = 0;
		newt.c_cc[VTIME] = 0;

		if (tcsetattr(fd, TCSANOW, &newt) < 0)
			return false;

		nread = read(fd, &ch, 1);
		tcsetattr(fd, TCSANOW, &old);

		if (nread > 0) {
			detail::PeekedChar() = ch;
			return true;
		}

		return false;
#endif
	}
}
