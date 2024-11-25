#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Static function for displaying the simulation state
static void display_simulation_state(Manager *manager);

/**
 * Initializes the `Manager`.
 *
 * Sets up the manager by initializing the system array, resource array, and event queue.
 * Prepares the simulation to be run.
 *
 * @param[out] manager  Pointer to the `Manager` to initialize.
 */
void manager_init(Manager *manager) {
    manager->simulation_running = 1; // Any non-zero value to indicate the simulation is running
    system_array_init(&manager->system_array);
    resource_array_init(&manager->resource_array);
    event_queue_init(&manager->event_queue);
}

/**
 * Cleans up the `Manager`.
 *
 * Frees all resources associated with the manager.
 *
 * @param[in,out] manager  Pointer to the `Manager` to clean.
 */
void manager_clean(Manager *manager) {
    system_array_clean(&manager->system_array);
    resource_array_clean(&manager->resource_array);
    event_queue_clean(&manager->event_queue);
}

/**
 * Thread function for running a single system.
 *
 * @param[in] arg  Pointer to the `System` to be run.
 * @return         NULL (no return value).
 */
static void *system_thread_func(void *arg) {
    System *system = (System *)arg;

    while (system->status != TERMINATE) {
        system_run(system);
    }

    return NULL;
}

/**
 * Runs the manager loop.
 *
 * Handles event processing, updates system statuses, and displays the simulation state.
 * Continues until the simulation is no longer running.
 *
 * @param[in,out] manager  Pointer to the `Manager`.
 */
void manager_run(Manager *manager) {
    pthread_t threads[manager->system_array.size];

    // Launch threads for each system
    for (int i = 0; i < manager->system_array.size; i++) {
        if (pthread_create(&threads[i], NULL, system_thread_func, manager->system_array.systems[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread for system.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Main manager loop
    while (manager->simulation_running) {
        Event event;

        // Process events if one is available
        while (event_queue_pop(&manager->event_queue, &event)) {
            printf("Event: [%s] Resource [%s] Status [%d] Priority [%d]\n",
                   event.system->name,
                   event.resource->name,
                   event.status,
                   event.priority);

            // Example: Stop simulation if oxygen or fuel is empty
            if (event.status == STATUS_EMPTY && (strcmp(event.resource->name, "Oxygen") == 0 || strcmp(event.resource->name, "Fuel") == 0)) {
                printf("Critical resource depleted. Terminating simulation.\n");
                manager->simulation_running = 0;
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
 * Displays the current simulation state.
 *
 * Outputs the statuses of resources and systems to the console.
 *
 * @param[in] manager  Pointer to the `Manager` containing the simulation state.
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

    for (int i = 0; i < manager->resource_array.size; i++) {
        Resource *resource = manager->resource_array.resources[i];
        printf("%s: %d / %d\n", resource->name, resource->amount, resource->max_capacity);
    }

    printf("\nSystem Statuses:\n");
    printf("---------------\n");

    for (int i = 0; i < manager->system_array.size; i++) {
        System *system = manager->system_array.systems[i];
        printf("%s: %s\n", system->name, system->status == TERMINATE ? "TERMINATE" : "ACTIVE");
    }

    printf("\n");
    fflush(stdout);

    last_display_time = current_time;
}