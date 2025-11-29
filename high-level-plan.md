
# 'High' Level Plan

-Roope Myller
30.11.2025

# Overview

This document outlines the general high-level plan for the project work in terms of code structure, major components and their interactions. It serves as a roadmap for development, helping to ensure that all parts of the project are aligned and integrated effectively.

# Structure

The structure of the project is organized into several key components, each responsible for a specific aspect of the system. The main components include:  

1. **Main Application (main.c)**  
   - Initializing the system
   - System Mode state management
   - FreeRTOS task creation and management

2. **Controller Module (controller.c/.h)**
    - PI controller logic and algorithms

3. **Plant Simulation (plant.c/.h)**
    - State-space converter model
    - State update and output calculation

4. **User Interface (ui_control.c/.h)**
    - User inputs and display management
    - Change system mode state
    - Buttons, LEDs, UART communication

5. **PWM Output Module (pwm_output.c/.h)**
    - PWM signal generation and management


# Repository Structure

The repository is organized to reflect the modular structure of the project, with separate files for each major component. Below is a suggested directory structure:

```
project_work/src/
├── main.c              // Task creation, initialization
├── controller.c/.h     // PI controller implementation
├── plant.c/.h          // Plant simulation
├── ui_control.c/.h     // User interface
├── pwm_output.c/.h     // PWM output management
└── other files...      // Additional files as needed
```

# FreeRTOS Task ARchitectyure

## Task Priorities (Higher number = Higher priority)

1. **Controller Task** (Priority 3)
    - Period: xx ms ???
    - The PI-controller

2. **Plant Task** (Priority 2)
    - Period: xx ms ???
    - The plant simulation

3. **UI Control Task** (Priority 2)
    - Period: xx ms ???
    - User interface management
        - Buttons
        - LEDs
        - UART comms

4. **PWM Output Task** (Priority 1)
    - Period: xx ms ???
    - PWM signal generation for RGB-led

Or 3 and 4 can be combined into one task if we want
Also, UI can be divided to the UI (buttons and LEDs) and UART comms tasks, might be better for responsiveness and modularity

## Mode-Based Task Behavior

### Configuration Mode
- Controller: **SUSPENDED** (not running)
- Plant: **SUSPENDED** (not running)
- PWM: **OFF** (LED shows mode only)
- UI: **ACTIVE** (parameter adjustment)

### Idling Mode
- Controller: **SUSPENDED**
- Plant: **SUSPENDED**
- PWM: **OFF**
- UI: **ACTIVE** (waiting for mode change)

### Modulating Mode
- Controller: **ACTIVE** (control loop running)
- Plant: **ACTIVE** (simulation running)
- PWM: **ACTIVE** (LED shows control output)
- UI: **ACTIVE** (reference adjustment, monitoring)

## Main.c clarification

- Initializes hardware and peripherals
- Creates FreeRTOS tasks with appropriate priorities
- Manages system mode state transitions based on UI inputs
- Starts the FreeRTOS scheduler to run the tasks