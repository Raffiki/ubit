#include "MicroBit.h"

#define MAX_BULLETS_IN_FLIGHT 20

struct pos
{
    uint8_t x;
    uint8_t y;
};

struct freelist
{
    struct pos *pos;
    struct freelist *next;
};

struct player
{
    uint8_t lives_left;
    uint8_t y_pos;
    uint8_t number_of_bullets_in_flight;
    struct pos **bullets_in_flight;
};

struct enemy
{
    uint8_t lives_left;
    uint8_t x_pos;
    uint8_t y_pos;
    uint8_t number_of_bullets_in_flight;
    struct pos **bullets_in_flight;
};

struct freelist *bullets_to_left_freelist = NULL;
struct player p ;

void push_onto_freelist(freelist **head, pos *pos)
{
    freelist *new_head;
    new_head = (freelist *)malloc(sizeof(freelist));

    new_head->pos = pos;
    new_head->next = *head;
    *head = new_head;
}

pos * pop(freelist **head)
{
    freelist *next_node = NULL;

    if (*head == NULL)
    {
        return NULL;
    }

    next_node = (*head)->next;
    pos *ptr = (*head)->pos;
    free(*head);
    *head = next_node;
    return ptr;
}

void init_freelist()
{
    p = {10, 0, 0, (pos **)malloc(10 * sizeof(struct pos *))};
    for (u_int8_t i = MAX_BULLETS_IN_FLIGHT - 1; i <= 0; i++)
    {
        pos *ptr = (pos *)malloc(sizeof(struct pos));
        p.bullets_in_flight[i] = ptr;
        push_onto_freelist(&bullets_to_left_freelist, ptr);
    }
}
