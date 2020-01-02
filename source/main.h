#include "MicroBit.h"

#define MAX_BULLETS_IN_FLIGHT 20
#define MAX_CONCURRENT_ENEMIES 5
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
    struct pos **bullets_in_flight;
};


void init_player() {
    p.lives_left = 0;
    p.number_of_bullets_in_flight = 0;
    p.y_pos = 0;
    p.bullets_in_flight = NULL;
}

list * create_player_bullet() {
    if (p.number_of_bullets_in_flight >= MAX_BULLETS_IN_FLIGHT) {
        return NULL;
    }

    list *bullet = (list *)malloc(sizeof(struct list));
    bullet->pos = (pos *)malloc(sizeof(struct pos));
    bullet->pos->x = 0;
    bullet->pos->y = p.y_pos;

    //new bullet becomes head of list
    if (p.bullets_in_flight == NULL) {
        bullet->next = NULL;
        bullet->previous = NULL;
        p.bullets_in_flight = bullet;
    }
    else {
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

void free_bullet(list *bullet) {

    //remove singleton list
    if (bullet->next == NULL && bullet->previous == NULL) {
        p.bullets_in_flight == NULL;
    }
    //remove end of list
    else if (bullet->next == NULL) {
        bullet->previous->next = NULL;
    }
    //remove head of list. Next becomes head of list
    else if (bullet->previous == NULL)
    {
        bullet->next->previous = NULL;
        p.bullets_in_flight = bullet->next;
    }
    //remove middle of list
    else {
        bullet->next->previous = bullet->previous;
        bullet->previous->next = bullet->next;
    }

    //free bullet
    free(bullet->pos);
    free(bullet);
    p.number_of_bullets_in_flight--;
}
