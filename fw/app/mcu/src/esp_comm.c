/**
 * @file    esp_comm.c
 * @brief   Puente de comunicación STM32 ↔ ESP32 via UART4 (JSON por línea).
 *
 * @details
 *  CommTask bloquea en osThreadFlagsWait(COMM_UART_RX_FLAG).
 *  Cuando UART4 IRQ completa una línea, notifica a CommTask via ese flag.
 *  CommTask parsea el JSON con cJSON (o un parser mínimo propio) y actúa:
 *    - "dispense" → postea EVT_BUTTON a la queue de SM_Task
 *    - "tare"     → ejecuta la tara (placeholder para STM32)
 *    - "set_schedules" → actualiza el schedule dinámico interno
 *
 * @author  Matías Durante
 * @version 1.0
 * @date    2025
 */

#include "esp_comm.h"
#include "dev_uart.h"
#include "dev_hx711.h"
#include "app_sm.h"
#include "ds3231.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os2.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* =========================================================================== */
/*                          VARIABLES PRIVADAS                                 */
/* =========================================================================== */

/** @brief Handle de CommTask — usado por dev_uart para notificar. */
static osThreadId_t s_comm_task_handle = NULL;

/** @brief Schedule dinámico recibido desde la App. */
static Comm_Schedule s_schedules[COMM_MAX_SCHEDULES];
static uint8_t       s_schedule_count = 0;

/** @brief Mutex para proteger s_schedules (lectura desde SM_Task). */
static osMutexId_t   s_schedule_mutex = NULL;

/* =========================================================================== */
/*                      PARSER JSON MÍNIMO (sin librería externa)              */
/* =========================================================================== */
/*
 * Usamos un parser de strings simple para evitar dependencias externas.
 * Busca el valor de una clave en un JSON plano (sin anidamiento complejo).
 * Para el JSON anidado de "schedules" usamos un parser de array propio.
 */

/**
 * @brief  Busca el valor string de una clave en un JSON plano.
 * @param  json  Buffer JSON terminado en '\0'.
 * @param  key   Clave a buscar (sin comillas).
 * @param  out   Buffer de salida.
 * @param  outSz Tamaño del buffer de salida.
 * @return true si encontró la clave.
 */
static bool json_get_string(const char *json, const char *key,
                             char *out, uint16_t outSz)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *p = strstr(json, search);
    if (!p) return false;

    p += strlen(search);
    while (*p == ' ' || *p == ':') p++;

    if (*p == '"') {
        p++;
        uint16_t i = 0;
        while (*p && *p != '"' && i < outSz - 1) {
            out[i++] = *p++;
        }
        out[i] = '\0';
        return true;
    }
    return false;
}

/**
 * @brief  Busca el valor entero de una clave en un JSON plano.
 * @param  json  Buffer JSON.
 * @param  key   Clave a buscar.
 * @param  out   Valor de salida.
 * @return true si encontró la clave.
 */
static bool json_get_int(const char *json, const char *key, int *out)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *p = strstr(json, search);
    if (!p) return false;

    p += strlen(search);
    while (*p == ' ' || *p == ':') p++;

    if (*p == '-' || (*p >= '0' && *p <= '9')) {
        *out = atoi(p);
        return true;
    }
    return false;
}

/**
 * @brief  Busca el valor booleano de una clave en un JSON plano.
 */
static bool json_get_bool(const char *json, const char *key, bool *out)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *p = strstr(json, search);
    if (!p) return false;

    p += strlen(search);
    while (*p == ' ' || *p == ':') p++;

    if (strncmp(p, "true", 4) == 0)       { *out = true;  return true; }
    if (strncmp(p, "false", 5) == 0)      { *out = false; return true; }
    return false;
}

/* =========================================================================== */
/*                      PROCESAMIENTO DE COMANDOS                              */
/* =========================================================================== */

/**
 * @brief  Procesa el comando "dispense" — postea evento a SM_Task.
 */
static void handle_dispense(const char *json)
{
    int amount = 50;
    json_get_int(json, "amount", &amount);

    /* Postear EVT_BUTTON a la queue de SM_Task para disparar dispensación */
    Dispenser_PostButtonEvent();

    uint8_t buf[128];
    snprintf((char *)buf, sizeof(buf),
             "[COMM] Dispensar %d g — evento enviado a SM_Task\r\n", amount);
    uartSendString(buf);
}

