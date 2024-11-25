#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Static function to display the simulation state
static void display_simulation_state(Manager *manager);

// Thread function for running a single system
static void *system_thread_func(void *arg);

/**
 * Initializes the `Manager`.
 *
 * Prepares the manager by initializing all arrays and the event queue.
 */
void manager_init(Manager *manager) {
    manager->simulation_running = 1; // Set simulation as running
    system_array_init(&manager->system_array);
    resource_array_init(&manager->resource_array);
    event_queue_init(&manager->event_queue);
}

/**
 * Cleans up the `Manager`.
 *
 * Frees resources, systems, and events.
 */
void manager_clean(Manager *manager) {
    system_array_clean(&manager->system_array);
    resource_array_clean(&manager->resource_array);
    event_queue_clean(&manager->event_queue);
}

/**
 * Runs the simulation manager loop.
 *
 * Handles threads, processes events, and terminates the simulation if critical resources are depleted.
 */
void manager_run(Manager *manager) {
    pthread_t threads[manager->system_array.size];

    // Validate systems
    if (manager->system_array.size == 0) {
        fprintf(stderr, "Error: No systems available to run.\n");
        manager->simulation_running = 0;
        return;
    }

    // Create threads for each system
    for (int i = 0; i < manager->system_array.size; i++) {
        if (pthread_create(&threads[i], NULL, system_thread_func, manager->system_array.systems[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread for system.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Main manager loop
    while (manager->simulation_running) {
        Event event;

        // Process events if any exist
        while (event_queue_pop(&manager->event_queue, &event)) {
            printf("Event: [%s] Resource [%s] Status [%d] Priority [%d]\n",
                   event.system->name,
                   event.resource->name,
                   event.status,
                   event.priority);

            // Critical condition: stop simulation if oxygen or fuel is empty
            if (event.status == STATUS_EMPTY && 
               (strcmp(event.resource->name, "Oxygen") == 0 || strcmp(event.resource->name, "Fuel") == 0)) {
                printf("Critical resource [%s] depleted by system [%s].\n", event.resource->name, event.system->name);
                manager->simulation_running = 0;

                // Set all systems to TERMINATE
                for (int i = 0; i < manager->system_array.size; i++) {
                    manager->system_array.systems[i]->status = TERMINATE;
                }
                break;
            }
        }

        // Display simulation state periodically
        display_simulation_state(manager);
        usleep(MANAGER_WAIT_TIME * 1000);
    }

    // Wait for threads to finish
    for (int i = 0; i < manager->system_array.size; i++) {
        pthread_join(threads[i], NULL);
    }
}


/**
 * Thread function to run individual systems.
 *
 * @param[in] arg  Pointer to the `System` to run.
 * @return         NULL.
 */
static void *system_thread_func(void *arg) {
    System *system = (System *)arg;

    while (system->status != TERMINATE) {
        system_run(system);
    }

    return NULL;
}

/**
 * Displays the current simulation state.
 *
 * Outputs the statuses of all resources and systems to the console.
 */
static void display_simulation_state(Manager *manager) {
    static const int display_interval = 1; // Update interval in seconds
    static time_t last_display_time = 0;

    time_t current_time = time(NULL);
    if (difftime(current_time, last_display_time) < display_interval) {
        return; // Skip display if it's not time yet
    }

    printf(ANSI_CLEAR ANSI_MV_TL);
    printf("Current Resource Amounts:\n");
    printf("-------------------------\n");

    // Display resources
    for (int i = 0; i < manager->resource_array.size; i++) {
        Resource *resource = manager->resource_array.resources[i];
        pthread_mutex_lock(&resource->mutex);
        printf("%s: %d / %d\n", resource->name, resource->amount, resource->max_capacity);
        pthread_mutex_unlock(&resource->mutex);
    }

    printf("\nSystem Statuses:\n");
    printf("---------------\n");

    // Display systems
    for (int i = 0; i < manager->system_array.size; i++) {
        System *system = manager->system_array.systems[i];
        printf("%s: %s\n", system->name, 
               (system->status == TERMINATE) ? "TERMINATE" : "ACTIVE");
    }

    printf("\n");
    fflush(stdout);

    last_display_time = current_time;
}
