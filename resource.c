#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Resource functions */

/**
 * Creates a new `Resource` object.
 *
 * Allocates memory for a new `Resource` and initializes its fields.
 * The `name` is dynamically allocated, and a mutex is initialized for thread safety.
 *
 * @param[out] resource      Pointer to the `Resource*` to be allocated and initialized.
 * @param[in]  name          Name of the resource (the string is copied).
 * @param[in]  amount        Initial amount of the resource.
 * @param[in]  max_capacity  Maximum capacity of the resource.
 */
void resource_create(Resource **resource, const char *name, int amount, int max_capacity) {
    *resource = malloc(sizeof(Resource));
    if (*resource == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for Resource.\n");
        exit(EXIT_FAILURE);
    }

    (*resource)->name = strdup(name);  // Duplicate the resource name
    if ((*resource)->name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for Resource name.\n");
        free(*resource);
        exit(EXIT_FAILURE);
    }

    (*resource)->amount = amount;
    (*resource)->max_capacity = max_capacity;

    // Initialize the mutex
    if (pthread_mutex_init(&(*resource)->mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex for Resource.\n");
        free((*resource)->name);
        free(*resource);
        exit(EXIT_FAILURE);
    }
}

/**
 * Initializes a `ResourceAmount` structure.
 *
 * Links a `Resource` to a specific `amount` value.
 *
 * @param[out] resource_amount  Pointer to the `ResourceAmount` to initialize.
 * @param[in]  resource         Pointer to the `Resource`.
 * @param[in]  amount           The amount of the resource to associate.
 */
void resource_amount_init(ResourceAmount *resource_amount, Resource *resource, int amount) {
    if (resource_amount == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to resource_amount_init.\n");
        exit(EXIT_FAILURE);
    }
    if (resource == NULL) {
        // Allow NULL resource for scenarios like "produce_none"
        resource_amount->resource = NULL;
        resource_amount->amount = 0;
        printf("Debug: Resource is NULL in resource_amount_init. Allowed for cases like 'produce_none'.\n");
        return;
    }
    resource_amount->resource = resource;
    resource_amount->amount = amount;
}



/**
 * Destroys a `Resource` object.
 *
 * Frees all memory associated with the `Resource`, including the mutex.
 *
 * @param[in,out] resource  Pointer to the `Resource` to be destroyed.
 */
void resource_destroy(Resource *resource) {
    if (resource == NULL) {
        return;
    }

    // Destroy the mutex
    pthread_mutex_destroy(&resource->mutex);
    free(resource->name);
    free(resource);
}

/* ResourceArray functions */

/**
 * Initializes the `ResourceArray`.
 *
 * Allocates memory for the array of `Resource*` pointers and sets initial values.
 *
 * @param[out] array  Pointer to the `ResourceArray` to initialize.
 */
void resource_array_init(ResourceArray *array) {
    array->resources = malloc(sizeof(Resource *) * 1);  // Start with capacity 1
    if (array->resources == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for ResourceArray.\n");
        exit(EXIT_FAILURE);
    }

    array->size = 0;
    array->capacity = 1;
}

/**
 * Cleans up the `ResourceArray` by destroying all resources and freeing memory.
 *
 * Iterates through the array, destroys each `Resource`, and frees the array memory.
 *
 * @param[in,out] array  Pointer to the `ResourceArray` to clean.
 */
void resource_array_clean(ResourceArray *array) {
    for (int i = 0; i < array->size; i++) {
        resource_destroy(array->resources[i]);
    }

    free(array->resources);
    array->resources = NULL;
    array->size = 0;
    array->capacity = 0;
}

/**
 * Adds a `Resource` to the `ResourceArray`, resizing if necessary.
 *
 * Resizes the array when the capacity is reached and adds the new `Resource`.
 * Use of realloc is NOT permitted.
 *
 * @param[in,out] array     Pointer to the `ResourceArray`.
 * @param[in]     resource  Pointer to the `Resource` to add.
 */
void resource_array_add(ResourceArray *array, Resource *resource) {
    if (array->size == array->capacity) {
        // Double the capacity
        int new_capacity = array->capacity * 2;
        Resource **new_resources = malloc(sizeof(Resource *) * new_capacity);
        if (new_resources == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for ResourceArray resizing.\n");
            exit(EXIT_FAILURE);
        }

        // Copy old resources to the new array
        for (int i = 0; i < array->size; i++) {
            new_resources[i] = array->resources[i];
        }

        free(array->resources);
        array->resources = new_resources;
        array->capacity = new_capacity;
    }

    array->resources[array->size++] = resource;  // Add the new resource
}
