# UI Control Details

-Roope Myller
30.11.2025

The UI includes:
- 4 buttons for mode and parameter control
- LEDs for status indication
- UART communication to serial terminal


## Controlling System Modes

UART Console Commands always working:
- `set mode <mode_name>`: Switch to another mode (configuration/idle/modulation)
- `show mode`: Get current system mode
- `help`: print available commands

**Button num1** is used to cycle through system modes:

1. **Configuration Mode**
    - Adjust controller parameters (Kp, Ki)
    - LED indication: specific LED or Color (RGB)
    - Buttons
        - B1: Switch mode
        - B2: Switch parameter (Kp/Ki)
        - B3: Decrease parameter value
        - B4: Increase parameter value
    - UART Console Commands
        - `set kp <value>`: Set Kp parameter
        - `set ki <value>`: Set Ki parameter
        - `show params`: Get current Kp and Ki values

2. **Idle Mode**
    - System is idle, no active control
    - LED indication: specific LED or Color (RGB)
    - Buttons
        - B1: Switch mode
        - B2, B3, B4: No function
    - UART Console Commands
        - no specific commands in this mode

3. **Modulation Mode**
    - Active control of the plant using the PI controller
    - LED indication: specific LED or Color (RGB)
    - Buttons
        - B1: Switch mode
        - B2: No function
        - B3: Decrease reference voltage
        - B4: Increase reference voltage
    - UART Console Commands:
        - `set ref <value>`: Set reference voltage
        - `show ref`: Get current reference voltage

## UI Task Structure

### Option A: Single UI Task (Recommended for simplicity, maybe start with this one)

```
UI Task (Priority 1, Period: 100ms)
├── Button Handler (non-blocking)
├── LED Update
└── UART Handler (non-blocking)
```

**Advantages:**
- Simpler synchronization
- All UI logic in one place
- Easier to maintain mode state

### Option B: Split UI Tasks (Better for responsiveness)

```
Button/LED Task (Priority 2, Period: 50ms)
└── Handle buttons and update LEDs

UART Task (Priority 1, Period: 100ms)
└── Handle console input/output
```

**Advantages:**
- More responsive button handling
- UART processing doesn't block button response
- Better modularity

**Recommendation:** Start with **Option A**, refactor to Option B if needed.

# Error Handling

We should use error handling for basic stuff like:
- Invalid UART commands
- Button debouncing issues
- Out-of-range parameter values
- etc. anything that comes to mind?

# Syncronization & Shared Data Protection

## Mutexes

We can use FreeRTOSmutexes to protect shared data between tasks
