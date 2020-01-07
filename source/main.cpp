#include "main.h"
#include <stdio.h>

uint8_t done = 0;

void advance_game()
{
	if (p.lives_left <= 0)
	{
		uBit.display.print("YOU LOST! GAME OVER!");
		done = 1;
	}
}

void let_enemies_shoot()
{
	for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
	{
		if (enemies[i] != NULL)
		{
			if (uBit.random(10) % 10 == 0)
			{
				//uBit.serial.send(ManagedString(enemies[i]->number_of_bullets_in_flight));
				struct list *bullet = create_player_bullet(enemies[i]);

				if (bullet == NULL)
				{
					uBit.serial.send("could not acquire next bullet for enemy\n");
				}
			}
		}
	}
}

void advance_bullets()
{

	advance_bullets_for(&p, 0);
	check_bullet_impact(&p);

	//for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
	//{
	//	if (enemies[i] != NULL)
	//	{
	//		advance_bullets_for(enemies[i], 1);
	//		check_bullet_impact(enemies[i]);
	//	}
	//}
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

			list *bullet = enemies[i]->bullets_in_flight;
			while (bullet != NULL)
			{
				uBit.display.image.setPixelValue(bullet->pos->x, bullet->pos->y, 255);
				bullet = bullet->next;
			}
		}
	}
}

uint8_t run_loop()
{
	uint8_t i = 0, j = 0;

	//generate new enemy between 5 and 15 seconds
	uint8_t generate_enemy_cycles = 50 + uBit.random(100);

	while (!done)
	{
		display_game();

		//advance bullets every 500 ms
		if (i % 5 == 0)
		{
			//let_enemies_shoot();
			advance_bullets();
			i = 0;
		}
		i++;

		//advance enemies every 2000 ms
		if (j % 30 == 0)
		{
			//advance_enemies();
			j = 0;
		}
		j++;

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
	struct list *bullet = create_player_bullet(&p);

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
