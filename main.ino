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
#define AUDIO_ENABLE_PIN   1
#define AUDIO_DATA_PIN     6

// ==========================================================
// CONFIGURACIÓN DE AUDIO
// ==========================================================

// Probamos 2000 Hz.
// Si 2400 Hz sonaba más fuerte, después volvemos a 2400.
#define AUDIO_FREQUENCY    2000

// ==========================================================
// VARIABLES GLOBALES
// ==========================================================

String cama_txt = "---";
String solucion_seleccionada = "---";
String volumen_txt = "---";
String tiempo_txt = "---";
String velocidad_txt = "---";

// ==========================================================
// AUDIO
// ==========================================================

void sonido_encendido() {
  // 128 = 50% duty
  // Es la máxima excursión AC que podemos obtener
  // usando esta señal PWM de 3.3 V.
  ledcWrite(AUDIO_DATA_PIN, 128);
}

void sonido_apagado() {
  ledcWrite(AUDIO_DATA_PIN, 0);
}

// ==========================================================
// 1. VALIDACIÓN PRELIMINAR
// ==========================================================

bool validar_datos_main(String solucion, float vol, float tiempo_h) {

  float min_vol = 0;
  float max_vol = 0;

  float min_t_h = 0;
  float max_t_h = 0;

  Serial.println("\n--- VALIDANDO DATOS EN MAIN ---");

  Serial.print("Solución: '");
  Serial.print(solucion);
  Serial.println("'");

  Serial.print("Volumen: ");
  Serial.print(vol);
  Serial.println(" mL");

  Serial.print("Tiempo: ");
  Serial.print(tiempo_h);
  Serial.println(" hrs");

  // --------------------------------------------------------
  // SALINA
  // --------------------------------------------------------

  if (solucion.indexOf("Salina") >= 0) {

    min_vol = 500;
    max_vol = 1000;

    min_t_h = 8;
    max_t_h = 24;
  }

  // --------------------------------------------------------
  // LACTATO / RINGER
  // --------------------------------------------------------

  else if (
    solucion.indexOf("Lactato") >= 0 ||
    solucion.indexOf("Ringer") >= 0
  ) {

    min_vol = 500;
    max_vol = 1000;

    min_t_h = 2;
    max_t_h = 8;
  }

  // --------------------------------------------------------
  // DEXTROSA
  // --------------------------------------------------------

  else if (
    solucion.indexOf("Dextrosa") >= 0
  ) {

    min_vol = 250;
    max_vol = 1000;

    min_t_h = 4;
    max_t_h = 12;
  }

  // --------------------------------------------------------
  // MIXTA
  // --------------------------------------------------------

  else if (
    solucion.indexOf("Mixta") >= 0
  ) {

    min_vol = 500;
    max_vol = 1000;

    min_t_h = 8;
    max_t_h = 24;
  }

  // --------------------------------------------------------
  // AGUA / DESTILADA
  // --------------------------------------------------------

  else if (
    solucion.indexOf("Agua") >= 0 ||
    solucion.indexOf("Destilada") >= 0
  ) {

    min_vol = 50;
    max_vol = 100;

    min_t_h = 0.5;
    max_t_h = 1.0;
  }

  // --------------------------------------------------------
  // SOLUCIÓN NO RECONOCIDA
  // --------------------------------------------------------

  else {

    Serial.println(
      "--> ERROR EN MAIN: Solución no reconocida"
    );

    return false;
  }

  // --------------------------------------------------------
  // VOLUMEN
  // --------------------------------------------------------

  if (
    vol < min_vol ||
    vol > max_vol
  ) {

    Serial.println(
      "--> ERROR EN MAIN: Volumen fuera de rango permisible"
    );

    return false;
  }

  // --------------------------------------------------------
  // TIEMPO
  // --------------------------------------------------------

  if (
    tiempo_h < min_t_h ||
    tiempo_h > max_t_h
  ) {

    Serial.println(
      "--> ERROR EN MAIN: Tiempo fuera de rango permisible"
    );

    return false;
  }

  Serial.println(
    "--> MAIN CORRECTO: Pasa a la pantalla Velocidad"
  );

  return true;
}

// ==========================================================
// 2. VALIDACIÓN FINAL
// ==========================================================

bool validar_velocidad(
  float vol,
  float tiempo_h,
  float vel_ingresada
) {

  if (tiempo_h <= 0)
    return false;

  float vel_calculada =
    vol / tiempo_h;

  Serial.println("\n--- VALIDANDO VELOCIDAD ---");

  Serial.print(
    "Velocidad calculada: "
  );

  Serial.print(
    vel_calculada
  );

  Serial.print(
    " vs Ingresada: "
  );

  Serial.println(
    vel_ingresada
  );

  if (
    abs(
      vel_calculada -
      vel_ingresada
    ) > 1.0
  ) {

    Serial.println(
      "--> ERROR: La velocidad no coincide con la fórmula Vol/Tiempo"
    );

    return false;
  }

  Serial.println(
    "--> VELOCIDAD CORRECTA: Procede a la infusión"
  );

  return true;
}

