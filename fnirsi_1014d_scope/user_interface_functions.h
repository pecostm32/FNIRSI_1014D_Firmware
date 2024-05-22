//----------------------------------------------------------------------------------------------------------------------------------

#ifndef USER_INTERFACE_FUNCTIONS_H
#define USER_INTERFACE_FUNCTIONS_H

//----------------------------------------------------------------------------------------------------------------------------------

void ui_setup_display_lib(void);

void ui_setup_main_screen(void);

void ui_setup_view_screen(void);
void ui_close_view_screen(void);

void ui_setup_usb_screen(void);

void ui_draw_outline(void);

void ui_display_logo(void);
void ui_display_run_stop_text(void);
void ui_display_move_speed(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Grid and cursor functions
//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_grid(void);
void ui_draw_pointers(void);
void ui_draw_time_cursors(void);
void ui_draw_volt_cursors(void);
void ui_display_selected_text(void);              //Display the text "Selected" near the selected cursor

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_settings(void);
void ui_display_trigger_mode(void);
void ui_display_trigger_channel(void);
void ui_display_trigger_edge(void);
void ui_display_trigger_y_position(void);
void ui_display_trigger_x_position(void);

void ui_display_time_per_division(void);
void ui_display_waiting_triggered_text(void);

void ui_display_channel_settings(PCHANNELSETTINGS settings);

void ui_display_channel_probe(PCHANNELSETTINGS settings);
void ui_display_channel_coupling(PCHANNELSETTINGS settings);
void ui_display_channel_sensitivity(PCHANNELSETTINGS settings);
void ui_display_channel_position(PCHANNELSETTINGS settings);

void ui_display_measurements(void);
void ui_update_measurements(void);

void ui_display_main_menu(void);
void ui_unhighlight_main_menu_item(void);
void ui_highlight_main_menu_item(void);

void ui_display_measurements_menu(void);
void ui_display_measurements_menu_items(uint32 xpos, PCHANNELSETTINGS settings);

void ui_display_channel_menu(PCHANNELSETTINGS settings);
void ui_display_channel_menu_probe_magnification_select(PCHANNELSETTINGS settings);
void ui_display_channel_menu_coupling_select(PCHANNELSETTINGS settings);
void ui_display_channel_menu_fft_on_off_select(PCHANNELSETTINGS settings);

void ui_display_cursor_measurements(void);

void ui_cursor_print_value(char *buffer, int32 value, uint32 scale, char *header, char *sign);

//----------------------------------------------------------------------------------------------------------------------------------
// Functions for displaying the measurement values on the measurements slots
// These functions directly display the value on the screen
//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vmax(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_vmin(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_vavg(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_vrms(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_vpp(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_vp(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_freq(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_cycle(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_time_plus(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_time_min(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_duty_plus(uint32 ypos, PCHANNELSETTINGS settings);
void ui_display_duty_min(uint32 ypos, PCHANNELSETTINGS settings);

void ui_display_duty_cycle(uint32 ypos, PCHANNELSETTINGS settings, uint32 value);
void ui_display_voltage(uint32 ypos, PCHANNELSETTINGS settings, int32 value, uint32 signedvalue);
void ui_print_value(uint32 ypos, int32 value, uint32 scale, char *designator, uint32 signedvalue);
uint32 ui_print_decimal(uint32 xpos, uint32 ypos, int32 value, uint32 decimal);

//----------------------------------------------------------------------------------------------------------------------------------
// Functions for displaying the measurement values on the measurements menu
// These functions setup the text to be displayed in the measurementtext buffer
//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vmax(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_vmin(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_vavg(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_vrms(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_vpp(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_vp(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_freq(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_cycle(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_time_plus(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_time_min(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_duty_plus(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);
void ui_msm_display_duty_min(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings);

void ui_msm_display_voltage(PCHANNELSETTINGS settings, int32 value);
void ui_msm_print_value(char *buffer, int32 value, uint32 scale, char *designator);
char *ui_msm_print_decimal(char *buffer, int32 value, uint32 decimals, uint32 negative);

//----------------------------------------------------------------------------------------------------------------------------------
// File display functions
//----------------------------------------------------------------------------------------------------------------------------------

//These two functions are for save guarding the operational settings when switched to waveform view mode
void ui_save_setup(PSCOPESETTINGS settings);
void ui_restore_setup(PSCOPESETTINGS settings);

//These two functions are for the system settings, preparing for and restoring from file
void ui_prepare_setup_for_file(void);
void ui_restore_setup_from_file(void);

int32 ui_check_waveform_file(void);

void ui_print_file_name(uint32 filenumber);

int32 ui_load_thumbnail_file(void);
int32 ui_save_thumbnail_file(void);

void ui_save_view_item_file(int32 type);

void ui_remove_item_from_thumbnails(uint32 delete);

int32 ui_load_trace_data(void);

int32 ui_load_bitmap_data(void);

void ui_sync_thumbnail_files(void);

void ui_initialize_and_display_thumbnails(void);

void ui_display_thumbnails(void);

void ui_display_thumbnail_data(uint32 xstart, uint32 xend, uint32 ypos, uint32 color, uint8 *buffer);

void ui_create_thumbnail(PTHUMBNAILDATA thumbnaildata);

void ui_thumbnail_set_trace_data(PCHANNELSETTINGS settings, uint8 *buffer);
void ui_thumbnail_calculate_trace_data(int32 xstart, int32 ystart, int32 xend, int32 yend);

void ui_thumbnail_draw_pointer(uint32 xpos, uint32 ypos, uint32 direction, uint32 color);

int32 ui_display_picture_item(void);

void ui_display_selected_signs(void);

void ui_display_file_status_message(int32 msgid, int32 alwayswait);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* USER_INTERFACE_FUNCTIONS_H */

//----------------------------------------------------------------------------------------------------------------------------------
