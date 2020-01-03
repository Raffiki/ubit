#include "main.h"
#include <stdio.h>

uint8_t done = 0;

void advance_game() {
	if (p.lives_left <= 0) {
   		uBit.display.print("YOU LOST! GAME OVER!");
		done = 1;
	}
}

void advance_enemy(uint8_t i) {
	enemies[i]->x_pos--;
	if (enemies[i]->x_pos < 0) {
		p.lives_left--;
		remove_enemy(i);
	}
}

void advance_bullets()
{
	list *position = p.bullets_in_flight;
	for (uint8_t i = 0; i < p.number_of_bullets_in_flight; i++)
	{
		position->pos->x++;
		if (position->pos->x >= DIM_X)
		{
			free_bullet(position);
		}
		else
		{
			//check if enemy hit
			for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
			{
				if (enemies[i] != NULL && enemies[i]->x_pos == position->pos->x && enemies[i]->y_pos == position->pos->y)
				{
					//uBit.serial.send("enemy lives left\n");
					//uBit.serial.send(ManagedString(enemies[i]->lives_left));
					enemies[i]->lives_left--;
					if (enemies[i]->lives_left == 0)
					{
						remove_enemy(i);
					}
				}
			}
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

	for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
	{
		if (enemies[i] != NULL)
		{
			uBit.display.image.setPixelValue(enemies[i]->x_pos, enemies[i]->y_pos, 255);
			uBit.display.image.setPixelValue(enemies[i]->x_pos, enemies[i]->y_pos + 1, 255);
		}
	}
}

uint8_t run_loop()
{
	uint8_t i = 0;
	//generate new enemy between 5 and 15 seconds
	uint8_t generate_enemy_cycles = 50 + uBit.random(100);

	while (!done)
	{
		display_game();
		if (i % 5 == 0)
		{
			advance_bullets();
			i = 0;
		}
		i++;

		if (generate_enemy_cycles == 0)
		{
			generate_enemy();
			generate_enemy_cycles = 50 + uBit.random(100);
		}

		advance_game();
		uBit.sleep(100);
		generate_enemy_cycles--;
	}
	return 0;
}

void on_move_up(MicroBitEvent e)
{
	p.y_pos--;
}

void on_move_down(MicroBitEvent e)
{
	p.y_pos++;
}

void on_shoot(MicroBitEvent e)
{
	uBit.serial.send("shoot\n");
	struct list *bullet = create_player_bullet();

	if (bullet == NULL)
	{
		uBit.serial.send("could not acquire next bullet\n");
		return;
	}
}

void on_save(MicroBitEvent e)
{
}

void on_restore(MicroBitEvent e)
{
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
	init_enemies();
	run_loop();
	release_fiber();
}
