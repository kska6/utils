// UtilsInput.h
#pragma once

#ifdef _WIN32
#include <conio.h>
#else
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#endif

namespace input_ns {
	inline int GetCh() {
#ifdef _WIN32
		return _getch();
#else
		char buf = 0;
		termios old = { 0 };

		fflush(stdout);

		if (tcgetattr(0, &old) < 0)
			perror("tcsetattr()");

		old.c_lflag &= ~ICANON;
		old.c_lflag &= ~ECHO;
		old.c_cc[VMIN] = 1;
		old.c_cc[VTIME] = 0;

		if (tcsetattr(0, TCSANOW, &old) < 0)
			perror("tcsetattr ICANON");

		if (read(0, &buf, 1) < 0)
			perror("read()");

		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;

		if (tcsetattr(0, TCSADRAIN, &old) < 0)
			perror("tcsetattr ~ICANON");

		return buf;
#endif
	}

	inline bool KbHit() {
#ifdef _WIN32
		return _kbhit() != 0;
#else
		termios old = { 0 }, newt = { 0 };
		unsigned char ch = 0;
		int nread = 0;

		if (tcgetattr(0, &old) < 0)
			return false;

		newt = old;
		newt.c_lflag &= ~(ICANON | ECHO);
		newt.c_cc[VMIN] = 0;
		newt.c_cc[VTIME] = 0;

		if (tcsetattr(0, TCSANOW, &newt) < 0)
			return false;

		nread = read(0, &ch, 1);
		tcsetattr(0, TCSANOW, &old);

		return nread > 0;
#endif
	}
}
