#include <Arduino.h>
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "ui.h"
#include "screens.h"
#include "structs.h"

// ==========================================================
// PINES ESP32-S3 ES3C28P
// ==========================================================
#define PIN_BACKLIGHT      45

// ==========================================================
// PIN DEL BUZZER (IO21)
// ==========================================================
#define PIN_BUZZER         21  
#define BUZZER_FREQ        2800

// ==========================================================
// PIN ANALÓGICO MICRÓFONO KY-037 (AO en IO2)
// ==========================================================
#define PIN_MIC_ANALOG     2     // Salida AO conectada a IO2
#define UMBRAL_SONIDO     2400   // Umbral seguro sobre el ruido de 2280

// ==========================================================
// VARIABLES GLOBALES
// ==========================================================
String cama_txt = "---";
String solucion_seleccionada = "---";
String volumen_txt = "---";
String tiempo_txt = "---";
String velocidad_txt = "---";

// Control de Caso Activo (1: Oclusión, 2: Aire, 3: Desviación, 4: Fin de Infusión)
int caso_activo = 0; 

// Variables de Control de Aplausos
unsigned long ultimo_tiempo_deteccion = 0;
unsigned long ventana_tiempo = 1800; // 1.8s de silencio para dar margen cómodo al aplaudir
int contador_aplausos = 0;
bool detectando = false;

// ==========================================================
// CONTROL DE BUZZER
// ==========================================================
void sonido_encendido() {
  ledcWriteTone(PIN_BUZZER, BUZZER_FREQ);
}

void sonido_apagado() {
  ledcWriteTone(PIN_BUZZER, 0);
}

// ==========================================================
// VALIDACIONES PRELIMINAR Y DE VELOCIDAD
// ==========================================================
bool validar_datos_main(String solucion, float vol, float tiempo_h) {
  float min_vol = 0, max_vol = 0;
  float min_t_h = 0, max_t_h = 0;

  if (solucion.indexOf("Salina") >= 0) {
    min_vol = 500; max_vol = 1000; min_t_h = 8; max_t_h = 24;
  } else if (solucion.indexOf("Lactato") >= 0 || solucion.indexOf("Ringer") >= 0) {
    min_vol = 500; max_vol = 1000; min_t_h = 2; max_t_h = 8;
  } else if (solucion.indexOf("Dextrosa") >= 0) {
    min_vol = 250; max_vol = 1000; min_t_h = 4; max_t_h = 12;
  } else if (solucion.indexOf("Mixta") >= 0) {
    min_vol = 500; max_vol = 1000; min_t_h = 8; max_t_h = 24;
  } else if (solucion.indexOf("Agua") >= 0 || solucion.indexOf("Destilada") >= 0) {
    min_vol = 50; max_vol = 100; min_t_h = 0.5; max_t_h = 1.0;
  } else {
    return false;
  }

  if (vol < min_vol || vol > max_vol) return false;
  if (tiempo_h < min_t_h || tiempo_h > max_t_h) return false;

  return true;
}

bool validar_velocidad(float vol, float tiempo_h, float vel_ingresada) {
  if (tiempo_h <= 0) return false;
  float vel_calculada = vol / tiempo_h;
  if (abs(vel_calculada - vel_ingresada) > 1.0) {
    return false;
  }
  return true;
}