/**
 * @brief  Procesa el comando "tare" — tara ambas galgas HX711.
 */
static void handle_tare(void)
{
    uartSendString((uint8_t *)"[COMM] Tara iniciada...\r\n");

    HX711_Status st = HX711_TareAll();

    if (st == HX711_OK) {
        /* Leer después de tarar para confirmar que da ~0 */
        float tank = 0.0f, plate = 0.0f;
        HX711_ReadGrams(HX711_CH_TANK,  &tank);
        HX711_ReadGrams(HX711_CH_PLATE, &plate);

        uint8_t buf[128];
        int tank_int  = (int)tank;
        int tank_dec  = (int)((tank  - tank_int)  * 100);
        int plate_int = (int)plate;
        int plate_dec = (int)((plate - plate_int) * 10);
        snprintf((char *)buf, sizeof(buf),
                 "[COMM] Tara OK — Tanque: %d.%02dg  Plato: %d.%01dg\r\n",
                 tank_int, tank_dec, plate_int, plate_dec);
        uartSendString(buf);

        Comm_SendTareDone();
    } else {
        uartSendString((uint8_t *)"[COMM] Error en tara — HX711 no responde.\r\n");
        Comm_SendTareDone();
    }
}

/**
 * @brief  Procesa el comando "set_schedules" — parsea el array de horarios.
 */
static void handle_set_schedules(const char *json)
{
    /* Buscar el inicio del array "schedules" */
    const char *arr = strstr(json, "\"schedules\"");
    if (!arr) return;

    arr = strchr(arr, '[');
    if (!arr) return;

    osMutexAcquire(s_schedule_mutex, osWaitForever);
    s_schedule_count = 0;

    /* Iterar sobre los objetos del array */
    const char *p = arr + 1;
    while (*p && s_schedule_count < COMM_MAX_SCHEDULES) {
        /* Buscar inicio de objeto */
        p = strchr(p, '{');
        if (!p) break;

        /* Encontrar el cierre del objeto */
        const char *end = strchr(p, '}');
        if (!end) break;

        /* Extraer el objeto como substring */
        char obj[128] = {0};
        uint16_t len = (uint16_t)(end - p + 1);
        if (len >= sizeof(obj)) { p = end + 1; continue; }
        memcpy(obj, p, len);

        /* Parsear campos del objeto */
        int  hour   = 0;
        int  minute = 0;
        bool active = false;
        char label[16] = "Sin nombre";

        json_get_int(obj,    "hour",   &hour);
        json_get_int(obj,    "minute", &minute);
        json_get_bool(obj,   "active", &active);
        json_get_string(obj, "label",  label, sizeof(label));

        s_schedules[s_schedule_count].hour   = (uint8_t)hour;
        s_schedules[s_schedule_count].minute = (uint8_t)minute;
        s_schedules[s_schedule_count].active = active;
        strncpy(s_schedules[s_schedule_count].label, label,
                sizeof(s_schedules[0].label) - 1);
        s_schedule_count++;

        p = end + 1;
    }

    osMutexRelease(s_schedule_mutex);

    /* Log en UART debug */
    uint8_t buf[128];
    snprintf((char *)buf, sizeof(buf),
             "[COMM] Horarios actualizados: %d recibidos.\r\n",
             s_schedule_count);
    uartSendString(buf);

    for (uint8_t i = 0; i < s_schedule_count; i++) {
        snprintf((char *)buf, sizeof(buf),
                 "  [%d] %02d:%02d  %-12s  %s\r\n",
                 i,
                 s_schedules[i].hour,
                 s_schedules[i].minute,
                 s_schedules[i].label,
                 s_schedules[i].active ? "ACTIVO" : "inactivo");
        uartSendString(buf);
    }
}

/**
 * @brief  Despacha el JSON recibido al handler correspondiente.
 */
static void process_json(const char *json)
{
    char cmd[32] = {0};

    if (!json_get_string(json, "cmd", cmd, sizeof(cmd))) return;

    if (strcmp(cmd, "dispense") == 0) {
        handle_dispense(json);
    }
    else if (strcmp(cmd, "tare") == 0) {
        handle_tare();
    }
    else if (strcmp(cmd, "set_schedules") == 0) {
        handle_set_schedules(json);
    }
    else {
        uint8_t buf[64];
        snprintf((char *)buf, sizeof(buf),
                 "[COMM] Comando desconocido: %s\r\n", cmd);
        uartSendString(buf);
    }
}

