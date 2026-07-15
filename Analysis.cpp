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
};

constexpr int findCenter(int w) { return w / 2; }
constexpr int doubleWidth(int w) { return ((w - 1) * 2) + 1; } // Need to account for center column with equal width on either side

static const int INITIAL_WIDTH = 255;    // 127 on either side + center
static std::vector<uint8_t> current(INITIAL_WIDTH, 0);
static std::vector<uint8_t> next(INITIAL_WIDTH, 0);
static int center = findCenter(INITIAL_WIDTH);
static int nextGrowStep = center - 1;


static std::string slopeName(const Slope& slope) {
	if (slope.dx == 0) {
		return "center";
	}
	std::ostringstream stream;
	stream << slope.dx << "/" << slope.dy;
	return stream.str();
}

static void growBuffers() {
	const int newWidth = doubleWidth(static_cast<int>(current.size()));
	const int newCentre = findCenter(newWidth);
	const int offset = newCentre - center;

	std::vector<uint8_t> newCurrent(newWidth, 0);
	std::vector<uint8_t> newNext(newWidth, 0);

	std::copy(current.begin(), current.end(), newCurrent.begin() + offset);
	std::copy(next.begin(), next.end(), newNext.begin() + offset);

	current.swap(newCurrent);
	next.swap(newNext);

	center = newCentre;
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
		const int x = slopes[i].dx < 0 ? 7 : 8;
		writeAt(x, static_cast<short>(i + 3), slopeName(slopes[i]));
	}

	// Initial state
	current[center] = 1;
	uint32_t step = 1;
	while (true) {
		writeAt(6, 1, std::to_string(step));

		for (int i = 0; i < slopes.size(); ++i) {
			auto& slope = slopes[i];
			int index = center + slope.x;
			bool black = current[index] != 0;
			if (black) {
				++slope.blackCount;
			}
			else {
				++slope.whiteCount;
			}
			uint64_t total = static_cast<uint64_t>(slope.blackCount) + slope.whiteCount;
			double ratio = (total == 0) ? 0.0 : (static_cast<double>(slope.blackCount) / total);

			std::ostringstream value;
			value << std::fixed << std::setprecision(8) << ratio;
			writeAt(20, static_cast<short>(i + 3), value.str());

			if (slope.dx != 0) {
				slope.accumulator++;
				if (slope.accumulator >= slope.dy) {
					slope.x += slope.dx;
					slope.accumulator = 0;
				}
			}
		}

		std::fill(next.begin(), next.end(), 0);

		const int width = static_cast<int>(current.size());
		for (int i = 1; i < width - 1; ++i) {
			int pattern = (current[i - 1] << 2) | (current[i] << 1) | current[i + 1];
			next[i] = (rule >> pattern) & 1;
		}
		current.swap(next);
		if (step == nextGrowStep) {
			growBuffers();
			nextGrowStep = center - 1;
		}
		step++;
	}

	return 0;
}