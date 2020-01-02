#include "main.h"

MicroBit uBit;

uint8_t done = 0;

void advance_game() {}
void advance_bullets()
{
	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{
		p.bullets_in_flight[i]->x++;
	}

	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{

	}
}

void display_game()
{
	uBit.display.clear();
	uBit.display.image.setPixelValue(0, p.y_pos, 255);


	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{
		struct pos *bullet = p.bullets_in_flight[i];
		uBit.display.image.setPixelValue(bullet->x, bullet->y, 255);
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
	struct pos *next_bullet = pop(&bullets_to_left_freelist);

	if (next_bullet == NULL) {
		uBit.serial.send("could not acquire next bullet\n");
		return;
	}

	next_bullet->x = 0;
	next_bullet->y = p.y_pos;
	p.number_of_bullets_in_flight++;
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
	init_freelist();
	run_loop();
	release_fiber();
}
