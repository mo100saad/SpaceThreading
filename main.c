#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void load_data(Manager *manager);

/**
 * Main entry point for the simulation.
 *
 * Initializes the manager, loads data, runs the simulation, and cleans up resources.
 */
int main(void) {
    Manager manager;

    // Step 1: Initialize the manager
    printf("Debug: Initializing manager...\n");
    manager_init(&manager);

    // Step 2: Load the data into the simulation
    printf("Debug: Loading data into the manager...\n");
    load_data(&manager);

    // Step 3: Start and manage the simulation
    printf("Debug: Starting simulation...\n");
    manager_run(&manager);

    // Step 4: Clean up resources and terminate
    printf("Debug: Cleaning up resources...\n");
    manager_clean(&manager);

    return 0;
}

/**
 * Loads sample data for the simulation.
 *
 * Adds resources and systems to the manager to simulate the flight.
 *
 * @param[in,out] manager  Pointer to the `Manager` to populate with data.
 */
void load_data(Manager *manager) {
    Resource *fuel, *oxygen, *energy, *distance;

    // Create resources with names, initial amounts, and capacities
    printf("Debug: Creating resources...\n");
    resource_create(&fuel, "Fuel", 1000, 1000);
    resource_create(&oxygen, "Oxygen", 20, 50);
    resource_create(&energy, "Energy", 30, 50);
    resource_create(&distance, "Distance", 0, 5000);

    // Verify resources are initialized
    if (fuel == NULL || oxygen == NULL || energy == NULL || distance == NULL) {
        fprintf(stderr, "Error: One or more resources failed to initialize.\n");
        exit(EXIT_FAILURE);
    }

    // Add resources to the manager's resource array
    printf("Debug: Adding resources to ResourceArray...\n");
    resource_array_add(&manager->resource_array, fuel);
    resource_array_add(&manager->resource_array, oxygen);
    resource_array_add(&manager->resource_array, energy);
    resource_array_add(&manager->resource_array, distance);

    // Create systems with specific consumption and production requirements
    System *propulsion, *life_support, *crew_capsule, *generator;

    ResourceAmount consume_fuel, produce_distance;
    printf("Debug: Initializing ResourceAmount for propulsion...\n");
    resource_amount_init(&consume_fuel, fuel, 5);
    resource_amount_init(&produce_distance, distance, 25);
    system_create(&propulsion, "Propulsion", consume_fuel, produce_distance, 50, &manager->event_queue);

    ResourceAmount consume_energy, produce_oxygen;
    printf("Debug: Initializing ResourceAmount for life support...\n");
    resource_amount_init(&consume_energy, energy, 7);
    resource_amount_init(&produce_oxygen, oxygen, 4);
    system_create(&life_support, "Life Support", consume_energy, produce_oxygen, 10, &manager->event_queue);

    ResourceAmount consume_oxygen, produce_none;
    printf("Debug: Initializing ResourceAmount for crew capsule...\n");
    resource_amount_init(&consume_oxygen, oxygen, 1);
    resource_amount_init(&produce_none, NULL, 0);
    system_create(&crew_capsule, "Crew", consume_oxygen, produce_none, 2, &manager->event_queue);

    ResourceAmount consume_fuel_energy, produce_energy;
    printf("Debug: Initializing ResourceAmount for generator...\n");
    resource_amount_init(&consume_fuel_energy, fuel, 5);
    resource_amount_init(&produce_energy, energy, 10);
    system_create(&generator, "Generator", consume_fuel_energy, produce_energy, 20, &manager->event_queue);

    // Add systems to the manager's system array
    printf("Debug: Adding systems to SystemArray...\n");
    system_array_add(&manager->system_array, propulsion);
    system_array_add(&manager->system_array, life_support);
    system_array_add(&manager->system_array, crew_capsule);
    system_array_add(&manager->system_array, generator);
}
