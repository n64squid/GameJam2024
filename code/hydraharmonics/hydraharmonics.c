#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "hydraharmonics.h"
#include "hydra.h"
#include "notes.h"
#include "logic.h"
#include "intro.h"
#include "outro.h"
#include "ui.h"

const MinigameDef minigame_def = {
	.gamename = "A Hydra Harmonics",
	.developername = "Catch-64",
	.description = "Who is the best singer?",
	.instructions = "Grab the most notes to win."
};

#define TIMER_GAME (NOTES_PER_PLAYER * PLAYER_MAX) + (NOTES_PER_PLAYER_SPECIAL * NOTES_SPECIAL_TYPES) + (13 / NOTES_SPEED)
#define TIMER_END_ANNOUNCE 2
#define TIMER_END_DISPLAY 1
#define TIMER_END_FANFARE 2
#define TIMER_END_TOTAL TIMER_END_ANNOUNCE + TIMER_END_DISPLAY + TIMER_END_FANFARE

rdpq_font_t *font_default;
rdpq_font_t *font_clarendon;

float timer;
hydraharmonics_stage_t stage = STAGE_START;

/*==============================
	minigame_init
	The minigame initialization function
==============================*/
void minigame_init()
{
	// Initiate subsystems
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

	// Define some variables
	timer = TIMER_GAME + TIMER_END_TOTAL;
	font_default = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
	font_clarendon = rdpq_font_load("rom:/hydraharmonics/Superclarendon-Regular-01.font64");
	rdpq_text_register_font(FONT_DEFAULT, font_default);
	rdpq_text_register_font(FONT_CLARENDON, font_clarendon);

	// Initiate game elements
	notes_init();
	hydra_init();
	ui_init ();
	intro_init();
	outro_init();
}

/*==============================
	minigame_fixedloop
	Code that is called every loop, at a fixed delta time.
	Use this function for stuff where a fixed delta time is
	important, like physics.
	@param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime)
{

	// Handle the stages
	if (stage == STAGE_START) {
		intro_interact();
	} else if (stage == STAGE_GAME) {
		// Add a new note every second
		timer -= deltatime;
		if ((int)timer != (int)(timer - deltatime) && notes_get_remaining(NOTES_GET_REMAINING_UNSPAWNED)) {
			notes_check_and_add();
		}
		// Skip to the end if there are no notes left
		if (!notes_get_remaining(NOTES_GET_REMAINING_ALL)) {
			notes_destroy_all();
			scores_get_winner();
			stage = STAGE_END;
			for (uint8_t i=0; i<PLAYER_MAX; i++) {
				hydra_animate (i, HYDRA_ANIMATION_SLEEP);
			}
		}
		// Do all the frame-by-frame tasks
		notes_move();
		note_hit_detection();
		hydra_shell_bounce();
	} else if (stage == STAGE_END) {
		outro_interact();
	} else if (stage == STAGE_RETURN_TO_MENU) {
		minigame_end();
	}
}

/*==============================
	minigame_loop
	Code that is called every loop.
	@param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
	// Prepare the RDP
	rdpq_attach(display_get(), NULL);

	// Draw things
	rdpq_set_mode_copy(true);
	ui_draw();
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	hydra_draw();
	rdpq_set_mode_standard();
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	notes_draw();

	// Things that should only be drawn at particular stages
	if (stage == STAGE_GAME) {
		// Game stage
		hydra_move();
	} else if (stage == STAGE_START) {
		// Intro stage / countdown
		intro_draw();
	} else if (stage == STAGE_END) {
		// Announce a winner
		outro_sign_draw();
	}

	rdpq_text_printf(NULL, FONT_DEFAULT, 200, 180, "Timer: %f\nRemaining:%i\nStage:%i\nOutro timer: %li", timer, notes_get_remaining(NOTES_GET_REMAINING_ALL), stage, outro_timer);

	rdpq_detach_show();
	hydra_adjust_hats();
}

/*==============================
	minigame_cleanup
	Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
	// Free allocated memory
	notes_clear();
	hydra_clear();
	scores_clear();
	intro_clear();
	outro_clear();
	ui_clear();

	// Free the fonts
	rdpq_text_unregister_font(FONT_DEFAULT);
	rdpq_font_free(font_default);
	rdpq_text_unregister_font(FONT_CLARENDON);
	rdpq_font_free(font_clarendon);

	// Close the display
	display_close();
}
