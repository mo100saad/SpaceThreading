#include "defs.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Initializes an `Event` structure.
 *
 * Sets up an `Event` with the provided system, resource, status, priority, and amount.
 *
 * @param[out] event     Pointer to the `Event` to initialize.
 * @param[in]  system    Pointer to the `System` that generated the event.
 * @param[in]  resource  Pointer to the `Resource` associated with the event.
 * @param[in]  status    Status code representing the event type.
 * @param[in]  priority  Priority level of the event.
 * @param[in]  amount    Amount related to the event (e.g., resource amount).
 */
void event_init(Event *event, System *system, Resource *resource, int status, int priority, int amount) {
    event->system = system;
    event->resource = resource;
    event->status = status;
    event->priority = priority;
    event->amount = amount;
}

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
    //Check if its null
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex for EventQueue.\n");
        exit(EXIT_FAILURE);
    }
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

    pthread_mutex_destroy(&queue->mutex);  // Destroy the mutex, ensure no memory leak
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
    pthread_mutex_lock(&queue->mutex);  // Lock the mutex
    EventNode *new_node = malloc(sizeof(EventNode));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for EventNode.\n");
        pthread_mutex_unlock(&queue->mutex);  // Unlock before exiting
        exit(EXIT_FAILURE);
    }

    new_node->event = *event;
    new_node->next = NULL;

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
    pthread_mutex_unlock(&queue->mutex);  // Unlock the mutex
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
    pthread_mutex_lock(&queue->mutex);  // Lock the mutex
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);  // Unlock before returning
        return 0; // Queue is empty
    }

    EventNode *node_to_remove = queue->head;
    *event = node_to_remove->event;

    queue->head = queue->head->next;
    free(node_to_remove);
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);  // Unlock the mutex
    return 1;
}