// ==========================================================
// EVENTO: IR A PROGRAMAR
// ==========================================================

void evento_ir_a_programar(lv_event_t * e) {

  if (
    lv_event_get_code(e) ==
    LV_EVENT_CLICKED
  ) {

    // ------------------------------------------------------
    // SOLUCIÓN
    // ------------------------------------------------------

    if (
      objects.opciones != NULL
    ) {

      char opcion_elegida[64];

      lv_dropdown_get_selected_str(
        objects.opciones,
        opcion_elegida,
        sizeof(opcion_elegida)
      );

      solucion_seleccionada =
        String(opcion_elegida);
    }

    // ------------------------------------------------------
    // CAMA
    // ------------------------------------------------------

    if (
      objects.numero_de_cama != NULL
    ) {

      const char* val =
        lv_textarea_get_text(
          objects.numero_de_cama
        );

      if (val != NULL)
        cama_txt = String(val);
    }

    // ------------------------------------------------------
    // VOLUMEN
    // ------------------------------------------------------

    if (
      objects.volumen != NULL
    ) {

      const char* val =
        lv_textarea_get_text(
          objects.volumen
        );

      if (val != NULL)
        volumen_txt = String(val);
    }

    // ------------------------------------------------------
    // TIEMPO
    // ------------------------------------------------------

    if (
      objects.tiemto_total != NULL
    ) {

      const char* val =
        lv_textarea_get_text(
          objects.tiemto_total
        );

      if (val != NULL)
        tiempo_txt = String(val);
    }

    float vol_num =
      volumen_txt.toFloat();

    float tiempo_num =
      tiempo_txt.toFloat();

    bool main_ok =
      validar_datos_main(
        solucion_seleccionada,
        vol_num,
        tiempo_num
      );

    if (main_ok) {

      if (
        objects.velocidad != NULL
      ) {

        lv_scr_load(
          objects.velocidad
        );
      }

    } else {

      if (
        objects.error != NULL
      ) {

        lv_scr_load(
          objects.error
        );
      }
    }
  }
}

// ==========================================================
// EVENTO: BOTÓN INICIAR
// ==========================================================

void evento_boton_iniciar(lv_event_t * e) {

  if (
    lv_event_get_code(e) ==
    LV_EVENT_CLICKED
  ) {

    if (
      objects.indique_el_valor_ != NULL
    ) {

      const char* val =
        lv_textarea_get_text(
          objects.indique_el_valor_
        );

      if (val != NULL)
        velocidad_txt =
          String(val);
    }

    float vol_num =
      volumen_txt.toFloat();

    float tiempo_num =
      tiempo_txt.toFloat();

    float vel_num =
      velocidad_txt.toFloat();

    bool vel_ok =
      validar_velocidad(
        vol_num,
        tiempo_num,
        vel_num
      );

    if (vel_ok) {

      if (
        objects.cargando != NULL
      ) {

        lv_scr_load(
          objects.cargando
        );
      }

    } else {

      if (
        objects.error != NULL
      ) {

        lv_scr_load(
          objects.error
        );
      }
    }
  }
}

// ==========================================================
// SETUP
// ==========================================================

void setup() {

  Serial.begin(115200);

  // ========================================================
  // RETROILUMINACIÓN
  // ========================================================

  pinMode(
    PIN_BACKLIGHT,
    OUTPUT
  );

  digitalWrite(
    PIN_BACKLIGHT,
    HIGH
  );

  // ========================================================
  // PANTALLA
  // ========================================================

  LCD_Init();

  Lvgl_Init();

  ui_init();

  // ========================================================
  // AMPLIFICADOR FM8002E
  // ========================================================

  pinMode(
    AUDIO_ENABLE_PIN,
    OUTPUT
  );

  // Primero deshabilitado
  digitalWrite(
    AUDIO_ENABLE_PIN,
    HIGH
  );

  delay(20);

  // LOW = AMPLIFICADOR ENCENDIDO
  digitalWrite(
    AUDIO_ENABLE_PIN,
    LOW
  );

  delay(50);

  // ========================================================
  // PWM DE AUDIO
  // ========================================================

  ledcAttach(
    AUDIO_DATA_PIN,
    AUDIO_FREQUENCY,
    8
  );

  // Comenzar en silencio
  ledcWrite(
    AUDIO_DATA_PIN,
    0
  );

  Serial.println(
    "===================================="
  );

  Serial.println(
    "SISTEMA DE BOMBA DE INFUSION"
  );

  Serial.print(
    "Frecuencia audio: "
  );

  Serial.print(
    AUDIO_FREQUENCY
  );

  Serial.println(
    " Hz"
  );

  Serial.println(
    "Amplificador FM8002E: ENCENDIDO"
  );

  Serial.println(
    "===================================="
  );
}

