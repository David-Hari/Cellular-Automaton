#include <windows.h>

#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

struct Slope {
	int dx;
	int dy;

	// Runtime state
	int x = 0;
	int accumulator = 0;
	int blackCount = 0;
	int whiteCount = 0;
	uint8_t byte = 0;
};

static const int INITIAL_WIDTH = 255;
static std::vector<uint8_t> current(INITIAL_WIDTH, 0);
static std::vector<uint8_t> next(INITIAL_WIDTH, 0);
static int centre = INITIAL_WIDTH / 2;
static int nextGrowStep = INITIAL_WIDTH / 2;


static std::ostream& slopeName(std::ostream& stream, const Slope& slope) {
	if (slope.dx == 0) {
		return stream << "center";
	}
	return stream << slope.dx << "/" << slope.dy;
}

static void growBuffers() {
	const int oldWidth = static_cast<int>(current.size());
	const int newWidth = oldWidth * 2;
	const int newCentre = newWidth / 2;
	const int offset = newCentre - centre;

	std::vector<uint8_t> newCurrent(newWidth, 0);
	std::vector<uint8_t> newNext(newWidth, 0);

	std::copy(current.begin(), current.end(), newCurrent.begin() + offset);
	std::copy(next.begin(), next.end(), newNext.begin() + offset);

	current.swap(newCurrent);
	next.swap(newNext);

	centre = newCentre;
}

static void writeAt(short x, short y, const std::string& text) {
	COORD pos{ x, y };
	DWORD written;
	WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.size()), pos, &written);
};


int main(int argc, char* argv[]) {
	int rule = 30;
	if (argc >= 2) {
		rule = std::stoi(argv[1]);
	}

	std::vector<Slope> slopes = {
		{-1, 1},
		{-1, 2},
		{-1, 3},
		{-1, 5},
		{-1, 10},
		{-1, 100},
		{-1, 1000},
		{ 0, 1},
		{ 1, 1000},
		{ 1, 100},
		{ 1, 10},
		{ 1, 5},
		{ 1, 3},
		{ 1, 2},
		{ 1, 1}
	};

	current[centre] = 1;

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor;
	cursor.dwSize = 1;
	cursor.bVisible = FALSE;
	SetConsoleCursorInfo(console, &cursor);
	system("cls"); // Clear the console before starting.

	writeAt(0, 0, "Rule: " + std::to_string(rule));
	writeAt(0, 1, "Step:");
	writeAt(0, 2, "Slope (horz/vert)   Proportion of black / total cells");
	for (int i = 0; i < slopes.size(); ++i) {
		std::ostringstream name;
		slopeName(name, slopes[i]);
		const int x = slopes[i].dx < 0 ? 7 : 8;
		writeAt(x, static_cast<short>(i + 3), name.str());
	}

	uint32_t step = 0;
	while (true) {
		writeAt(6, 1, std::to_string(step));

		for (int i = 0; i < slopes.size(); ++i) {
			auto& slope = slopes[i];
			int index = centre + slope.x;
			bool black = current[index] != 0;
			if (black) {
				++slope.blackCount;
				slope.byte <<= 1;
			}
			else {
				++slope.whiteCount;
				slope.byte = (slope.byte << 1) | 1;
			}
			uint64_t total = static_cast<uint64_t>(slope.blackCount) + slope.whiteCount;
			double ratio = (total == 0) ? 0.0 : (static_cast<double>(slope.blackCount) / total);

			std::ostringstream value;
			value << std::fixed << std::setprecision(4) << ratio;
			writeAt(20, static_cast<short>(i + 3), value.str());

			if (slope.dx != 0) {
				slope.accumulator++;
				if (slope.accumulator >= slope.dy) {
					slope.x += slope.dx;
					slope.accumulator = 0;
				}
			}
		}

		bool writeByte = ((step + 1) % 8) == 0;
		for (auto& s : slopes) {
			if (writeByte) {
				// display byte
				s.byte = 0;
			}
		}

		std::fill(next.begin(), next.end(), 0);

		const int width = static_cast<int>(current.size());
		for (int i = 1; i < width - 1; ++i) {
			if (current[i - 1]) {
			}
			int pattern = (current[i - 1] << 2) | (current[i] << 1) | current[i + 1];
			next[i] = (rule >> pattern) & 1;
		}
		current.swap(next);
		step++;
		if (step == nextGrowStep) {
			growBuffers();
			nextGrowStep = static_cast<int>(current.size()) / 2;
		}
	}

	system("cls");
	cursor.bVisible = TRUE;
	SetConsoleCursorInfo(console, &cursor);

	return 0;
}