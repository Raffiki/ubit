#include "main.h"
#include <stdio.h>

uint8_t done = 0;

void advance_game() {}
void advance_bullets()
{
	list *position = p.bullets_in_flight;
	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{
		position->pos->x++;
		if (position->pos->x >= DIM_X) {
			free_bullet(position);
		}
		position = position->next;
	}
}

void display_game()
{
	uBit.display.clear();
	uBit.display.image.setPixelValue(0, p.y_pos, 255);

	list *position = p.bullets_in_flight;
	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{
		//uBit.serial.send(ManagedString(p.bullets_in_flight->pos->x));
		uBit.display.image.setPixelValue(position->pos->x, position->pos->y, 255);
		position = position->next;
	}
}

uint8_t run_loop()
{
	uint8_t i = 0;
	while (!done)
	{
		display_game();
		if (i % 5 == 0) {
			advance_bullets();
			i = 0;
		}
		i++;
		advance_game();
		uBit.sleep(100);
	}
	return 0;
}


void spawn_enemy()
{
	uint8_t x = uBit.random(5);
	uint8_t y = uBit.random(5);
}

void on_move_up(MicroBitEvent e) {
	p.y_pos--;
}

void on_move_down(MicroBitEvent e) {
	p.y_pos++;
}

void on_shoot(MicroBitEvent e) {
	uBit.serial.send("shoot\n");
	struct list *bullet = create_player_bullet();

	if (bullet == NULL) {
		uBit.serial.send("could not acquire next bullet\n");
		return;
	}
}


void on_save(MicroBitEvent e) {

}

void on_restore(MicroBitEvent e) {

}

void set_up_listeners()
{
    uBit.serial.send("setting up\n");
	uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, on_move_up);
	uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, on_move_down);
	uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, on_shoot);
	uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_LONG_CLICK, on_save);
	uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_LONG_CLICK, on_restore);
}

int main()
{
	uBit.init();
	set_up_listeners();
	init_player();
	run_loop();
	release_fiber();
}