// ==========================================================
// LÓGICA DE INTERVENCIÓN EN PANTALLA "SOLUCIONES"
// ==========================================================
void procesar_seleccion_intervencion(int opcion_elegida) {
  bool es_correcta = false;
  String txt_caso = "";
  String txt_accion = "";
  String txt_motivo = "";

  if (caso_activo == 1) { // CASO 1: OCLUSIÓN DISTAL
    txt_caso = "Oclusion Distal";
    if (opcion_elegida == 1) {
      es_correcta = true;
      txt_accion = "Verificar cateter venoso";
      txt_motivo = "Despeja el bloqueo o acodamiento directo en la vena del paciente.";
    } else if (opcion_elegida == 2) {
      txt_accion = "Activar purga de la bomba";
      txt_motivo = "La purga es para aire, no para desbloquear la vena.";
    } else if (opcion_elegida == 3) {
      txt_accion = "Comprobar guia de infusion";
      txt_motivo = "Revisar la guia no desobstruye un cateter tapado.";
    } else if (opcion_elegida == 4) {
      txt_accion = "Cambiar a modo KVO (1 mL/h)";
      txt_motivo = "Reducir el flujo no elimina la resistencia mecanica en la via.";
    }
  } 
  else if (caso_activo == 2) { // CASO 2: AIRE EN LA LÍNEA
    txt_caso = "Aire en la Linea";
    if (opcion_elegida == 2) {
      es_correcta = true;
      txt_accion = "Activar purga de la bomba";
      txt_motivo = "Elimina la burbuja del sensor automaticamente sin contaminar la via.";
    } else if (opcion_elegida == 1) {
      txt_accion = "Verificar cateter venoso";
      txt_motivo = "Revisar el brazo del paciente no quita el aire del tubo.";
    } else if (opcion_elegida == 3) {
      txt_accion = "Comprobar guia de infusion";
      txt_motivo = "Verificar la guia no elimina la burbuja atrapada.";
    } else if (opcion_elegida == 4) {
      txt_accion = "Cambiar a modo KVO (1 mL/h)";
      txt_motivo = "Cambiar la velocidad no extrae el aire y mantiene el riesgo de embolia.";
    }
  } 
  else if (caso_activo == 3) { // CASO 3: DESVIACIÓN DE FLUJO
    txt_caso = "Desviacion de Flujo";
    if (opcion_elegida == 3) {
      es_correcta = true;
      txt_accion = "Comprobar guia de infusion";
      txt_motivo = "Asegura que el tubo plastico tenga el diametro adecuado y este bien colocado.";
    } else if (opcion_elegida == 1) {
      txt_accion = "Verificar cateter venoso";
      txt_motivo = "El problema es la velocidad de entrega, no un bloqueo en la vena.";
    } else if (opcion_elegida == 2) {
      txt_accion = "Activar purga de la bomba";
      txt_motivo = "Purgar gasta solucion pero no corrige el calibre del tubo.";
    } else if (opcion_elegida == 4) {
      txt_accion = "Cambiar a modo KVO (1 mL/h)";
      txt_motivo = "Bajar el flujo no resuelve el error de calibracion del volumen.";
    }
  }
  else if (caso_activo == 4) { // CASO 4: FIN DE INFUSIÓN
    txt_caso = "Fin de Infusion";
    if (opcion_elegida == 4) {
      es_correcta = true;
      txt_accion = "Cambiar a modo KVO (1 mL/h)";
      txt_motivo = "Mantiene un flujo minimo continuo para evitar que la vena del paciente se coagule.";
    } else if (opcion_elegida == 1) {
      txt_accion = "Verificar cateter venoso";
      txt_motivo = "La via no esta obstruida; simplemente se termino la solucion prescrita.";
    } else if (opcion_elegida == 2) {
      txt_accion = "Activar purga de la bomba";
      txt_motivo = "No hay aire en el sistema para purgar.";
    } else if (opcion_elegida == 3) {
      txt_accion = "Comprobar guia de infusion";
      txt_motivo = "La guia esta bien colocada; el volumen completo ya fue entregado.";
    }
  }

  // Cargar pantalla según el resultado
  if (es_correcta) {
    if (objects.caso != NULL) lv_label_set_text(objects.caso, txt_caso.c_str());
    if (objects.accion_realizada != NULL) lv_label_set_text(objects.accion_realizada, txt_accion.c_str());
    
    if (objects.motivo_clinico != NULL) {
      lv_label_set_long_mode(objects.motivo_clinico, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(objects.motivo_clinico, 220);
      lv_label_set_text(objects.motivo_clinico, txt_motivo.c_str());
    }
    
    if (objects.bien_ != NULL) lv_scr_load(objects.bien_);
  } else {
    if (objects.accion_error != NULL) lv_label_set_text(objects.accion_error, txt_accion.c_str());
    
    if (objects.motivo_del_error != NULL) {
      lv_label_set_long_mode(objects.motivo_del_error, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(objects.motivo_del_error, 220);
      lv_label_set_text(objects.motivo_del_error, txt_motivo.c_str());
    }
    
    if (objects.mal != NULL) lv_scr_load(objects.mal);
  }
}

// ==========================================================
// EVENTOS DE BOTONES TÁCTILES
// ==========================================================
void evento_boton_dinamico(lv_event_t * e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    if (label != NULL) {
      const char * txt = lv_label_get_text(label);
      if (txt != NULL) {
        String texto = String(txt);
        if (texto.indexOf("cateter") >= 0) procesar_seleccion_intervencion(1);
        else if (texto.indexOf("purga") >= 0) procesar_seleccion_intervencion(2);
        else if (texto.indexOf("guia") >= 0) procesar_seleccion_intervencion(3);
        else if (texto.indexOf("KVO") >= 0) procesar_seleccion_intervencion(4);
      }
    }
  }
}

void evento_ir_a_programar(lv_event_t * e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    if (objects.opciones != NULL) {
      char opcion_elegida[64];
      lv_dropdown_get_selected_str(objects.opciones, opcion_elegida, sizeof(opcion_elegida));
      solucion_seleccionada = String(opcion_elegida);
    }
    if (objects.numero_de_cama != NULL) {
      const char* val = lv_textarea_get_text(objects.numero_de_cama);
      if (val != NULL) cama_txt = String(val);
    }
    if (objects.volumen != NULL) {
      const char* val = lv_textarea_get_text(objects.volumen);
      if (val != NULL) volumen_txt = String(val);
    }
    if (objects.tiemto_total != NULL) {
      const char* val = lv_textarea_get_text(objects.tiemto_total);
      if (val != NULL) tiempo_txt = String(val);
    }

    float vol_num = volumen_txt.toFloat();
    float tiempo_num = tiempo_txt.toFloat();

    if (validar_datos_main(solucion_seleccionada, vol_num, tiempo_num)) {
      if (objects.velocidad != NULL) lv_scr_load(objects.velocidad);
    } else {
      if (objects.error != NULL) lv_scr_load(objects.error);
    }
  }
}

