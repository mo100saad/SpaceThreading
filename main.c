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
    manager_init(&manager);

    // Step 2: Load the data into the simulation
    load_data(&manager);

    // Step 3: Start and manage the simulation
    manager_run(&manager);

    // Step 4: Clean up resources and terminate
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
    resource_create(&fuel, "Fuel", 1000, 1000);
    resource_create(&oxygen, "Oxygen", 20, 50);
    resource_create(&energy, "Energy", 30, 50);
    resource_create(&distance, "Distance", 0, 5000);

    // Add resources to the manager's resource array
    resource_array_add(&manager->resource_array, fuel);
    resource_array_add(&manager->resource_array, oxygen);
    resource_array_add(&manager->resource_array, energy);
    resource_array_add(&manager->resource_array, distance);

    // Create systems with specific consumption and production requirements
    System *propulsion, *life_support, *crew_capsule, *generator;

    ResourceAmount consume_fuel, produce_distance;
    resource_amount_init(&consume_fuel, fuel, 5);
    resource_amount_init(&produce_distance, distance, 25);
    system_create(&propulsion, "Propulsion", consume_fuel, produce_distance, 50, &manager->event_queue);

    ResourceAmount consume_energy, produce_oxygen;
    resource_amount_init(&consume_energy, energy, 7);
    resource_amount_init(&produce_oxygen, oxygen, 4);
    system_create(&life_support, "Life Support", consume_energy, produce_oxygen, 10, &manager->event_queue);

    ResourceAmount consume_oxygen, produce_none;
    resource_amount_init(&consume_oxygen, oxygen, 1);
    resource_amount_init(&produce_none, NULL, 0);
    system_create(&crew_capsule, "Crew", consume_oxygen, produce_none, 2, &manager->event_queue);

    ResourceAmount consume_fuel_energy, produce_energy;
    resource_amount_init(&consume_fuel_energy, fuel, 5);
    resource_amount_init(&produce_energy, energy, 10);
    system_create(&generator, "Generator", consume_fuel_energy, produce_energy, 20, &manager->event_queue);

    // Add systems to the manager's system array
    system_array_add(&manager->system_array, propulsion);
    system_array_add(&manager->system_array, life_support);
    system_array_add(&manager->system_array, crew_capsule);
    system_array_add(&manager->system_array, generator);
}
