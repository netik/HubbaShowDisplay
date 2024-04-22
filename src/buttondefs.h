
// List of features of the AceButton library that we want to examine.
#define FEATURE_BASELINE 0
#define FEATURE_BASELINE_DIGITAL_READ 1
#define FEATURE_BUTTON_CONFIG 2
#define FEATURE_BUTTON_CONFIG_FAST1 3
#define FEATURE_BUTTON_CONFIG_FAST2 4
#define FEATURE_BUTTON_CONFIG_FAST3 5
#define FEATURE_ENCODED_4TO2_BUTTON_CONFIG 6
#define FEATURE_ENCODED_8TO3_BUTTON_CONFIG 7
#define FEATURE_ENCODED_BUTTON_CONFIG 8
#define FEATURE_LADDER_BUTTON_CONFIG 9

// Select one of the FEATURE_* parameters and compile. Then look at the flash
// and RAM usage, compared to FEATURE_BASELINE usage to determine how much
// flash and RAM is consumed by the selected feature.
// NOTE: This line is modified by a 'sed' script in collect.sh. Be careful
// when modifying its format.
#define FEATURE 1