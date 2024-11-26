#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Helper functions just used by this C file to clean up our code
// Using static means they can't get linked into other files
static int system_convert(System *);
static void system_simulate_process_time(System *);
static int system_store_resources(System *);

/**
 * Creates a new `System` object.
 *
 * Allocates memory for a new `System` and initializes its fields.
 * The `name` is dynamically allocated.
 *
 * @param[out] system          Pointer to the `System*` to be allocated and initialized.
 * @param[in]  name            Name of the system (the string is copied).
 * @param[in]  consumed        `ResourceAmount` representing the resource consumed.
 * @param[in]  produced        `ResourceAmount` representing the resource produced.
 * @param[in]  processing_time Processing time in milliseconds.
 * @param[in]  event_queue     Pointer to the `EventQueue` for event handling.
 */
void system_create(System **system, const char *name, ResourceAmount consumed, ResourceAmount produced, int processing_time, EventQueue *event_queue) {
    *system = malloc(sizeof(System));
    if (*system == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for System.\n");
        exit(EXIT_FAILURE);
    }

    (*system)->name = strdup(name);
    if ((*system)->name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for System name.\n");
        free(*system);
        exit(EXIT_FAILURE);
    }

    (*system)->consumed = consumed;
    (*system)->produced = produced;
    (*system)->amount_stored = 0;
    (*system)->processing_time = processing_time;
    (*system)->status = STANDARD;
    (*system)->event_queue = event_queue;
    pthread_mutex_init(&(*system)->mutex, NULL); // Initialize the mutex
}


/**
 * Destroys a `System` object.
 *
 * Frees all memory associated with the `System`.
 *
 * @param[in,out] system  Pointer to the `System` to be destroyed.
 */
void system_destroy(System *system) {
    if (system != NULL) {
        if (system->name != NULL) {
            printf("Debug: Freeing system name: %s\n", system->name);
            free(system->name); // Free dynamically allocated name
        }
        pthread_mutex_destroy(&system->mutex); // Destroy the mutex
        free(system); // Free the System
    }
}



/**
 * Runs the main loop for a `System`.
 *
 * This function manages the lifecycle of a system, including resource conversion,
 * processing time simulation, and resource storage. It generates events based on
 * the success or failure of these operations.
 *
 * @param[in,out] system  Pointer to the `System` to run.
 */
void system_run(System *system) {
    Event event;
    int result_status;
    
    if (system->amount_stored == 0) {
        // Need to convert resources (consume and process)
        result_status = system_convert(system);

        if (result_status != STATUS_OK) {
            // Report that resources were out / insufficient
            event_init(&event, system, system->consumed.resource, result_status, PRIORITY_HIGH, system->consumed.resource->amount);
            event_queue_push(system->event_queue, &event);    
            // Sleep to prevent looping too frequently and spamming with events
            usleep(SYSTEM_WAIT_TIME * 1000);          
        }
    }

    if (system->amount_stored > 0) {
        // Attempt to store the produced resources
        result_status = system_store_resources(system);

        if (result_status != STATUS_OK) {
            event_init(&event, system, system->produced.resource, result_status, PRIORITY_LOW, system->produced.resource->amount);
            event_queue_push(system->event_queue, &event);
            // Sleep to prevent looping too frequently and spamming with events
            usleep(SYSTEM_WAIT_TIME * 1000);
        }
    }
}

/**
 * Converts resources in a `System`.
 *
 * Handles the consumption of required resources and simulates processing time.
 * Updates the amount of produced resources based on the system's configuration.
 *
 * @param[in,out] system           Pointer to the `System` performing the conversion.
 * @return                         `STATUS_OK` if successful, or an error status code.
 */
static int system_convert(System *system) {
    int status;
    Resource *consumed_resource = system->consumed.resource;
    int amount_consumed = system->consumed.amount;

    if (consumed_resource == NULL) {
        status = STATUS_OK;
    } else {
        pthread_mutex_lock(&consumed_resource->mutex);
        if (consumed_resource->amount >= amount_consumed) {
            consumed_resource->amount -= amount_consumed;
            status = STATUS_OK;
        } else {
            status = (consumed_resource->amount == 0) ? STATUS_EMPTY : STATUS_INSUFFICIENT;
        }
        pthread_mutex_unlock(&consumed_resource->mutex);
    }

    if (status == STATUS_OK) {
        system_simulate_process_time(system);

        if (system->produced.resource != NULL) {
            system->amount_stored += system->produced.amount;
        } else {
            system->amount_stored = 0;
        }
    }

    return status;
}

