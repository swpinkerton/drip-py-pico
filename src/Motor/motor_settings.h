#define USING_BREADBOARD

// GPIO Pins
// NUmber is the GPIO number not the pin number
#ifdef USING_BREADBOARD
// X Motor
#define X_DIR_PIN       0
#define X_STEP_PIN      1
#define X_ENABLE_PIN    2
#define X_ENDSTOP_PIN   3

// Y Motor
#define Y_DIR_PIN       6
#define Y_STEP_PIN      7
#define Y_ENABLE_PIN    8
#define Y_ENDSTOP_PIN   9

// Dropper Motor
#define Z_HOSE_DIR_PIN        10
#define Z_HOSE_STEP_PIN       11
#define Z_HOSE_ENABLE_PIN     12
#define Z_HOSE_ENDSTOP_PIN    13

// Electrodes Motor
#define Z_ELECTRODE_DIR_PIN        18
#define Z_ELECTRODE_STEP_PIN       19
#define Z_ELECTRODE_ENABLE_PIN     20
#define Z_ELECTRODE_ENDSTOP_PIN    21

#else

#endif

// Global Settings
#define MICROSTEPS                  32

// Gantry Settings
#define GANTRY_THREAD_PITCH_MM          0.4
#define GANTRY_STEPS_PER_REVOLUTION     24 * MICROSTEPS
#define GANTRY_ACCELERATION_RPM         390.0
#define GANTRY_MAX_RPM                  1000.0
// #define GANTRY_MAX_RPM                  30.0
#define GANTRY_ZEROING_RPM              400.0

// Dropper settings
#define Z_ACCELERATION_RPM              200.0
#define Z_MAX_RPM                       30.0
#define Z_STEPS_PER_REVOLUTION          20 * MICROSTEPS
#define HOSE_OFFSET_X                   0
#define HOSE_OFFSET_Y                   0
#define ELECTRODES_OFFSET_X             0
#define ELECTRODES_OFFSET_Y             10

// Sizing Settings
#define WELL_START_LOCATION_X       10 // mm
#define WELL_START_LOCATION_Y       10 // mm
#define WELL_SPACING                9  // mm
#define WELL_HOME_X                 10 // mm
#define WELL_HOME_Y                 10 // mm