//----------------------------------------------------------------------------------------------------------------------------------

#ifndef SCOPE_FUNCTIONS_H
#define SCOPE_FUNCTIONS_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "fnirsi_1014d_scope.h"
#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data processing functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquire_trace_data(void);

void scope_process_trigger(uint32 count);

uint32 scope_do_baseline_calibration(void);
uint32 scope_do_channel_calibration(void);

void scope_do_auto_setup(void);

uint32 scope_check_channel_range(PCHANNELSETTINGS settings);

void scope_calculate_trigger_vertical_position(void);

void scope_set_50_percent_trigger(void);

void scope_calculate_sample_range_properties(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data display functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_trace_data(void);

int32 scope_get_x_sample(PCHANNELSETTINGS settings, int32 index);
int32 scope_get_y_sample(PCHANNELSETTINGS settings, int32 index);

void scope_display_channel_trace(PCHANNELSETTINGS settings);

//----------------------------------------------------------------------------------------------------------------------------------
// Configuration data functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_load_configuration_data(void);
void scope_save_configuration_data(void);

void scope_reset_config_data(void);
void scope_save_config_data(void);
void scope_restore_config_data(void);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* SCOPE_FUNCTIONS_H */

//----------------------------------------------------------------------------------------------------------------------------------