/**
 * Simulates the processing time for a `System`.
 *
 * Adjusts the processing time based on the system's current status (e.g., SLOW, FAST)
 * and sleeps for the adjusted time to simulate processing.
 *
 * @param[in] system  Pointer to the `System` whose processing time is being simulated.
 */
static void system_simulate_process_time(System *system) {
    int adjusted_processing_time;

    // Adjust based on the current system status modifier
    switch (system->status) {
        case SLOW:
            adjusted_processing_time = system->processing_time * 2;
            break;
        case FAST:
            adjusted_processing_time = system->processing_time / 2;
            break;
        default:
            adjusted_processing_time = system->processing_time;
    }

    // Sleep for the required time
    usleep(adjusted_processing_time * 1000);
}

/**
 * Stores produced resources in a `System`.
 *
 * Adds the produced resources to the corresponding resource's amount,
 * considering the maximum capacity. Updates `amount_stored` to reflect
 * any leftover resources that couldn't be stored.
 *
 * @param[in,out] system  Pointer to the `System` storing resources.
 * @return                `STATUS_OK` if all resources were stored, or `STATUS_CAPACITY` if not all could be stored.
 */
static int system_store_resources(System *system) {
    Resource *produced_resource = system->produced.resource;
    int available_space, amount_to_store;

    if (produced_resource == NULL || system->amount_stored == 0) {
        system->amount_stored = 0;
        return STATUS_OK;
    }

    pthread_mutex_lock(&produced_resource->mutex);
    amount_to_store = system->amount_stored;
    available_space = produced_resource->max_capacity - produced_resource->amount;

    if (available_space >= amount_to_store) {
        produced_resource->amount += amount_to_store;
        system->amount_stored = 0;
    } else if (available_space > 0) {
        produced_resource->amount += available_space;
        system->amount_stored = amount_to_store - available_space;
    }
    pthread_mutex_unlock(&produced_resource->mutex);

    return (system->amount_stored == 0) ? STATUS_OK : STATUS_CAPACITY;
}


/**
 * Initializes the `SystemArray`.
 *
 * Allocates memory for the array of `System*` pointers with an initial capacity.
 *
 * @param[out] array  Pointer to the `SystemArray` to initialize.
 */
void system_array_init(SystemArray *array) {
    array->systems = malloc(sizeof(System *) * 1);
    if (array->systems == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for SystemArray.\n");
        exit(EXIT_FAILURE);
    }
    array->size = 0;
    array->capacity = 1;
}

/**
 * Cleans up the `SystemArray` by destroying all systems and freeing memory.
 *
 * @param[in,out] array  Pointer to the `SystemArray` to clean.
 */
void system_array_clean(SystemArray *array) {
    for (int i = 0; i < array->size; i++) {
        system_destroy(array->systems[i]);
    }
    free(array->systems);
    array->size = 0;
    array->capacity = 0;
}

/**
 * Adds a `System` to the `SystemArray`, resizing if necessary (doubling the size).
 *
 * @param[in,out] array   Pointer to the `SystemArray`.
 * @param[in]     system  Pointer to the `System` to add.
 */
void system_array_add(SystemArray *array, System *system) {
    if (array->size == array->capacity) {
        int new_capacity = array->capacity * 2;
        System **new_systems = malloc(sizeof(System *) * new_capacity);
        if (new_systems == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for SystemArray resizing.\n");
            exit(EXIT_FAILURE);
        }

        // Copy existing systems to the new array
        for (int i = 0; i < array->size; i++) {
            new_systems[i] = array->systems[i];
        }

        free(array->systems);
        array->systems = new_systems;
        array->capacity = new_capacity;
    }

    array->systems[array->size++] = system;
}
