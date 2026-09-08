// File contains all code to simulate the game

#include <stdio.h>

// Macros to avoid repetition of lengthy code
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed) // To check if button was just pressed down
#define is_down(b) input->buttons[b].is_down // To check if button is pressed down
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed) // To check if button was just released

// Ball and player specs
float player_1_p, player_1_dp, player_2_p, player_2_dp = 0.f;
float player_half_size_x = 2.5, player_half_size_y = 12;
float arena_half_size_x = 85, arena_half_size_y = 45;
float ball_p_x, ball_p_y, ball_dp_x = -130, ball_dp_y = 0, ball_half_size = 1;
float friction = 10.f;

// Plink line x coordinate
float plink_x = 0;

// Score keeping
int player_1_score, player_2_score;

bool winner;

internal void
simulate_player(float* p, float* dp, float ddp, float dt) {
	// Incorporate friction into total acceleration
	ddp -= *dp * friction;

	// Get new position + velocity using current acceleration
	// Speed in units/seconds, dt is seconds passed in that frame, multiply to get how many units player should move in that frame
	*p = *p + *dp * dt + ddp * dt * dt * 0.5f;
	*dp = *dp + ddp * dt;

	// Wall collisions
	if (*p + player_half_size_y > arena_half_size_y) {
		// Change position to just at arena border
		*p = arena_half_size_y - player_half_size_y;
		// Change speed to bounce in opposite direction
		*dp *= -0.5;
	}
	if (*p - player_half_size_y < -1 * arena_half_size_y) {
		// Change position to just at arena border
		*p = -1 * arena_half_size_y + player_half_size_y;
		// Change speed to bounce in opposite direction
		*dp *= -0.5;
	}
}
internal void
simulate_player_friction_only(float* p, float* dp, float ddp, float dt) {
	// Incorporate friction into total acceleration
	ddp -= *dp * friction;

	// Get new position + velocity using current acceleration
	// Speed in units/seconds, dt is seconds passed in that frame, multiply to get how many units player should move in that frame
	*p = *p + *dp * dt + ddp * dt * dt * 0.5f;
	*dp = *dp + ddp * dt;

	// Wall collisions
	if (*p + player_half_size_y > arena_half_size_y) {
		// Change position to just at arena border
		*p = arena_half_size_y - player_half_size_y;
		// Change speed to bounce in opposite direction
		*dp *= -0.5;
	}
	if (*p - player_half_size_y < -1 * arena_half_size_y) {
		// Change position to just at arena border
		*p = -1 * arena_half_size_y + player_half_size_y;
		// Change speed to bounce in opposite direction
		*dp *= -0.5;
	}
}

// Collisions for ball
	// If ball is between leftmost and rightmost x-coordinates of paddle,
	// and ball between upper and lower y-coordinates of paddle,
	// then collision occured
internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y + hs1y < p2y + hs2y);
}

// Buttons
float btn_half_size_x = 30;
float btn_half_size_y = 5;
// Exit button
float btn_exit_x = arena_half_size_x - arena_half_size_x * 0.25;
float btn_exit_y = arena_half_size_y - arena_half_size_y * 0.3;
float btn_exit_half_size = arena_half_size_x * 0.03;
float btn_exit_symbol_half_size = btn_exit_half_size * 0.15;
// Menu buttons
float btn_play_x, btn_settings_x, btn_how_to_x = 0;
float btn_play_y = 5;
float btn_settings_y = -7.5;
float btn_how_to_y = -20;
// Settings buttons
float settings_game_level_y = arena_half_size_y * 0.2;
float btn_easy_x = arena_half_size_x * 0.04;
float btn_medium_x = arena_half_size_x * 0.24;
float btn_hard_x = arena_half_size_x * 0.44;
float btn_evil_x = arena_half_size_x * 0.64;
float settings_btn_half_size_x = 7.5;
float settings_btn_half_size_y = 2.5;


// What gamemode the user is in
// New type Gamemode can only hold two different values -- GM_MENU (i.e. 0) or GM_GAMEPLAY (i.e. 1)
enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
	GM_GAME_OVER,
	GM_PAUSED, //maybe
	GM_SETTINGS,
	GM_HOW_TO_PLAY,
};

Gamemode current_gamemode;

