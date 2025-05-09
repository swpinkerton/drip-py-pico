// Pins

// X Motor
#define X_DIR_PIN       0
#define X_STEP_PIN      1
#define X_ENABLE_PIN    5
#define X_ENDSTOP_PIN   4

// Y Motor
#define Y_DIR_PIN       6
#define Y_STEP_PIN      7
#define Y_ENABLE_PIN    14
#define Y_ENDSTOP_PIN   15

// Dropper Motor
#define Z_HOSE_DIR_PIN        19
#define Z_HOSE_STEP_PIN       19
#define Z_HOSE_ENABLE_PIN     19
#define Z_HOSE_ENDSTOP_PIN    19

// Electrodes Motor
#define Z_ELECTRODE_DIR_PIN        19
#define Z_ELECTRODE_STEP_PIN       19
#define Z_ELECTRODE_ENABLE_PIN     19
#define Z_ELECTRODE_ENDSTOP_PIN    19

// Global Settings
#define MICROSTEPS                  32

// Gantry Settings
#define GANTRY_THREAD_PITCH_MM          0.4
#define GANTRY_STEPS_PER_REVOLUTION     24 * MICROSTEPS
#define GANTRY_ACCELERATION_RPM         390.0
#define GANTRY_MAX_RPM                  1000.0
#define GANTRY_ZEROING_RPM              400.0

// Dropper settings
#define Z_ACCELERATION_RPM              200.0
#define Z_MAX_RPM                       200.0
#define Z_STEPS_PER_REVOLUTION          20 * MICROSTEPS

// Sizing Settings
#define WELL_START_LOCATION_X       10 // mm
#define WELL_START_LOCATION_Y       10 // mm
#define WELL_SPACING                9  // mm
#define WELL_HOME_X                 10 // mm
#define WELL_HOME_Y                 10 // mm