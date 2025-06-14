// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////


// SCREEN: ui_MainScreen
void ui_MainScreen_screen_init(void);
lv_obj_t * ui_MainScreen;
lv_obj_t * ui_temperature;
lv_obj_t * ui_NumTemperature;
lv_obj_t * ui_labCelsius;
lv_obj_t * ui_Button1;
lv_obj_t * ui_ButtonStart;
lv_obj_t * ui_labBattery;
lv_obj_t * ui_labPressureBottom;
lv_obj_t * ui_NumPressureBottom;
lv_obj_t * ui_labUserTemp;
lv_obj_t * ui_NumUserTemp;
lv_obj_t * ui_searching;
lv_obj_t * ui_connected;
lv_obj_t * ui_default;
lv_obj_t * ui_batterylevel;
lv_obj_t * ui_mainpower;
lv_obj_t * ui_pourover;
lv_obj_t * ui_tea;
lv_obj_t * ui_barista;
// CUSTOM VARIABLES
lv_obj_t * uic_MainScreen;
lv_obj_t * uic_default;

// EVENTS
lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_MainScreen_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_MainScreen);
}