// Game levels -- 0: Easy, 1: Medium (default), 2: Hard, 3: Evil
int game_level = 1;

// Color Scheme
u32 arena_color = 0xffaa33;

// Function is internal so that it cannot be accessed from outside the file, only called
internal void simulate_game(Input* input, float dt) {
	// Draw arena
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, arena_color);
	// Draw arena borders
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);
	// Draw dashed line

	// Main gameplay
	if (current_gamemode == GM_GAMEPLAY) {
		// Easy, medium, and hard mode settings
		switch (game_level) {
			case 0: {
				plink_x = 30;
			} break;
			case 1: {
				plink_x = 0;
			} break;
			case 2: {
				plink_x = -30;
			} break;
			case 3: {
				plink_x = arena_half_size_x * (-1.1);
			} break;
			default: {
				plink_x = 0;
			}
		}

		// User keys control acceleration
		float player_1_ddp = 0.f; // units/seconds^2
		float player_2_ddp = 0.f; // units/seconds^2
		if (is_down(BUTTON_UP) || is_down(BUTTON_W)) player_1_ddp += 2000;
		if (is_down(BUTTON_DOWN) || is_down(BUTTON_S)) player_1_ddp -= 2000;

		//if (ball_p_y > player_2_p + player_half_size_y) player_2_ddp += 2000;
		//if (ball_p_y < player_2_p - player_half_size_y) player_2_ddp -= 2000;
		player_2_ddp = (ball_p_y - player_2_p) * 100;
		if (player_2_ddp > 2000) player_2_ddp = 2000;
		if (player_2_ddp < -2000) player_2_ddp = -2000;

		// Create and move paddles
		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		// Computer paddle only moves after player hits ball
		if (ball_dp_x < 0) simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);
		// TODO: Have computer slow to stop instead of just stopping right after hitting ball
		// Or, keep it this way as it might seem intentional? (computer stops right after the plink
		// to cue the user into where the ball was just hit from)

		// Simulate Ball
		{
			// Position equations for ball
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// Check collision with paddles
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				// Set ball y-axis velocity based on paddle velocity and where it hit paddle
				ball_dp_y = player_1_dp * 0.75f + (ball_p_y - player_1_p) * 2.f;
				player_1_score++;
				play_plink_eb2();
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				// Set ball y-axis velocity based on paddle velocity and where it hit paddle
				ball_dp_y = player_2_dp * 0.75f + (ball_p_y - player_2_p) * 2.f;
				player_2_score++;
			}

			// Check collision with top/bottom of arena; if so, bounce away
			if ((ball_p_y + ball_half_size) > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < arena_half_size_y * -1) {
				ball_p_y = arena_half_size_y * -1 + ball_half_size;
				ball_dp_y *= -1;
			}

			// If ball hits right/left of arena, game over
			if (ball_p_x + ball_half_size > arena_half_size_x || ball_p_x - ball_half_size < arena_half_size_x * -1) {
				if (ball_p_x + ball_half_size > arena_half_size_x) winner = false;
				else winner = true;
				player_1_ddp = 0;
				player_2_ddp = 0;
				player_1_dp = 0;
				player_2_dp = 0;
				ball_dp_x = 0;
				ball_dp_y = 0;
				current_gamemode = GM_GAME_OVER;
			}
		}

		// Draw plink line
		draw_plink_line(plink_x, arena_half_size_y, 0.3f, 4.f, 10, 0.8f, 0xff5500, arena_color);

		// Draw score
		draw_number(player_1_score, 10, 40, 1.f, 0x0000ff);
		// account for numbers needing to expand to left for player 2 by having x as -10 - (5 * # of digits)
		draw_number(player_2_score, -10, 40, 1.f, 0x0000ff);

		// Draw ball
		if (ball_p_x > plink_x) {
			draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, arena_color);
		}
		else draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		// Draw paddles
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);
	}

	else if (current_gamemode == GM_MENU) {
		draw_word("PLINK", 0, 25, 2.f, 0xffffff);

		draw_rect(btn_play_x, btn_play_y, btn_half_size_x, btn_half_size_y, 0xff0000);
		draw_word("PLAY", 0, 5, .8f, 0xffffff);

		draw_rect(btn_settings_x, btn_settings_y, btn_half_size_x, btn_half_size_y, 0xff0000);
		draw_word("SETTINGS", 0, -7.5, .8f, 0xffffff);

		draw_rect(btn_how_to_x, btn_how_to_y, btn_half_size_x, btn_half_size_y, 0xff0000);
		draw_word("HOW TO PLAY", 0, -20, .8f, 0xffffff);
	}

	else if (current_gamemode == GM_GAME_OVER) {

		// TODO: Black overlay here
		clear_screen(0x000000); //temp
		if (winner) {
			// TODO: Display "You Won!" here, along with some other stuff (TBD)
			draw_word("WIN", 0, 0, 1.f, 0xff0000);
		}
		else {
			// TODO: Display "Game Over" here, along with some other stuff (TBD)
			draw_word("LOSE", 0, 0, 1.f, 0xff0000);
		}
	}

	else if (current_gamemode == GM_SETTINGS) {
		// TODO
		// Main pop-up background
		draw_rect(0, 0, arena_half_size_x * 0.8, arena_half_size_y * 0.8, 0x0000ff);
		draw_word("SETTINGS", 0, arena_half_size_y * 0.6, 1.f, 0xffffff);

		// Exit button, top right corner
		draw_rect(btn_exit_x, btn_exit_y, btn_exit_half_size, btn_exit_half_size, 0xbbbbbb);
		draw_word("X", btn_exit_x, btn_exit_y, btn_exit_symbol_half_size, 0x666666);

		// TODO: Allow user to select game mode: Easy, medium, hard, or evil
		float settings_game_level_y = arena_half_size_y * 0.2;
		float easy_x = arena_half_size_x * 0.04;
		float medium_x = arena_half_size_x * 0.24;
		float hard_x = arena_half_size_x * 0.44;
		float evil_x = arena_half_size_x * 0.64;
		float arrow_x;
		draw_word("SELECT GAME MODE", arena_half_size_x * (-0.41), settings_game_level_y, 0.6f, 0xeeeeee);
		// Easy level
		draw_rect(easy_x, settings_game_level_y, btn_half_size_x / 4, btn_half_size_y / 2, 0x00ff00);
		draw_word("EASY", easy_x, settings_game_level_y, 0.5f, 0xeeeeee);
		// Medium level
		draw_rect(medium_x, settings_game_level_y, btn_half_size_x / 4, btn_half_size_y / 2, 0xffbb00);
		draw_word("MEDIUM", medium_x, settings_game_level_y, 0.35f, 0xeeeeee);
		// Hard level
		draw_rect(hard_x, settings_game_level_y, btn_half_size_x / 4, btn_half_size_y / 2, 0xff0000);
		draw_word("HARD", hard_x, settings_game_level_y, 0.5f, 0xeeeeee);
		// Evil level
		draw_rect(evil_x, settings_game_level_y, btn_half_size_x / 4, btn_half_size_y / 2, 0xff00ff);
		draw_word("EVIL", evil_x, settings_game_level_y, 0.5f, 0xeeeeee);
		// Draw downwards arrow on top of what game mode the user is in
		switch (game_level) {
			case 0: {
				arrow_x = easy_x;
			} break;
			case 1: {
				arrow_x = medium_x;
			} break;
			case 2: {
				arrow_x = hard_x;
			} break;
			case 3: {
				arrow_x = evil_x;
			} break;
			default: {
				arrow_x = medium_x;
			}
		}
		// TODO: Draw arrow floating up and down from settings_game_level_y * 1.5 to ... * 2.0
		draw_arrow(2, arrow_x, settings_game_level_y * 1.75, 1.f, 0xffffff);
	}

	else if (current_gamemode == GM_HOW_TO_PLAY) {
		// TODO
		// Main pop-up background
		draw_rect(0, 0, arena_half_size_x * 0.8, arena_half_size_y * 0.8, 0x0000ff);
		draw_word("HOW TO PLAY", 0, 25, 1.f, 0xffffff);

		// Exit button, top right corner
		draw_rect(btn_exit_x, btn_exit_y, btn_exit_half_size, btn_exit_half_size, 0xbbbbbb);
		draw_word("X", btn_exit_x, btn_exit_y, btn_exit_symbol_half_size, 0x666666);
	}

	// Maybe: have if (gamemode == settings OR how to play), then draw pop-up and button
	// so that the code doesn't repeat for both; and have unique features stored in each respective else if above

}

