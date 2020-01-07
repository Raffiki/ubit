#include "MicroBit.h"

#define MAX_BULLETS_IN_FLIGHT 10
#define MAX_CONCURRENT_ENEMIES 3
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

void remove_enemy(uint8_t i);

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
    uint8_t return_val = 100;

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

    if (free_y_pos == 100)
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
        uBit.serial.send(ManagedString(e->number_of_bullets_in_flight));
    }
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
        uBit.serial.send("max bullets in flight\n");
        return NULL;
    }

    list *bullet = (list *)malloc(sizeof(struct list));
    if (bullet == NULL)
    {
        uBit.serial.send("could not malloc bullet\n");
        return NULL;
    }
    bullet->pos = (pos *)malloc(sizeof(struct pos));
    if (bullet->pos == NULL)
    {
        uBit.serial.send("could not malloc bullet->pos\n");
        return NULL;
    }
    bullet->pos->x = p->x_pos;
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

list *free_bullet(list *bullet, player *p)
{

    //remove singleton list
    if (bullet->next == NULL && bullet->previous == NULL)
    {
        uBit.serial.send("remove singleton\n");
        p->bullets_in_flight = NULL;

    }

    //remove end of list
    else if (bullet->next == NULL)
    {
        uBit.serial.send("remove tail\n");
        bullet->previous->next = NULL;
    }

    //remove head of list. Next becomes head of list
    else if (bullet->previous == NULL)
    {

        uBit.serial.send("remove head\n");
        bullet->next->previous = NULL;
        p->bullets_in_flight = bullet->next;
    }
    //remove middle of list
    else
    {
        uBit.serial.send("remove middle\n");
        bullet->next->previous = bullet->previous;
        bullet->previous->next = bullet->next;
    }

    //free bullet
    list *next = NULL;
    if (bullet->next != NULL)
    {
        next = bullet->next;
    }

    if (bullet == NULL)
    {

        uBit.serial.send("freeing bullet is NULL\n");
    }
    if (bullet->pos == NULL)
    {

        uBit.serial.send("freeing bullet pos is NULL\n");
    }
    free(bullet->pos);
    free(bullet);

    uBit.serial.send("free\n");
    p->number_of_bullets_in_flight--;

    return next;
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
    list *bullet = p->bullets_in_flight;

    while (bullet)
    {
        bullet->pos->x++;

        if (bullet->pos->x >= DIM_X)
        {
            list *next = free_bullet(bullet, p);
            bullet = next;
        }
        else
        {
            bullet = bullet->next;
        }
        //            uBit.serial.send("advancing enemy bullet\n");
        //            uBit.serial.send(ManagedString(position->pos->x));
        //            uBit.serial.send("  ");
        //            uBit.serial.send(ManagedString(position->pos->y));

        //            bullet->pos->x--;
        //            if (bullet->pos->x <= 0)
        //            {
        //                free_bullet(bullet);
        //                bullet = p->bullets_in_flight;
        //            }
        //            else
        //            {
        //                bullet = bullet->next;
        //            }
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
            if (enemies[i] != NULL && enemies[i]->x_pos == position->pos->x && (enemies[i]->y_pos == position->pos->y || enemies[i]->y_pos + 1 == position->pos->y))
            {
                enemies[i]->lives_left--;
                uBit.serial.send(ManagedString(enemies[i]->lives_left));
                if (enemies[i]->lives_left == 0)
                {
                    remove_enemy(i);
                }
            }
        }
    }
    position = position->next;
}

void remove_enemy(uint8_t i)
{
    uBit.serial.send("removing enemy\n");
    uBit.serial.send(ManagedString(enemies[i]->number_of_bullets_in_flight));

    list *bullet = enemies[i]->bullets_in_flight;

    while (bullet != NULL)
    {
        list *bullet_to_remove = bullet;
        bullet = bullet->next;
        free(bullet_to_remove->pos);
        free(bullet_to_remove);
    }

    free(enemies[i]);
    enemies[i] = NULL;
}