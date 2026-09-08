#include <cmath>

// Define 3x5 bitmasks for numbers 0-9
// 1 = filled pixel, 0 = empty space
const unsigned char FONT_3X5_NUMBERS[10][5] = {
	{0x07, 0x05, 0x05, 0x05, 0x07}, // 0 (Binary: 111, 101, 101, 101, 111)
	{0x02, 0x02, 0x02, 0x02, 0x02}, // 1
	{0x07, 0x01, 0x07, 0x04, 0x07}, // 2
	{0x07, 0x01, 0x07, 0x01, 0x07}, // 3
	{0x05, 0x05, 0x07, 0x01, 0x01}, // 4
	{0x07, 0x04, 0x07, 0x01, 0x07}, // 5
	{0x07, 0x04, 0x07, 0x05, 0x07}, // 6
	{0x07, 0x01, 0x01, 0x01, 0x01}, // 7
	{0x07, 0x05, 0x07, 0x05, 0x07}, // 8
	{0x07, 0x05, 0x07, 0x01, 0x07}  // 9
};

const unsigned char FONT_5X5_ARROWS[4][5] = {
	{ 0x04, 0x04, 0x14, 0x0D, 0x04 }, // UP 0
	{ 0x04, 0x04, 0x14, 0x0D, 0x04 }, // RIGHT 1
	{ 0x04, 0x04, 0x15, 0x0E, 0x04 }, // DOWN 2
	{ 0x04, 0x04, 0x14, 0x0D, 0x04 }, // LEFT 3
};

const unsigned char FONT_5X5_LETTERS_UPPER[27][5] = {
{ 0x1E, 0x05, 0x05, 0x05, 0x1E }, // A 0
{ 0x1F, 0x15, 0x15, 0x15, 0x0A }, // B 1
{ 0x0E, 0x11, 0x11, 0x11, 0x0A }, // C 2
{ 0x1F, 0x11, 0x11, 0x11, 0x0E }, // D 3
{ 0x1F, 0x15, 0x15, 0x11, 0x11 }, // E 4
{ 0x1F, 0x05, 0x05, 0x01, 0x01 }, // F 5
{ 0x0E, 0x11, 0x11, 0x15, 0x1D }, // G 6
{ 0x1F, 0x04, 0x04, 0x04, 0x1F }, // H 7
{ 0x11, 0x11, 0x1F, 0x11, 0x11 }, // I 8
{ 0x09, 0x11, 0x11, 0x0F, 0x01 }, // J 9
{ 0x1F, 0x04, 0x04, 0x0A, 0x11 }, // K 10
{ 0x1F, 0x10, 0x10, 0x10, 0x10 }, // L 11
{ 0x1F, 0x02, 0x04, 0x02, 0x1F }, // M 12
{ 0x1F, 0x02, 0x04, 0x08, 0x1F }, // N 13
{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // O 14
{ 0x1F, 0x09, 0x09, 0x09, 0x06 }, // P 15
{ 0x0E, 0x11, 0x15, 0x09, 0x16 }, // Q 16
{ 0x1F, 0x05, 0x05, 0x0D, 0x12 }, // R 17
{ 0x12, 0x15, 0x15, 0x15, 0x09 }, // S 18
{ 0x01, 0x01, 0x1F, 0x01, 0x01 }, // T 19
{ 0x0F, 0x10, 0x10, 0x10, 0x0F }, // U 20 
{ 0x07, 0x08, 0x10, 0x08, 0x07 }, // V 21
{ 0x0F, 0x10, 0x0C, 0x10, 0x0F }, // W 22
{ 0x11, 0x0A, 0x04, 0x0A, 0x11 }, // X 23
{ 0x01, 0x02, 0x1C, 0x02, 0x01 }, // Y 24
{ 0x11, 0x19, 0x15, 0x13, 0x11 }, // Z 25
{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // [SPACE] 26
};

//TODO:
// const unsigned char FONT_5X5_LETTERS_LOWER[26][5]
// const unsigned char FONT_9X13_NUMBERS[10][13]
// const unsigned char FONT_9X13_LETTERS[26][13]

internal void render_background() {
	u32* pixel = (u32*)render_state.memory;
	// For each pixel slot, fill in with the rgb for orange, then increment pixel to point to the next spot in memory
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = x + y;
		}
	}
}

internal void clear_screen(u32 color) {
	u32* pixel = (u32*)render_state.memory;
	// For each pixel slot, fill in with the rgb for the color specified by the user, then increment pixel to point to the next spot in memory
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = color;
		}
	}
}

internal void draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
	// Ensure rect coordinates do not go beyon scope of screen's pixels
	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);

	for (int y = y0; y < y1; y++) {
		// As y iterates from y0 to y1, pixel is the [x0, x1) away from x0 of each row [y0, y1)
		u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = color;
		}
	}
}

internal void
draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color) {
	// First x,y is origin/center of rect,
	// next x,y is what percentage of the screen the rect should occupy in the x and y planes

	// What percentage of the screen the rect should occupy
	x *= render_state.height * render_scale;
	y *= render_state.height * render_scale;
	half_size_x *= render_state.height * render_scale;
	half_size_y *= render_state.height * render_scale;

	// Translate these values to a coordinate system where the origin is in the top-left corner of the screen (rather than the center)
	x += render_state.width / 2.f;
	y += render_state.height / 2.f;

	// Change float unit to pixels
	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;

	draw_rect_in_pixels(x0, y0, x1, y1, color);
}

