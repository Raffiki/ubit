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
    uint8_t y_pos;
    uint8_t number_of_bullets_in_flight;
    struct list *bullets_in_flight;
};
struct player p;

struct enemy
{
    uint8_t lives_left;
    uint8_t x_pos;
    uint8_t y_pos;
    uint8_t number_of_bullets_in_flight;
    struct list *bullets_in_flight;
};
struct enemy *enemies[MAX_CONCURRENT_ENEMIES];

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
            if (enemies[i] != NULL && enemies[i]->x_pos == DIM_X - 1 && (enemies[i]->y_pos == random_y || (enemies[i]->y_pos + 1) == random_y))
            {
                is_occupied = 1;
            }
        }

        if (is_occupied > 0)
        {
            cont--;
        }
        else
        {
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
        uBit.serial.send("cannot create enemy: no free y-slot\n");
    }
    else
    {
        uBit.serial.send(ManagedString(free_slot));
        uBit.serial.send(ManagedString(free_y_pos));
        enemy *e = (enemy *)malloc(sizeof(struct enemy));
        e->lives_left = 5;
        e->bullets_in_flight = NULL;
        e->number_of_bullets_in_flight = 0;
        e->x_pos = DIM_X - 1;
        e->y_pos = free_y_pos;

        enemies[free_slot] = e;
    }
}

void remove_enemy(uint8_t i) {
    free(enemies[i]);
    enemies[i] = NULL;
}

void init_player()
{
    p.lives_left = 0;
    p.number_of_bullets_in_flight = 0;
    p.y_pos = 0;
    p.bullets_in_flight = NULL;
}

void init_enemies()
{
    for (uint8_t i = 0; i < MAX_CONCURRENT_ENEMIES; i++)
    {
        enemies[i] = NULL;
    }
}

list *create_player_bullet()
{
    if (p.number_of_bullets_in_flight >= MAX_BULLETS_IN_FLIGHT)
    {
        return NULL;
    }

    list *bullet = (list *)malloc(sizeof(struct list));
    bullet->pos = (pos *)malloc(sizeof(struct pos));
    bullet->pos->x = 0;
    bullet->pos->y = p.y_pos;

    //new bullet becomes head of list
    if (p.bullets_in_flight == NULL)
    {
        bullet->next = NULL;
        bullet->previous = NULL;
        p.bullets_in_flight = bullet;
    }
    else
    {
        bullet->next = p.bullets_in_flight;
        p.bullets_in_flight->previous = bullet;
        bullet->previous = NULL;
    }

    p.bullets_in_flight = bullet;
    p.number_of_bullets_in_flight++;

    uBit.serial.send(ManagedString(bullet->pos->x));
    uBit.serial.send(ManagedString(p.bullets_in_flight->pos->x));
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
