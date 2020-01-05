#include "MicroBit.h"

#define MAX_BULLETS_IN_FLIGHT 20
#define MAX_CONCURRENT_ENEMIES 5
#define ENEMY_WIDTH 2
#define DIM_X 5
#define DIM_Y 5

MicroBit uBit;

struct pos
{
    uint8_t x;
    uint8_t y;
};

struct list
{
    struct pos *pos;
    struct list *next;
    struct list *previous;
};

struct player
{
    uint8_t lives_left;
    uint8_t x_pos;
    uint8_t y_pos;
    uint8_t number_of_bullets_in_flight;
    struct list *bullets_in_flight;
};
struct player p;
struct player *enemies[MAX_CONCURRENT_ENEMIES];

uint8_t get_free_slot()
{
    uint8_t return_val = NULL;
    for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
    {
        if (enemies[i] == NULL)
        {
            return_val = i;
        }
    }
    return return_val;
}

uint8_t get_free_y_pos()
{
    uint8_t cont = 10;
    uint8_t return_val = NULL;

    while (cont > 0)
    {
        uint8_t random_y = uBit.random(DIM_Y - 1);
        uint8_t is_occupied = 0;
        for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
        {
            if (enemies[i] != NULL && enemies[i]->x_pos == DIM_X - 1 && (enemies[i]->y_pos == random_y || enemies[i]->y_pos == random_y + 1))
            {
                is_occupied = 1;
            }
        }

        if (is_occupied > 0)
        {
		    uBit.serial.send(ManagedString(random_y));
            cont--;
        }
        else
        {
    uBit.serial.send(" found y location\n");
		    uBit.serial.send(ManagedString(random_y));
            cont = 0;
            return_val = random_y;
        }
    }

    return return_val;
}

void generate_enemy()
{
    uBit.serial.send("start spawning new enemy\n");

    uint8_t free_slot = get_free_slot();
    if (free_slot == NULL)
    {
        uBit.serial.send("cannot create enemy: no free memory slot\n");
        return;
    }

    uint8_t free_y_pos = get_free_y_pos();

    if (free_y_pos == NULL)
    {
        uBit.serial.send("cannot create enemy: tried 10 times to find free slot ad random. Gave up...\n");
    }
    else
    {
        player *e = (player *)malloc(sizeof(struct player));
        e->lives_left = 5;
        e->bullets_in_flight = NULL;
        e->number_of_bullets_in_flight = 0;
        e->x_pos = DIM_X - 1;
        e->y_pos = free_y_pos;

        enemies[free_slot] = e;
    }
}

void remove_enemy(uint8_t i)
{
    uBit.serial.send("removing enemy\n");
    free(enemies[i]);
    enemies[i] = NULL;
}

void init_player()
{
    p.lives_left = 10;
    p.number_of_bullets_in_flight = 0;
    p.x_pos = 0;
    p.y_pos = 0;
    p.bullets_in_flight = NULL;
}

void init_enemies()
{
    //uBit.serial.send("cleaning enemy locations");
    for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
    {
        enemies[i] = NULL;
    }
}

list *create_player_bullet(player *p)
{
    if (p->number_of_bullets_in_flight >= MAX_BULLETS_IN_FLIGHT)
    {
        return NULL;
    }

    list *bullet = (list *)malloc(sizeof(struct list));
    bullet->pos = (pos *)malloc(sizeof(struct pos));
    bullet->pos->x = 0;
    bullet->pos->y = p->y_pos;

    //new bullet becomes head of list
    if (p->bullets_in_flight == NULL)
    {
        bullet->next = NULL;
        bullet->previous = NULL;
        p->bullets_in_flight = bullet;
    }
    else
    {
        bullet->next = p->bullets_in_flight;
        p->bullets_in_flight->previous = bullet;
        bullet->previous = NULL;
    }

    p->bullets_in_flight = bullet;
    p->number_of_bullets_in_flight++;

    return bullet;
}

void free_bullet(list *bullet)
{

    //remove singleton list
    if (bullet->next == NULL && bullet->previous == NULL)
    {
        p.bullets_in_flight == NULL;
    }
    //remove end of list
    else if (bullet->next == NULL)
    {
        bullet->previous->next = NULL;
    }
    //remove head of list. Next becomes head of list
    else if (bullet->previous == NULL)
    {
        bullet->next->previous = NULL;
        p.bullets_in_flight = bullet->next;
    }
    //remove middle of list
    else
    {
        bullet->next->previous = bullet->previous;
        bullet->previous->next = bullet->next;
    }

    //free bullet
    free(bullet->pos);
    free(bullet);
    p.number_of_bullets_in_flight--;
}

void advance_enemy(uint8_t i)
{
    enemies[i]->x_pos--;
    if (enemies[i]->x_pos == 0)
    {
        p.lives_left--;
        remove_enemy(i);
    }
}

void advance_enemies()
{
    for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
    {
        if (enemies[i] != NULL)
        {
            advance_enemy(i);
        }
    }
}

void advance_bullets_for(player *p, uint8_t is_enemy)
{
    list *position = p->bullets_in_flight;
    for (uint8_t i = 0; i < p->number_of_bullets_in_flight; i++)
    {
        if (is_enemy == 0)
        {
            position->pos->x++;
            if (position->pos->x >= DIM_X)
            {
                free_bullet(position);
            }
        }
        else
        {
            position->pos->x--;
            if (position->pos->x == 0)
            {
                free_bullet(position);
            }
        }
        position = position->next;
    }
}

void check_bullet_impact(player *pl)
{
    list *position = pl->bullets_in_flight;
    for (uint8_t i = 0; i < pl->number_of_bullets_in_flight; i++)
    {
        //check if player is hit
        if (p.x_pos == position->pos->x && p.y_pos == position->pos->y)
        {
            p.lives_left--;
        }

        //check if enemy hit
        for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
        {
            if (enemies[i] != NULL && enemies[i]->x_pos == position->pos->x && enemies[i]->y_pos == position->pos->y)
            {
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