// Draw border around any rect with center at origin
internal void
draw_arena_borders(float arena_x, float arena_y, u32 color) {
	arena_x *= render_state.height * render_scale;
	arena_y *= render_state.height * render_scale;

	int x0 = (int)((float)render_state.width / 2 - arena_x);
	int x1 = (int)((float)render_state.width / 2 + arena_x);
	int y0 = (int)((float)render_state.height / 2 - arena_y);
	int y1 = (int)((float)render_state.height / 2 + arena_y);

	// Top, left, bottom, right
	draw_rect_in_pixels(x0, 0, x1, y0, color);
	draw_rect_in_pixels(0, 0, x0, render_state.height, color);
	draw_rect_in_pixels(x0, y1, x1, render_state.height, color);
	draw_rect_in_pixels(x1, 0, render_state.width, render_state.height, color);
}

internal void
draw_plink_line(float plink_x, float half_size, float thickness, float padding, int dashes, float dash_spacing, u32 line_color, u32 arena_color) {
	half_size -= padding;
	float y_top = half_size;
	float y_bottom = half_size * (-1);

	float dash_size = (half_size * 2) / dashes;

	// What percentage of the screen the rect should occupy
	draw_rect(plink_x, 0, thickness, half_size, line_color);

	for (int i = 1; i < dashes; i++) {
		draw_rect(plink_x, half_size - i * dash_size, thickness, dash_spacing, arena_color);
	}
}

// Here, x and y are the top left coordinates of the number, not center
internal void
draw_number(int number, float x, float y, float pixel_size, u32 color) {
	if (number < 0) return;
	// If multi-digit, draw first digit then move onto rest
	else if (number > 9) {
		int temp = number;
		int divisor = 1;
		while (temp >= 10) {
			temp /= 10;
			divisor *= 10;
		}

		int first_digit = std::floor(temp);
		int remaining_digits = number % divisor;

		draw_number(first_digit, x, y, pixel_size, color);
		float new_x = x + 4 * pixel_size;
		draw_number(remaining_digits, new_x, y, pixel_size, color);
		return;
	}

	float half_size = pixel_size / 2;

	// Loop through each set of columns in the row
	for (int row = 0; row < 5; row++) {
		unsigned char rowData = FONT_3X5_NUMBERS[number][row];

		// Loop through each column
		for (int col = 0; col < 3; col++) {
			// Check if the specific bit is set (from left to right)
			// Draw pixel at that location if it needs to be drawn
			if (rowData & (1 << (2 - col))) {
				int px = x + ((col - 1) * pixel_size);
				int py = y - ((row - 2) * pixel_size);
				draw_rect(px, py, half_size, half_size, color);
			}
		}
	}
}

// TODO: arrow_type parameter as "DOWN" rather than 2, (etc.), function itself translates
internal void
draw_arrow(int arrow_type, float x, float y, float pixel_size, u32 color) {

	float half_size = pixel_size / 2;
	// Loop through each set of columns in the row
	for (int row = 0; row < 5; row++) {
		unsigned char rowData = FONT_5X5_ARROWS[arrow_type][row];

		// Loop through each column
		for (int col = 0; col < 5; col++) {
			// Check if the specific bit is set (from left to right)
			// Draw pixel at that location if it needs to be drawn
			if (rowData & (1 << (4 - col))) {
				//int py = x + ((col - 2) * pixel_size); // should be px
				//int px = y - ((row - 4) * pixel_size); // should be py
				float px = x + (col - 2) * pixel_size;
				float py = y - (row - 2) * pixel_size;
				draw_rect(px, py, half_size, half_size, color);
			}
		}
	}
}



internal void
draw_word(const char* word, float x, float y, float pixel_size, u32 color) {

	int letter_val;
	float pixel_length = 0.f;
	float space_length = 3.f;
	float half_size = pixel_size / 2;

	// Make the following more concise -- takes up too much time/power doing this to figure out length
	for (int i = 0; word[i] != '\0'; i++) {
		if (word[i] == ' ') pixel_length += space_length * pixel_size;
		else {
			pixel_length += 6.f * pixel_size;
		}
	}
	pixel_length -= 1.f * pixel_size;

	x = x - (pixel_length * 0.5) + 2.5 * pixel_size;
	
	char buffer[256];
	sprintf_s(buffer,
		"pixel_length: (%f)\n",
		pixel_length);
	OutputDebugStringA(buffer);

	for (int i = 0; word[i] != '\0'; i++) {
		if (word[i] == ' ') {
			x += space_length * pixel_size;
			continue;
		}
		else if (word[i] < 'A' || word[i] > 'Z') {
			continue;
		}
		//else {
		letter_val = int(word[i]) - 65;
		//}
		
		// If letter is capital, bitmap = FONT_5X5_LETTERS_UPPER else bitmap = the other one

		// Loop through each set of columns in the row
		for (int row = 0; row < 5; row++) {
			unsigned char rowData = FONT_5X5_LETTERS_UPPER[letter_val][row];

			// Loop through each column
			for (int col = 0; col < 5; col++) {
				// Check if the specific bit is set (from left to right)
				// Draw pixel at that location if it needs to be drawn
				if (rowData & (1 << (4 - col))) {
					//int py = x + ((col - 2) * pixel_size); // should be px
					//int px = y - ((row - 4) * pixel_size); // should be py
					float px = x + (row - 2) * pixel_size;
					float py = y + (col - 2) * pixel_size;
					draw_rect(px, py, half_size, half_size, color);
				}
			}
		}
		x += 6.f * pixel_size;
	}
}

