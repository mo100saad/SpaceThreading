#include "defs.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Initializes the `EventQueue`.
 *
 * Sets up the queue for use, initializing any necessary data.
 *
 * @param[out] queue  Pointer to the `EventQueue` to initialize.
 */
void event_queue_init(EventQueue *queue) {
    queue->head = NULL;
    queue->size = 0;
}

/**
 * Cleans up the `EventQueue`.
 *
 * Frees all memory associated with the queue, including all nodes.
 *
 * @param[in,out] queue  Pointer to the `EventQueue` to clean.
 */
void event_queue_clean(EventQueue *queue) {
    EventNode *current = queue->head;
    EventNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->size = 0;
}

/**
 * Pushes an `Event` onto the `EventQueue`.
 *
 * Adds the event to the queue in priority order (highest priority first).
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[in]     event  Pointer to the `Event` to push onto the queue.
 */
void event_queue_push(EventQueue *queue, const Event *event) {
    EventNode *new_node = malloc(sizeof(EventNode));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for EventNode.\n");
        exit(EXIT_FAILURE);
    }

    new_node->event = *event;
    new_node->next = NULL;

    // Insert in priority order
    if (queue->head == NULL || queue->head->event.priority < event->priority) {
        new_node->next = queue->head;
        queue->head = new_node;
    } else {
        EventNode *current = queue->head;
        while (current->next != NULL && current->next->event.priority >= event->priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }

    queue->size++;
}

/**
 * Pops an `Event` from the `EventQueue`.
 *
 * Removes the highest priority event from the queue.
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[out]    event  Pointer to the `Event` structure to store the popped event.
 * @return               Non-zero if an event was successfully popped; zero otherwise.
 */
int event_queue_pop(EventQueue *queue, Event *event) {
    if (queue->head == NULL) {
        return 0; // Queue is empty
    }

    EventNode *node_to_remove = queue->head;
    *event = node_to_remove->event;

    queue->head = queue->head->next;
    free(node_to_remove);
    queue->size--;

    return 1;
}