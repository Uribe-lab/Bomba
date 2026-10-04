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
    SCREEN_ID_SOLUCIONES_ = 6,
    SCREEN_ID_BIEN_ = 7,
    SCREEN_ID_MAL = 8,
    _SCREEN_ID_LAST = 8
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *velocidad;
    lv_obj_t *error;
    lv_obj_t *cargando;
    lv_obj_t *alarma;
    lv_obj_t *soluciones_;
    lv_obj_t *bien_;
    lv_obj_t *mal;
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
    lv_obj_t *texto_de_alarma;
    lv_obj_t *obj9;
    lv_obj_t *alarma_imagen;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *caso;
    lv_obj_t *accion_realizada;
    lv_obj_t *motivo_clinico;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *accion_error;
    lv_obj_t *motivo_del_error;
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

void create_screen_soluciones_();
void tick_screen_soluciones_();

void create_screen_bien_();
void tick_screen_bien_();

void create_screen_mal();
void tick_screen_mal();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/