/* =========================================================================== */
/*                          ENVÍO DE MENSAJES AL ESP32                        */
/* =========================================================================== */

void Comm_SendData(const Comm_Data *t)
{
    if (!t) return;

    // newlib-nano no soporta %f en snprintf por defecto.
    // Convertimos a entero + decimal manualmente para evitar el problema.
    int tank_int  = (int)t->tankWeight;
    int tank_dec  = (int)((t->tankWeight  - tank_int)  * 100);
    int plate_int = (int)t->plateWeight;
    int plate_dec = (int)((t->plateWeight - plate_int) * 10);

    uint8_t buf[UART_TX_MAX_SIZE];
    int len = snprintf((char *)buf, sizeof(buf),
        "{\"type\":\"telemetry\","
        "\"tank_weight\":%d.%02d,"
        "\"plate_weight\":%d.%01d,"
        "\"datetime\":{"
            "\"hour\":%d,\"minute\":%d,\"second\":%d,"
            "\"day\":%d,\"month\":%d,\"year\":%d,\"weekday\":%d"
        "}}\n",
        tank_int,  tank_dec,
        plate_int, plate_dec,
        t->hour, t->minute, t->second,
        t->day, t->month, t->year, t->weekday);

    if (len > 0) uartSendStringSize(buf, (uint16_t)len);
}

void Comm_SendFeedingDone(uint8_t amount, const char *label,
                          uint8_t hour, uint8_t minute)
{
    uint8_t buf[UART_TX_MAX_SIZE];
    int len = snprintf((char *)buf, sizeof(buf),
        "{\"type\":\"feeding_done\","
        "\"amount\":%d,"
        "\"label\":\"%s\","
        "\"hour\":%d,"
        "\"minute\":%d}\n",
        amount, label ? label : "Manual", hour, minute);

    if (len > 0) uartSendStringSize(buf, (uint16_t)len);
}

void Comm_SendTareDone(void)
{
    uartSendString((uint8_t *)"{\"type\":\"tare_done\"}\n");
}

/* =========================================================================== */
/*                          API PÚBLICA — SCHEDULES                           */
/* =========================================================================== */

void Comm_GetSchedules(Comm_Schedule *out, uint8_t *count)
{
    if (!out || !count) return;

    osMutexAcquire(s_schedule_mutex, osWaitForever);
    memcpy(out, s_schedules, s_schedule_count * sizeof(Comm_Schedule));
    *count = s_schedule_count;
    osMutexRelease(s_schedule_mutex);
}

/* =========================================================================== */
/*                          COMM TASK                                          */
/* =========================================================================== */

void Comm_Task(void *argument)
{
    (void)argument;

    char line[UART_RX_BUF_SIZE];

    uartSendString((uint8_t *)"[COMM] CommTask iniciada.\r\n");

    for (;;)
    {
        /* Esperar notificación de UART4 IRQ (línea completa recibida) */
        uint32_t flags = osThreadFlagsWait(COMM_UART_RX_FLAG,
                                           osFlagsWaitAny,
                                           osWaitForever);

        if (flags & COMM_UART_RX_FLAG) {
            uint16_t len = uartGetLine(line, sizeof(line));
            if (len > 0) {
                process_json(line);
            }
        }
    }
}

/* =========================================================================== */
/*                          INICIALIZACIÓN                                     */
/* =========================================================================== */

HAL_StatusTypeDef Comm_InitOS(void)
{
    /* Crear mutex para proteger el schedule */
    s_schedule_mutex = osMutexNew(NULL);
    if (s_schedule_mutex == NULL) return HAL_ERROR;

    /* Crear CommTask */
    static const osThreadAttr_t task_attr = {
        .name       = "CommTask",
        .stack_size = 512 * 4,
        .priority   = (osPriority_t)osPriorityAboveNormal,
    };

    s_comm_task_handle = osThreadNew(Comm_Task, NULL, &task_attr);
    if (s_comm_task_handle == NULL) return HAL_ERROR;

    /* Inicializar UART4 con RX por IRQ, pasando el handle de CommTask */
    if (!uartInit(s_comm_task_handle)) return HAL_ERROR;

    return HAL_OK;
}