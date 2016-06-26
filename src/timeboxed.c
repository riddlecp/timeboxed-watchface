#include <pebble.h>
#include <time.h>
#include "keys.h"
#include "locales.h"
#include "health.h"
#include "text.h"
#include "weather.h"
#include "configs.h"
#include "positions.h"
#include "screen.h"

static Window *watchface;

static uint8_t min_counter;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *error_tuple = dict_find(iterator, KEY_ERROR);

    if (error_tuple) {
        get_health_data();
        return;
    }

    Tuple *update_tuple = dict_find(iterator, KEY_HASUPDATE);
    if (update_tuple) {
        int update_val = update_tuple->value->int8;
        persist_write_int(KEY_HASUPDATE, update_val);
        notify_update(update_val);
        return;
    }

    Tuple *temp_tuple = dict_find(iterator, KEY_TEMP);
    Tuple *max_tuple = dict_find(iterator, KEY_MAX);
    Tuple *min_tuple = dict_find(iterator, KEY_MIN);
    Tuple *weather_tuple = dict_find(iterator, KEY_WEATHER);
    Tuple *feels_tuple = dict_find(iterator, KEY_FEELS);
    Tuple *speed_tuple = dict_find(iterator, KEY_SPEED);
    Tuple *direction_tuple = dict_find(iterator, KEY_DIRECTION);

    if (temp_tuple && max_tuple && min_tuple && weather_tuple && is_weather_enabled()) {
        int temp_val = (int)temp_tuple->value->int32;
        int max_val = (int)max_tuple->value->int32;
        int min_val = (int)min_tuple->value->int32;
        int weather_val = (int)weather_tuple->value->int32;

        update_weather_values(temp_val, weather_val);
        update_forecast_values(max_val, min_val);

        //int feels_val = (int)feels_tuple->value->int32;
        int speed_val = (int)speed_tuple->value->int32;
        int direction_val = (int)direction_tuple->value->int32;

        //update_feels_value(feels_val);
        update_wind_values(speed_val, direction_val);
        store_weather_values(temp_val, max_val, min_val, weather_val, speed_val, direction_val);

        APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather data updated. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
        return;
    }

    int configs = 0;
    signed int tz_hour = 0;
    uint8_t tz_minute = 0;
    static char tz_name[TZ_LEN];

    Tuple *enableHealth = dict_find(iterator, KEY_ENABLEHEALTH);
    if (enableHealth) {
        bool enabled = enableHealth->value->int8;
        if (enabled) configs += FLAG_HEALTH;
    }

    Tuple *useKm = dict_find(iterator, KEY_USEKM);
    if (useKm) {
        bool enabled = useKm->value->int8;
        if (enabled) configs += FLAG_KM;
    }

    Tuple *useCal = dict_find(iterator, KEY_USECAL);
    if (useCal) {
        bool enabled = useCal->value->int8;
        if (enabled) configs += FLAG_CALORIES;
    }

    Tuple *showSleep = dict_find(iterator, KEY_SHOWSLEEP);
    if (showSleep) {
        bool enabled = showSleep->value->int8;
        if (enabled) configs += FLAG_SLEEP;
    }

    Tuple *enableWeather = dict_find(iterator, KEY_ENABLEWEATHER);
    if (enableWeather) {
        bool enabled =  enableWeather->value->int8;
        if (enabled) configs += FLAG_WEATHER;
    }

    Tuple *useCelsius = dict_find(iterator, KEY_USECELSIUS);
    if (useCelsius) {
        bool enabled = useCelsius->value->int8;
        if (enabled)  configs += FLAG_CELSIUS;
    }

    Tuple *timezones = dict_find(iterator, KEY_TIMEZONES);
    if (timezones) {
        signed int tz = timezones->value->int8;
        persist_write_int(KEY_TIMEZONES, tz);
        tz_hour = tz;
    }

    Tuple *timezonesMin = dict_find(iterator, KEY_TIMEZONESMINUTES);
    if (timezonesMin) {
        int tz_min = timezonesMin->value->int8;
        persist_write_int(KEY_TIMEZONESMINUTES, tz_min);
        tz_minute = tz_min;
    }

    Tuple *timezonesCode = dict_find(iterator, KEY_TIMEZONESCODE);
    if (timezones) {
        char* tz_code = timezonesCode->value->cstring;
        persist_write_string(KEY_TIMEZONESCODE, tz_code);
        strcpy(tz_name, tz_code);
        if (tz_code[0] != '#') configs += FLAG_TIMEZONES;
    }

    Tuple *bgColor = dict_find(iterator, KEY_BGCOLOR);
    if (bgColor) {
        persist_write_int(KEY_BGCOLOR, bgColor->value->int32);
    }

    Tuple *hoursColor = dict_find(iterator, KEY_HOURSCOLOR);
    if (hoursColor) {
        persist_write_int(KEY_HOURSCOLOR, hoursColor->value->int32);
    }

    Tuple *enableAdvanced = dict_find(iterator, KEY_ENABLEADVANCED);
    if (enableAdvanced) {
        bool enabled = enableAdvanced->value->int8;
        if (enabled) configs += FLAG_ADVANCED;
    }

    Tuple *dateColor = dict_find(iterator, KEY_DATECOLOR);
    if (dateColor) {
        persist_write_int(KEY_DATECOLOR, dateColor->value->int32);
    }

    Tuple *altHoursColor = dict_find(iterator, KEY_ALTHOURSCOLOR);
    if (altHoursColor) {
        persist_write_int(KEY_ALTHOURSCOLOR, altHoursColor->value->int32);
    }

    Tuple *batteryColor = dict_find(iterator, KEY_BATTERYCOLOR);
    if (batteryColor) {
        persist_write_int(KEY_BATTERYCOLOR, batteryColor->value->int32);
    }

    Tuple *batteryLowColor = dict_find(iterator, KEY_BATTERYLOWCOLOR);
    if (batteryLowColor) {
        persist_write_int(KEY_BATTERYLOWCOLOR, batteryLowColor->value->int32);
    }

    Tuple *weatherColor = dict_find(iterator, KEY_WEATHERCOLOR);
    if (weatherColor) {
        persist_write_int(KEY_WEATHERCOLOR, weatherColor->value->int32);
    }

    Tuple *tempColor = dict_find(iterator, KEY_TEMPCOLOR);
    if (tempColor) {
        persist_write_int(KEY_TEMPCOLOR, tempColor->value->int32);
    }

    Tuple *minColor = dict_find(iterator, KEY_MINCOLOR);
    if (minColor) {
        persist_write_int(KEY_MINCOLOR, minColor->value->int32);
    }

    Tuple *maxColor = dict_find(iterator, KEY_MAXCOLOR);
    if (maxColor) {
        persist_write_int(KEY_MAXCOLOR, maxColor->value->int32);
    }

    Tuple *windDirColor = dict_find(iterator, KEY_WINDDIRCOLOR);
    if (windDirColor) {
        persist_write_int(KEY_WINDDIRCOLOR, windDirColor->value->int32);
    }

    Tuple *windSpeedColor = dict_find(iterator, KEY_WINDSPEEDCOLOR);
    if (windSpeedColor) {
        persist_write_int(KEY_WINDSPEEDCOLOR, windSpeedColor->value->int32);
    }

    Tuple *stepsColor = dict_find(iterator, KEY_STEPSCOLOR);
    if (stepsColor) {
        persist_write_int(KEY_STEPSCOLOR, stepsColor->value->int32);
    }

    Tuple *distColor = dict_find(iterator, KEY_DISTCOLOR);
    if (distColor) {
        persist_write_int(KEY_DISTCOLOR, distColor->value->int32);
    }

    Tuple *calColor = dict_find(iterator, KEY_CALCOLOR);
    if (calColor) {
        persist_write_int(KEY_CALCOLOR, calColor->value->int32);
    }

    Tuple *stepsBehindColor = dict_find(iterator, KEY_STEPSBEHINDCOLOR);
    if (stepsBehindColor) {
        persist_write_int(KEY_STEPSBEHINDCOLOR, stepsBehindColor->value->int32);
    }

    Tuple *distBehindColor = dict_find(iterator, KEY_DISTBEHINDCOLOR);
    if (distBehindColor) {
        persist_write_int(KEY_DISTBEHINDCOLOR, distBehindColor->value->int32);
    }

    Tuple *calBehindColor = dict_find(iterator, KEY_CALBEHINDCOLOR);
    if (calBehindColor) {
        persist_write_int(KEY_CALBEHINDCOLOR, calBehindColor->value->int32);
    }

    Tuple *sleepBehindColor = dict_find(iterator, KEY_SLEEPBEHINDCOLOR);
    if (sleepBehindColor) {
        persist_write_int(KEY_SLEEPBEHINDCOLOR, sleepBehindColor->value->int32);
    }

    Tuple *deepBehindColor = dict_find(iterator, KEY_DEEPBEHINDCOLOR);
    if (deepBehindColor) {
        persist_write_int(KEY_DEEPBEHINDCOLOR, deepBehindColor->value->int32);
    }

    Tuple *fontType = dict_find(iterator, KEY_FONTTYPE);
    if (fontType) {
        persist_write_int(KEY_FONTTYPE, fontType->value->int8);
    }

    Tuple *bluetoothDisconnect = dict_find(iterator, KEY_BLUETOOTHDISCONNECT);
    if (bluetoothDisconnect) {
        bool enabled = bluetoothDisconnect->value->int8;
        if(enabled) configs += FLAG_BLUETOOTH;
    }

    Tuple *bluetoothColor = dict_find(iterator, KEY_BLUETOOTHCOLOR);
    if (bluetoothColor) {
        persist_write_int(KEY_BLUETOOTHCOLOR, bluetoothColor->value->int32);
    }

    Tuple *overrideLocation = dict_find(iterator, KEY_OVERRIDELOCATION);
    if (overrideLocation) {
        persist_write_string(KEY_OVERRIDELOCATION, overrideLocation->value->cstring);
    }

    Tuple *updateAvailable = dict_find(iterator, KEY_UPDATE);
    if (updateAvailable) {
        bool disabled = updateAvailable->value->int8;
        if (!disabled) configs += FLAG_UPDATE;
    }

    Tuple *updateColor = dict_find(iterator, KEY_UPDATECOLOR);
    if (updateColor) {
        persist_write_int(KEY_UPDATECOLOR, updateColor->value->int32);
    }

    Tuple *locale = dict_find(iterator, KEY_LOCALE);
    if (locale) {
        persist_write_int(KEY_LOCALE, locale->value->int8);
    }

    Tuple *dateFormat = dict_find(iterator, KEY_DATEFORMAT);
    if (dateFormat) {
        persist_write_int(KEY_DATEFORMAT, dateFormat->value->int8);
    }

    Tuple *textAlign = dict_find(iterator, KEY_TEXTALIGN);
    if (textAlign) {
        persist_write_int(KEY_TEXTALIGN, textAlign->value->int8);
    }

    Tuple *speedUnit = dict_find(iterator, KEY_SPEEDUNIT);
    if (speedUnit) {
        persist_write_int(KEY_SPEEDUNIT, speedUnit->value->int8);
    }

    Tuple *leadingZero = dict_find(iterator, KEY_LEADINGZERO);
    if (leadingZero) {
        bool disabled = leadingZero->value->int8;
        if (!disabled) configs += FLAG_LEADINGZERO;
    }

    Tuple *simpleMode = dict_find(iterator, KEY_SIMPLEMODE);
    if (simpleMode) {
        bool enabled = simpleMode->value->int8;
        if (enabled) configs += FLAG_SIMPLEMODE;
    }

    Tuple *slotA = dict_find(iterator, KEY_SLOTA);
    if (slotA) {
        int value = slotA->value->int8;
        set_module(SLOT_A, value, false);
        persist_write_int(KEY_SLOTA, value);
    }
    Tuple *slotB = dict_find(iterator, KEY_SLOTB);
    if (slotB) {
        int value = slotB->value->int8;
        set_module(SLOT_B, value, false);
        persist_write_int(KEY_SLOTB, value);
    }
    Tuple *slotC = dict_find(iterator, KEY_SLOTC);
    if (slotC) {
        int value = slotC->value->int8;
        set_module(SLOT_C, value, false);
        persist_write_int(KEY_SLOTC, value);
    }
    Tuple *slotD = dict_find(iterator, KEY_SLOTD);
    if (slotD) {
        int value = slotD->value->int8;
        set_module(SLOT_D, value, false);
        persist_write_int(KEY_SLOTD, value);
    }

    Tuple *slotASleep = dict_find(iterator, KEY_SLEEPSLOTA);
    if (slotASleep) {
        int value = slotASleep->value->int8;
        set_module(SLOT_A, value, true);
        persist_write_int(KEY_SLEEPSLOTA, value);
    }
    Tuple *slotBSleep = dict_find(iterator, KEY_SLEEPSLOTB);
    if (slotBSleep) {
        int value = slotBSleep->value->int8;
        set_module(SLOT_B, value, true);
        persist_write_int(KEY_SLEEPSLOTB, value);
    }
    Tuple *slotCSleep = dict_find(iterator, KEY_SLEEPSLOTC);
    if (slotCSleep) {
        int value = slotCSleep->value->int8;
        set_module(SLOT_C, value, true);
        persist_write_int(KEY_SLEEPSLOTC, value);
    }
    Tuple *slotDSleep = dict_find(iterator, KEY_SLEEPSLOTD);
    if (slotDSleep) {
        int value = slotDSleep->value->int8;
        set_module(SLOT_D, value, true);
        persist_write_int(KEY_SLEEPSLOTD, value);
    }

    persist_write_int(KEY_CONFIGS, configs);
    set_config_toggles(configs);
    set_timezone(tz_name, tz_hour, tz_minute);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Configs persisted. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    destroy_text_layers();
    create_text_layers(watchface);
    load_screen(true, watchface);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! Reason code: %d", reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason code: %d", reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox send success!");
}

