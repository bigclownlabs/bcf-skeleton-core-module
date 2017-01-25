#include <application.h>
#include <usb_talk.h>
#include <bc_led.h>
#include <bc_button.h>
#include <bc_i2c.h>
#include <bc_temperature_tag.h>
#include <bc_humidity_tag.h>
#include <bc_lux_meter_tag.h>
#include <bc_barometer_tag.h>

#include <bc_spirit1.h>

struct
{
    struct { bool valid; float value; } temperature;
    struct { bool valid; float value; } humidity;
    struct { bool valid; float value; } luminosity;
    struct { bool valid; float value; } altitude;
    struct { bool valid; float value; } pressure;

} i2c_sensors;

bc_led_t led;

uint8_t rx_buffer[128];
size_t rx_length;

static void spirit1_event_handler(bc_spirit1_event_t event);

void temperature_tag_event_handler(bc_temperature_tag_t *self, bc_temperature_tag_event_t event)
{
    (void) event;

    i2c_sensors.temperature.valid = bc_temperature_tag_get_temperature_celsius(self, &i2c_sensors.temperature.value);
}

void humidity_tag_event_handler(bc_humidity_tag_t *self, bc_humidity_tag_event_t event)
{
    (void) event;

    i2c_sensors.humidity.valid = bc_humidity_tag_get_humidity_percentage(self, &i2c_sensors.humidity.value);
}

void lux_meter_event_handler(bc_lux_meter_tag_t *self, bc_lux_meter_tag_event_t event)
{
    (void) event;

    i2c_sensors.luminosity.valid = bc_lux_meter_tag_get_luminosity_lux(self, &i2c_sensors.luminosity.value);
}

void barometer_tag_event_handler(bc_barometer_tag_t *self, bc_barometer_tag_event_t event)
{
    (void) event;

    i2c_sensors.altitude.valid = bc_barometer_tag_get_altitude_meter(self, &i2c_sensors.altitude.value);
    i2c_sensors.pressure.valid = bc_barometer_tag_get_pressure_pascal(self, &i2c_sensors.pressure.value);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event)
{
    (void) self;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        static const uint8_t buf[4] = {1,2,3,4};

        bc_led_pulse(&led, 100);

        bc_spirit1_transmit(buf, sizeof(buf));
        /*
        static uint16_t event_count = 0;

        bc_led_set_mode(&led, (event_count & 1) != 0 ? BC_LED_MODE_BLINK : BC_LED_MODE_OFF);

        usb_talk_publish_push_button("", &event_count);

        event_count++;
        */
    }
}

void application_init(void)
{
    usb_talk_init();

    bc_led_init(&led, BC_GPIO_LED, false, false);
    //bc_led_set_mode(&led, BC_LED_MODE_BLINK);

    static bc_button_t button;

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler);

    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_400_KHZ);

    static bc_temperature_tag_t temperature_tag;

    bc_temperature_tag_init(&temperature_tag, BC_I2C_I2C0, BC_TEMPERATURE_TAG_I2C_ADDRESS_ALTERNATE);
    bc_temperature_tag_set_update_interval(&temperature_tag, 1000);
    bc_temperature_tag_set_event_handler(&temperature_tag, temperature_tag_event_handler);

    static bc_humidity_tag_t humidity_tag;

    bc_humidity_tag_init(&humidity_tag, BC_I2C_I2C0, BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT);
    bc_humidity_tag_set_update_interval(&humidity_tag, 1000);
    bc_humidity_tag_set_event_handler(&humidity_tag, humidity_tag_event_handler);

    static bc_lux_meter_tag_t lux_meter;

    bc_lux_meter_tag_init(&lux_meter, BC_I2C_I2C0, BC_LUX_METER_TAG_I2C_ADDRESS_DEFAULT);
    bc_lux_meter_tag_set_update_interval(&lux_meter, 1000);
    bc_lux_meter_tag_set_event_handler(&lux_meter, lux_meter_event_handler);

    static bc_barometer_tag_t barometer_tag;

    bc_barometer_tag_init(&barometer_tag, BC_I2C_I2C0);
    bc_barometer_tag_set_update_interval(&barometer_tag, 1000);
    bc_barometer_tag_set_event_handler(&barometer_tag, barometer_tag_event_handler);

    bc_spirit1_init();
    bc_spirit1_set_event_handler(spirit1_event_handler);
    bc_spirit1_receive(rx_buffer, &rx_length, 1000);
}

static void spirit1_event_handler(bc_spirit1_event_t event)
{
    if (event == BC_SPIRIT1_EVENT_RECEPTION_DONE)
    {
        bc_led_pulse(&led, 100);
    }

    if (event == BC_SPIRIT1_EVENT_TRANSMISSION_DONE)
    {
        bc_spirit1_receive(rx_buffer, &rx_length, 1000);
    }
}
