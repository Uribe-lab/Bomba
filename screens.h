#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_VELOCIDAD = 2,
    SCREEN_ID_ERROR = 3,
    SCREEN_ID_CARGANDO = 4,
    SCREEN_ID_ALARMA = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *velocidad;
    lv_obj_t *error;
    lv_obj_t *cargando;
    lv_obj_t *alarma;
    lv_obj_t *ord;
    lv_obj_t *numero_de_cama;
    lv_obj_t *opciones;
    lv_obj_t *volumen;
    lv_obj_t *tiemto_total;
    lv_obj_t *ir_a_programar_;
    lv_obj_t *teclado1;
    lv_obj_t *teclado_2;
    lv_obj_t *boton;
    lv_obj_t *indique_el_valor_;
    lv_obj_t *teclado_3;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *triangulo_error;
    lv_obj_t *obj3;
    lv_obj_t *suero_bar;
    lv_obj_t *obj4;
    lv_obj_t *label_cama;
    lv_obj_t *label_volumen;
    lv_obj_t *label_tiempo;
    lv_obj_t *label_velocidad;
    lv_obj_t *label_solucion;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_velocidad();
void tick_screen_velocidad();

void create_screen_error();
void tick_screen_error();

void create_screen_cargando();
void tick_screen_cargando();

void create_screen_alarma();
void tick_screen_alarma();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/