void evento_boton_iniciar(lv_event_t * e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    if (objects.indique_el_valor_ != NULL) {
      const char* val = lv_textarea_get_text(objects.indique_el_valor_);
      if (val != NULL) velocidad_txt = String(val);
    }
    float vol_num = volumen_txt.toFloat();
    float tiempo_num = tiempo_txt.toFloat();
    float vel_num = velocidad_txt.toFloat();

    if (validar_velocidad(vol_num, tiempo_num, vel_num)) {
      contador_aplausos = 0;
      detectando = false;

      if (objects.cargando != NULL) lv_scr_load(objects.cargando);
    } else {
      if (objects.error != NULL) lv_scr_load(objects.error);
    }
  }
}

// ==========================================================
// REVISIÓN CON TEMPORIZACIÓN DE RITMO HUMANO REAL
// ==========================================================
void revisar_mic_analogo() {
  if (objects.cargando == NULL || lv_scr_act() != objects.cargando) {
    contador_aplausos = 0;
    detectando = false;
    return;
  }

  // Muestreo rápido de picos
  int lectura_maxima = 0;
  for (int i = 0; i < 40; i++) {
    int val = analogRead(PIN_MIC_ANALOG);
    if (val > lectura_maxima) {
      lectura_maxima = val;
    }
  }

  // Anti-rebote aumentado a 220ms para filtrar ecos y rebotes
  if (lectura_maxima > UMBRAL_SONIDO && (::millis() - ultimo_tiempo_deteccion > 220)) {
    contador_aplausos++;
    ultimo_tiempo_deteccion = ::millis();
    detectando = true;

    Serial.print("-> Aplauso #");
    Serial.print(contador_aplausos);
    Serial.print(" detectado (Pico: ");
    Serial.print(lectura_maxima);
    Serial.println(")");
  }

  // Espera 1.8 segundos de silencio antes de validar el total acumulado
  if (detectando && (::millis() - ultimo_tiempo_deteccion > ventana_tiempo)) {
    Serial.print("\n=== FIN DE SECUENCIA: Total de aplausos contados = ");
    Serial.print(contador_aplausos);
    Serial.println(" ===");

    String mensaje_alerta = "";

    switch (contador_aplausos) {
      case 1:
        caso_activo = 1;
        mensaje_alerta = "Alarma de Oclusion Distal";
        break;
      case 2:
        caso_activo = 2;
        mensaje_alerta = "Alarma de Aire en la Linea";
        break;
      case 3:
        caso_activo = 3;
        mensaje_alerta = "Alarma Desviacion de Flujo";
        break;
      case 4:
        caso_activo = 4;
        mensaje_alerta = "Alarma Fin de Infusion";
        break;
      default:
        Serial.print(">> Conteo fuera de rango (1-4): ");
        Serial.println(contador_aplausos);
        break;
    }

    if (mensaje_alerta.length() > 0) {
      Serial.print("Cambiando pantalla a: ");
      Serial.println(mensaje_alerta);

      if (objects.texto_de_alarma != NULL) {
        lv_label_set_long_mode(objects.texto_de_alarma, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(objects.texto_de_alarma, 220);
        lv_obj_set_style_text_align(objects.texto_de_alarma, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(objects.texto_de_alarma, mensaje_alerta.c_str());
      }

      if (objects.alarma != NULL) {
        lv_scr_load(objects.alarma);
      }
    }

    contador_aplausos = 0;
    detectando = false;
  }
}

void asignar_eventos_recursivo(lv_obj_t * parent) {
  if (parent == NULL) return;
  uint32_t cnt = lv_obj_get_child_cnt(parent);
  for (uint32_t i = 0; i < cnt; i++) {
    lv_obj_t * child = lv_obj_get_child(parent, i);
    if (lv_obj_check_type(child, &lv_btn_class)) {
      lv_obj_add_event_cb(child, evento_boton_dinamico, LV_EVENT_CLICKED, NULL);
    } else {
      asignar_eventos_recursivo(child);
    }
  }
}

// ==========================================================
// SETUP
// ==========================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  ledcAttach(PIN_BUZZER, BUZZER_FREQ, 8);
  sonido_apagado();

  pinMode(PIN_MIC_ANALOG, INPUT);

  LCD_Init();
  Lvgl_Init();
  ui_init();

  Serial.println("\n-------------------------------------------");
  Serial.println("  SISTEMA DE INFUSIÓN LISTO");
  Serial.println("-------------------------------------------");
}

// ==========================================================
// LOOP
// ==========================================================
void loop() {
  Lvgl_Loop();
  ui_tick();

  static bool eventos_registrados = false;
  if (!eventos_registrados) {
    if (objects.ir_a_programar_ != NULL) {
      lv_obj_add_event_cb(objects.ir_a_programar_, evento_ir_a_programar, LV_EVENT_CLICKED, NULL);
    }
    if (objects.boton != NULL) {
      lv_obj_add_event_cb(objects.boton, evento_boton_iniciar, LV_EVENT_CLICKED, NULL);
    }

    if (objects.soluciones_ != NULL) {
      asignar_eventos_recursivo(objects.soluciones_);
    }

    eventos_registrados = true;
  }

  // Actualizar etiquetas en pantalla
  if (objects.label_cama != NULL) lv_label_set_text(objects.label_cama, cama_txt.c_str());
  if (objects.label_solucion != NULL) lv_label_set_text(objects.label_solucion, solucion_seleccionada.c_str());
  if (objects.label_volumen != NULL) { String tv = volumen_txt + " mL"; lv_label_set_text(objects.label_volumen, tv.c_str()); }
  if (objects.label_tiempo != NULL) { String tt = tiempo_txt + " Hrs"; lv_label_set_text(objects.label_tiempo, tt.c_str()); }
  if (objects.label_velocidad != NULL) { String tvel = velocidad_txt + " mL/h"; lv_label_set_text(objects.label_velocidad, tvel.c_str()); }

  // Monitoreo continuo del micrófono en la pantalla de carga
  revisar_mic_analogo();

  // Control del Buzzer en Alarma o Error
  static unsigned long ultimo_cambio_sonido = 0;
  static bool estado_buzzer = false;

  bool es_pantalla_error = (objects.error != NULL && lv_scr_act() == objects.error);
  bool es_pantalla_alarma = (objects.alarma != NULL && lv_scr_act() == objects.alarma);

  if (es_pantalla_error || es_pantalla_alarma) {
    if (::millis() - ultimo_cambio_sonido >= 300) {
      ultimo_cambio_sonido = ::millis();
      estado_buzzer = !estado_buzzer;

      if (estado_buzzer) {
        sonido_encendido();
      } else {
        sonido_apagado();
      }

      if (es_pantalla_error && objects.triangulo_error != NULL) {
        if (estado_buzzer) {
          lv_obj_clear_flag(objects.triangulo_error, LV_OBJ_FLAG_HIDDEN);
        } else {
          lv_obj_add_flag(objects.triangulo_error, LV_OBJ_FLAG_HIDDEN);
        }
      }
    }
  } else {
    if (estado_buzzer) {
      estado_buzzer = false;
      sonido_apagado();
    }
  }

  // Animación de la barra de suero
  static unsigned long ultimo_tiempo_suero = 0;
  static int nivel_suero = 100;

  if (::millis() - ultimo_tiempo_suero >= 400) {
    ultimo_tiempo_suero = ::millis();

    if (objects.cargando != NULL && lv_scr_act() == objects.cargando) {
      nivel_suero--;
      if (nivel_suero < 0) nivel_suero = 100;

      if (objects.suero_bar != NULL) {
        lv_bar_set_value(objects.suero_bar, nivel_suero, LV_ANIM_ON);
      }
    } else {
      nivel_suero = 100;
    }
  }
}