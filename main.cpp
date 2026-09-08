// Incudes Win32 API
#include <windows.h>

// Includes macros to unpack coordinates of mouse clicks etc.
#include <windowsx.h>

// Include utils file
#include "utils.cpp"

// Include mci for playing audio
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


// Variable for if game should be running or not
global_variable bool running = true;

// Struct for graphics buffer
struct Render_State {
	int height, width;
	void* memory;

	// Create an object from the BITMAPINFO struct, which describes what the graphics buffer should look like to Windows (allows it to interpret the raw bytes in the buffer correctly)
	BITMAPINFO bitmap_info;
};

// Render_State object for graphics buffer
global_variable Render_State render_state;
global_variable float render_scale = 0.01f;

// Include files excluded from build to call their functions
#include "platform_common.cpp"
#include "renderer.cpp"
#include "game.cpp"


// Function for whenever Windows sends a message back to the program (e.g. resizing, closing, moving window)
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	// Depending on what type of message it is, run...
	switch (uMsg) {
		// If message is windows closed or destroyed, tell game to stop running
	case WM_CLOSE:
	case WM_DESTROY: {
		running = false;
	} break;

	case WM_SIZE: {
		// RECT struct defines a rectangle (window); stores only left, right, bottom, top attributes of the window
		RECT rect;
		// GetClientRect retrieves the inner client area of the window (refered to using the window's handle) and stores that data in the memory address for rect
		GetClientRect(hwnd, &rect);
		render_state.width = rect.right - rect.left;
		render_state.height = rect.bottom - rect.top;

		// Bit size of graphics buffer = how many pixels there in the buffer * 32 bits per pixel
		// Graphics buffer is a temporary area of memory dedicated to storing the pixel data of the imagery that will be shown on the screen
		int size = render_state.width * render_state.height * sizeof(unsigned int);

		// If there's already data stored in render_state.memory, free it to make space for the next frame's data
		if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
		// Ask Windows to reserve (MEM_RESERVE) a chunk of memory the size of render_state.size (render_state.size) anywhere (0) that we can use for the graphics buffer,
		// that is ready to use immediately (MEM_COMMIT), and that we can read+write to (PAGE_READWRITE)
		// VirtualAlloc returns the starting address of that memory
		render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
		render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
		render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
		render_state.bitmap_info.bmiHeader.biPlanes = 1;
		render_state.bitmap_info.bmiHeader.biBitCount = 32;
		// Bitmap is not compressed; pixel data is stored directly in memory
		render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

	} break;

	default: {
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
	return result;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	/// Create Window Class
	// Create object of WNDCLASS struct
	WNDCLASS window_class = {};
	// Create handle (reference) to that window
	window_class.hInstance = hInstance;
	// Tell window to redraw when resized horizontally or vertically
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	// Create name for the object
	window_class.lpszClassName = L"Game Window Class";
	// Point to the window procedure function (window_callback) should any message occur
	window_class.lpfnWndProc = window_callback;

	/// Register Class
	// Use pointer to refer to window_class object without having to actually copy it
	RegisterClass(&window_class);

	/// Create Window
	// Create a handle for the window called "window"
	HWND window = CreateWindow(window_class.lpszClassName, L"Plink", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
	{
		/*
		// Fullscreen
		// Make window borderless -- make this NOT commented out after
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		// Get monitor info
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		// Set monitor width/height to game window's width/height
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		*/
	}
	HDC hdc = GetDC(window);

	// Create instance of Input struct from platform_common.cpp
	Input input = {};

	// Getting seconds per frame so we can set user's speed in terms of units/s not units/frame
	// Units/frame means different frame rate means different speeds, so player will move faster in smaller screens
	// (which have higher frame rate) than bigger ones (which have more pixels to process therefore lower frame rate)
	// which is not good so units/s will keep speed consistent no matter screen size
	float delta_time = 0.016666f; // How much time elapsed in one frame
	// Get CPU time stamp at beginning of frame
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	// Calculate how many computer ticks in one second
	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		// QuadPart is the attribute part of the LARGE_INTEGER struct that contains the 64-bit data that we need
		performance_frequency = (float)perf.QuadPart;
	}

	// For buttons
	bool is_down = false;
	bool prev_is_down = false;

	/// Game Loop
	while (running) {
		/// Input (get info about how the pixels on your screen should change here)
		// Store window's message (type MSG) in a variable called "message"
		MSG message;

		// Reset the change to input at the beginning of every frame
		// Go through all the buttons and set changed attribute to false
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}

		// If there's a MSG in Windows' MSG queue, then copy it into the memory address for the message variable and remove (PM_REMOVE) it from the queue; rest of while loop processes the message
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {

			switch (message.message) {
			case WM_KEYUP:
			case WM_KEYDOWN: {
				// First, know what key the user pressed
				// wParam contains the virtual key-code for whatever key was pressed down, vk_code stores that virtual key-code
				u32 vk_code = (u32)message.wParam;
				// Secondly, since we're processing KEY_UP (released) and KEY_DOWN (pressed) together, we need to know which happened
				bool is_down = ((message.lParam & (1 << 31)) == 0);

				// If button pressed, create instance of Input struct and go to the appropriate item (representing the state of that button)
				// in the buttons array (containing instances of Button_State) and change is_down attribute to whatever the is_down 
				// bool contains
#define process_button(b, vk)\
					case vk: {\
						input.buttons[b].changed = is_down != input.buttons[b].is_down;\
						input.buttons[b].is_down = is_down;\
					} break;

				switch (vk_code) {
					process_button(BUTTON_UP, VK_UP);
					process_button(BUTTON_DOWN, VK_DOWN);
					process_button(BUTTON_W, 'W');
					process_button(BUTTON_S, 'S');
				}
			} break;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP: {
				// Check if mousepad just got pressed or released
				if (message.message == WM_LBUTTONDOWN) is_down = true;
				else is_down = false;

				if (!is_down && prev_is_down) {
					// If up when prev state down, mousepad released, so get x and y coordinates of release point
					float x = float(GET_X_LPARAM(message.lParam));
					float y = float(GET_Y_LPARAM(message.lParam));

					if (current_gamemode == GM_MENU) {
						if (was_button_released(x, y, btn_play_x, btn_play_y, btn_half_size_x, btn_half_size_y)) current_gamemode = GM_GAMEPLAY;
						else if (was_button_released(x, y, btn_settings_x, btn_settings_y, btn_half_size_x, btn_half_size_y)) current_gamemode = GM_SETTINGS;
						else if (was_button_released(x, y, btn_how_to_x, btn_how_to_y, btn_half_size_x, btn_half_size_y)) current_gamemode = GM_HOW_TO_PLAY;
					}
					else if (current_gamemode == GM_SETTINGS) {
						if (was_button_released(x, y, btn_exit_x, btn_exit_y, btn_exit_half_size, btn_exit_half_size)) current_gamemode = GM_MENU;
						else if (was_button_released(x, y, btn_easy_x, settings_game_level_y, settings_btn_half_size_x, settings_btn_half_size_y)) game_level = 0;
						else if (was_button_released(x, y, btn_medium_x, settings_game_level_y, settings_btn_half_size_x, settings_btn_half_size_y)) game_level = 1;
						else if (was_button_released(x, y, btn_hard_x, settings_game_level_y, settings_btn_half_size_x, settings_btn_half_size_y)) game_level = 2;
						else if (was_button_released(x, y, btn_evil_x, settings_game_level_y, settings_btn_half_size_x, settings_btn_half_size_y)) game_level = 3;
					}
					else if (current_gamemode == GM_HOW_TO_PLAY) {
						if (was_button_released(x, y, btn_exit_x, btn_exit_y, btn_exit_half_size, btn_exit_half_size)) current_gamemode = GM_MENU;
					}
				}

				prev_is_down = is_down;
			} break;

			default: {
				TranslateMessage(&message);
				// Dispatch the message to the WNDPROC function (window_callback) so it can deal with it
				DispatchMessage(&message);
			}
			}
		}

		/// Simulate (change the pixels on your screen here)
		// Start out at the first pixel slot in the buffer's memory
		simulate_game(&input, delta_time); // Instead of creating a whole copy of input to pass as an argument, just point to the actual thing


		/// Render (render those pixels here)
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		// Get CPU time stamp at end of frame
		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		// Calculate seconds per frame
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		frame_begin_time = frame_end_time; // For next frame
	}

}