static void watchface_load(Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Watchface load start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));

    create_text_layers(window);

    min_counter = 20; // after loading, get the next weather update in 10 min

    load_timezone_from_storage();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Watchface load end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
}

static void watchface_unload(Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Unload start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));

    save_health_data_to_storage();

    unload_face_fonts();

    destroy_text_layers();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Unload end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    min_counter++;

    if (!is_update_disabled() && tick_time->tm_hour == 4 && tick_time->tm_min == 0) { // updates at 4:00am
        check_for_updates();
    }
    if (is_update_disabled()) {
        notify_update(false);
    }

    uint8_t tick_interval = is_user_sleeping() ? 90 : 30;

    show_sleep_data_if_visible(watchface);

    if(min_counter >= tick_interval) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Time for updates (%d). %d%03d", tick_interval, (int)time(NULL), (int)time_ms(NULL, NULL));
        if (is_weather_enabled()) {
            update_weather();
        }
        if (is_user_sleeping()) {
            queue_health_update();
        }
        min_counter = 0;
    }
    if (tick_time->tm_min % 2 == 0) { // check for health updates every 2 minutes
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting health from time. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
        get_health_data();
    }
}

static void init(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Init start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    init_sleep_data();
    queue_health_update();
    init_positions();

    watchface = window_create();

    window_set_window_handlers(watchface, (WindowHandlers) {
        .load = watchface_load,
        .unload = watchface_unload,
    });

    battery_state_service_subscribe(battery_handler);

    window_stack_push(watchface, true);

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(512, 64);

    connection_service_subscribe((ConnectionHandlers) {
	.pebble_app_connection_handler = bt_handler
    });

    load_screen(false, watchface);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Init end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
}

static void deinit(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Deinit start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    window_destroy(watchface);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Deinit end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
}

int main(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    init();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App event loop start. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    app_event_loop();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App event loop end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
    deinit();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App end. %d%03d", (int)time(NULL), (int)time_ms(NULL, NULL));
}
