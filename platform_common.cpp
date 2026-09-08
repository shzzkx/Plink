struct Button_State {
	// Is the key currently being pressed down
	bool is_down;
	// Has the key changed its state from the last frame
	// I.e. did it go from being pressed to released or released to pressed in that frame
	bool changed;
};

// Enumerate which buttons we care about
enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_W,
	BUTTON_S,
	//add space to pause -- if current_gamemode == GAMEPLAY, then pause game

	BUTTON_COUNT, // Defaults to number of different buttons specified above
};

struct Input {
	Button_State buttons[BUTTON_COUNT]; // Array of buttons of size of how many buttons were specified
};


#include <cstdio>

internal bool
was_button_released(float x_release, float y_release, float btn_x, float btn_y, float btn_half_size_x, float btn_half_size_y) {
	/*char buffer[256];

	sprintf_s(buffer,
		"original release: (%f, %f)\n",
		x_release, y_release);

	OutputDebugStringA(buffer);
	*/
	
	x_release -= render_state.width / 2.f;
	y_release *= (-1.f);
	y_release += (render_state.height / 2.f);

	// Needed because Win32 coor sys and game coor sys are scaled differently
	x_release /= 6.9f;
	y_release /= 6.9f;

	/*
	sprintf_s(buffer,
		"release: (%f, %f), y: (%f, %f)\n",
		x_release, y_release,
		btn_y - btn_half_size_y, btn_y + btn_half_size_y);

	OutputDebugStringA(buffer);
	*/

	if (x_release >= btn_x - btn_half_size_x &&
		x_release <= btn_x + btn_half_size_x &&
		y_release >= btn_y - btn_half_size_y &&
		y_release <= btn_y + btn_half_size_y) {
		return true;
	}
	else return false;
}

// Play audio files
float timer;
extern float delta_time;

void timer_func(float timer) {
	timer += delta_time;
	if (timer > 5.0f) {
		timer = 0.f;
		mciSendString(L"close music", NULL, 0, NULL);
	}
	else timer_func(timer);
}

void play_plink_eb2() {
	mciSendString(
		L"open \"C:\\Users\\skhan\\Downloads\\plink-eb2.mp3\" type mpegvideo alias music",
		NULL,
		0,
		NULL
	);

	mciSendString(L"play music", NULL, 0, NULL);

	timer_func(timer);

	mciSendString(L"close music", NULL, 0, NULL);
}
