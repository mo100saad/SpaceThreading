#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Helper fn for displaying sim state
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
    manager->simulation_running = 1; // Sim is running
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
    resource_array_clean(&manager->resource_array); // Free resources
    system_array_clean(&manager->system_array);     // Free systems
    event_queue_clean(&manager->event_queue);       // Free events
}

/**
 * Runs the manager loop.
 *
 * Handles event processing, updates system statuses, and displays the simulation state.
 * Stops when the simulation is no longer running.
 *
 * @param[in,out] manager  Pointer to the `Manager`.
 */
void manager_run(Manager *manager) {
    Event event;
    System *sys = NULL;

    // Periodically update the display
    display_simulation_state(manager);

    // Process events from the queue
    while (event_queue_pop(&manager->event_queue, &event)) {
        printf("Event: [%s] Resource [%s:%d] Status [%d]\n",
               event.system->name, event.resource->name, event.amount, event.status);

        // Check for stopping conditions
        if (strcmp(event.resource->name, "Oxygen") == 0 && event.status == STATUS_EMPTY) {
            printf("Oxygen depleted. Terminating systems.\n");
            manager->simulation_running = 0;
        }
        if (strcmp(event.resource->name, "Distance") == 0 && event.status == STATUS_CAPACITY) {
            printf("Destination reached. Terminating systems.\n");
            manager->simulation_running = 0;
        }

        // Update system statuses
        for (int i = 0; i < manager->system_array.size; ++i) {
            sys = manager->system_array.systems[i];
            if (sys->produced.resource == event.resource) {
                sys->status = (event.status == STATUS_LOW) ? FAST : SLOW;
            }
        }
    }
}

// Helper for ANSI term formatting
#define ANSI_CLEAR "\033[2J"
#define ANSI_MV_TL "\033[H"
#define ANSI_LN_CLR "\033[K"

/**
 * Displays the current simulation state.
 *
 * Outputs the statuses of resources and systems to the console.
 * Updates display periodically to avoid excessive refresh.
 *
 * @param[in] manager  Pointer to the `Manager`.
 */
static void display_simulation_state(Manager *manager) {
    static const int display_interval = 1;
    static time_t last_display_time = 0;

    time_t current_time = time(NULL);
    if (difftime(current_time, last_display_time) < display_interval) {
        return; // Skip refresh
    }

    printf(ANSI_CLEAR ANSI_MV_TL);
    printf(ANSI_LN_CLR "Current Resource Amounts:\n");
    printf(ANSI_LN_CLR "-------------------------\n");

    // Display resources
    for (int i = 0; i < manager->resource_array.size; i++) {
        Resource *resource = manager->resource_array.resources[i];
        printf(ANSI_LN_CLR "%s: %d / %d\n", resource->name, resource->amount, resource->max_capacity);
    }

    printf(ANSI_LN_CLR "\nSystem Statuses:\n");
    printf(ANSI_LN_CLR "---------------\n");

    // Display systems
    for (int i = 0; i < manager->system_array.size; i++) {
        System *system = manager->system_array.systems[i];
        const char *status_str;

        switch (system->status) {
            case TERMINATE: status_str = "TERMINATE"; break;
            case SLOW: status_str = "SLOW"; break;
            case STANDARD: status_str = "STANDARD"; break;
            case FAST: status_str = "FAST"; break;
            default: status_str = "UNKNOWN"; break;
        }

        printf(ANSI_LN_CLR "%-20s: %-10s\n", system->name, status_str);
    }

    printf(ANSI_LN_CLR "\n");
    fflush(stdout);
    last_display_time = current_time;
}
