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

static std::ostream& slopeName(std::ostream& stream, const Slope& slope) {
	if (slope.dx == 0) {
		return stream << "center";
	}
	return stream << slope.dx << "/" << slope.dy;
}

static bool getCell(const std::vector<uint8_t>& cells, int index) {
	if (index < 0 || index >= (int)cells.size())
		return false;
	return cells[index] != 0;
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

	std::ofstream csv("output.csv");
	csv << "step";
	for (const auto& slope : slopes) {
		csv << ",";
		slopeName(csv, slope);
	}
	for (const auto& slope : slopes) {
		csv << ",";
		slopeName(csv, slope);
		csv << " byte";
	}
	csv << "\n";

	// Large enough to avoid edge effects.
	const int width = 20000;
	const int centre = width / 2;

	std::vector<uint8_t> current(width, 0);
	std::vector<uint8_t> next(width, 0);
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
		writeAt(8, static_cast<short>(i + 3), name.str());
	}

	uint32_t step = 0;
	while (true) {
		writeAt(6, 1, std::to_string(step));
		csv << step;

		for (int i = 0; i < slopes.size(); ++i) {
			auto& slope = slopes[i];
			int index = centre + slope.x;
			bool black = getCell(current, index);
			if (black) {
				++slope.blackCount;
				slope.byte <<= 1;
			}
			else {
				++slope.whiteCount;
				slope.byte = (slope.byte << 1) | 1;
			}
			double ratio = (slope.blackCount + slope.whiteCount == 0) ? 0.0 :
				(static_cast<double>(slope.blackCount) / (slope.blackCount + slope.whiteCount));

			std::ostringstream value;
			value << std::fixed << std::setprecision(4) << ratio;
			writeAt(20, static_cast<short>(i + 3), value.str());
			csv << "," << ratio;

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
				csv << "," << (int)s.byte;
				s.byte = 0;
			}
			else {
				csv << ",";
			}
		}

		csv << std::endl;
		std::fill(next.begin(), next.end(), 0);

		for (int i = 1; i < width - 1; ++i) {
			int pattern = (current[i - 1] << 2) | (current[i] << 1) | current[i + 1];
			next[i] = (rule >> pattern) & 1;
		}
		current.swap(next);

		step++;
	}

	system("cls");
	cursor.bVisible = TRUE;
	SetConsoleCursorInfo(console, &cursor);

	return 0;
}