// ==========================================================
// LOOP
// ==========================================================

void loop() {

  // ========================================================
  // LVGL
  // ========================================================

  Lvgl_Loop();

  ui_tick();

  // ========================================================
  // REGISTRAR EVENTOS
  // ========================================================

  static bool eventos_registrados = false;

  if (!eventos_registrados) {

    if (
      objects.ir_a_programar_ != NULL
    ) {

      lv_obj_add_event_cb(
        objects.ir_a_programar_,
        evento_ir_a_programar,
        LV_EVENT_CLICKED,
        NULL
      );
    }

    if (
      objects.boton != NULL
    ) {

      lv_obj_add_event_cb(
        objects.boton,
        evento_boton_iniciar,
        LV_EVENT_CLICKED,
        NULL
      );
    }

    eventos_registrados = true;
  }

  // ========================================================
  // LABEL CAMA
  // ========================================================

  if (
    objects.label_cama != NULL
  ) {

    lv_label_set_text(
      objects.label_cama,
      cama_txt.c_str()
    );
  }

  // ========================================================
  // LABEL SOLUCIÓN
  // ========================================================

  if (
    objects.label_solucion != NULL
  ) {

    lv_label_set_text(
      objects.label_solucion,
      solucion_seleccionada.c_str()
    );
  }

  // ========================================================
  // LABEL VOLUMEN
  // ========================================================

  if (
    objects.label_volumen != NULL
  ) {

    String texto_volumen =
      volumen_txt + " mL";

    lv_label_set_text(
      objects.label_volumen,
      texto_volumen.c_str()
    );
  }

  // ========================================================
  // LABEL TIEMPO
  // ========================================================

  if (
    objects.label_tiempo != NULL
  ) {

    String texto_tiempo =
      tiempo_txt + " Hrs";

    lv_label_set_text(
      objects.label_tiempo,
      texto_tiempo.c_str()
    );
  }

  // ========================================================
  // LABEL VELOCIDAD
  // ========================================================

  if (
    objects.label_velocidad != NULL
  ) {

    String texto_velocidad =
      velocidad_txt + " mL/h";

    lv_label_set_text(
      objects.label_velocidad,
      texto_velocidad.c_str()
    );
  }

  // ========================================================
  // PARPADEO DEL ERROR
  // ========================================================

  static unsigned long
    ultimo_tiempo_triangulo = 0;

  static bool esta_oculto = false;

  if (
    ::millis() -
    ultimo_tiempo_triangulo >= 400
  ) {

    ultimo_tiempo_triangulo =
      ::millis();

    // ------------------------------------------------------
    // ESTAMOS EN ERROR
    // ------------------------------------------------------

    if (
      objects.error != NULL &&
      lv_scr_act() == objects.error
    ) {

      if (
        objects.triangulo_error != NULL
      ) {

        // Cambiar estado
        esta_oculto =
          !esta_oculto;

        // --------------------------------------------------
        // TRIÁNGULO OCULTO
        // --------------------------------------------------

        if (esta_oculto) {

          lv_obj_add_flag(
            objects.triangulo_error,
            LV_OBJ_FLAG_HIDDEN
          );

          sonido_apagado();
        }

        // --------------------------------------------------
        // TRIÁNGULO VISIBLE
        // --------------------------------------------------

        else {

          lv_obj_clear_flag(
            objects.triangulo_error,
            LV_OBJ_FLAG_HIDDEN
          );

          sonido_encendido();
        }
      }

    }

    // ------------------------------------------------------
    // NO ESTAMOS EN ERROR
    // ------------------------------------------------------

    else {

      esta_oculto = false;

      sonido_apagado();
    }
  }

  // ========================================================
  // BARRA DE SUERO
  // ========================================================

  static unsigned long
    ultimo_tiempo_suero = 0;

  static int nivel_suero = 0;

  if (
    ::millis() -
    ultimo_tiempo_suero >= 400
  ) {

    ultimo_tiempo_suero =
      ::millis();

    if (
      objects.cargando != NULL &&
      lv_scr_act() == objects.cargando
    ) {

      nivel_suero =
        (nivel_suero + 1) % 101;

      if (
        objects.suero_bar != NULL
      ) {

        lv_bar_set_value(
          objects.suero_bar,
          nivel_suero,
          LV_ANIM_ON
        );
      }

    } else {

      nivel_suero = 0;
    }
  }

  // ========================================================
  // LOOP RÁPIDO
  // ========================================================

  delay(5);
}