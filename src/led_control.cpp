#include "led_control.h"

#include <gpiod.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace std;

namespace
{
    struct gpiod_chip *chip = nullptr;
    struct gpiod_line_request *lineRequest = nullptr;

    const unsigned int GREEN_OFFSET = 19;
    const unsigned int RED_OFFSET = 26;

    unsigned int pinToOffset(const string &pin)
    {
        if (pin == GREEN_LED)
            return GREEN_OFFSET;
        if (pin == RED_LED)
            return RED_OFFSET;

        throw runtime_error("Unknown GPIO pin: " + pin);
    }

    void ensureRequestCreated()
    {
        if (lineRequest)
            return;

        chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip)
            throw runtime_error("Failed to open /dev/gpiochip0");

        struct gpiod_line_settings *settings = gpiod_line_settings_new();
        if (!settings)
            throw runtime_error("Failed to create gpiod line settings");

        if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0)
        {
            gpiod_line_settings_free(settings);
            throw runtime_error("Failed to set GPIO direction to output");
        }

        if (gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE) < 0)
        {
            gpiod_line_settings_free(settings);
            throw runtime_error("Failed to set initial GPIO output value");
        }

        struct gpiod_line_config *lineCfg = gpiod_line_config_new();
        if (!lineCfg)
        {
            gpiod_line_settings_free(settings);
            throw runtime_error("Failed to create gpiod line config");
        }

        unsigned int offsets[] = {GREEN_OFFSET, RED_OFFSET};
        if (gpiod_line_config_add_line_settings(lineCfg, offsets, 2, settings) < 0)
        {
            gpiod_line_config_free(lineCfg);
            gpiod_line_settings_free(settings);
            throw runtime_error("Failed to add GPIO line settings");
        }

        struct gpiod_request_config *requestCfg = gpiod_request_config_new();
        if (!requestCfg)
        {
            gpiod_line_config_free(lineCfg);
            gpiod_line_settings_free(settings);
            throw runtime_error("Failed to create gpiod request config");
        }

        gpiod_request_config_set_consumer(requestCfg, "SCMonitoring");

        lineRequest = gpiod_chip_request_lines(chip, requestCfg, lineCfg);
        if (!lineRequest)
        {
            gpiod_request_config_free(requestCfg);
            gpiod_line_config_free(lineCfg);
            gpiod_line_settings_free(settings);
            gpiod_chip_close(chip);
            chip = nullptr;
            throw runtime_error("Failed to request GPIO lines 19 and 26");
        }

        gpiod_request_config_free(requestCfg);
        gpiod_line_config_free(lineCfg);
        gpiod_line_settings_free(settings);
    }
}

void writeToFile(const string &path, const string &value)
{
    throw runtime_error("writeToFile is not used with libgpiod: " + path + " <- " + value);
}

void exportGPIO(const string &pin)
{
    (void)pin;
    ensureRequestCreated();
}

void setDirection(const string &pin, const string &direction)
{
    (void)pin;

    if (direction != "out")
        throw runtime_error("Only output direction is supported for LEDs");

    ensureRequestCreated();
}

void setValue(const string &pin, const string &value)
{
    ensureRequestCreated();

    unsigned int offset = pinToOffset(pin);
    enum gpiod_line_value outValue;

    if (value == "1")
        outValue = GPIOD_LINE_VALUE_ACTIVE;
    else if (value == "0")
        outValue = GPIOD_LINE_VALUE_INACTIVE;
    else
        throw runtime_error("Invalid GPIO value: " + value);

    if (gpiod_line_request_set_value(lineRequest, offset, outValue) < 0)
        throw runtime_error("Failed to set value for GPIO " + pin);
}

void unexportGPIO(const string &pin)
{
    (void)pin;

    if (lineRequest)
    {
        gpiod_line_request_release(lineRequest);
        lineRequest = nullptr;
    }

    if (chip)
    {
        gpiod_chip_close(chip);
        chip = nullptr;
    }
}

void allOff()
{
    setValue(GREEN_LED, "0");
    setValue(RED_LED, "0");
}

void normalOperation()
{
    for (int i = 0; i < 3; i++)
    {
        setValue(GREEN_LED, "1");
        setValue(RED_LED, "0");
        sleep(1);
    }
    allOff();
}

void errorDetected()
{
    setValue(GREEN_LED, "0");
    for (int i = 0; i < 3; i++)
    {
        setValue(RED_LED, "1");
        sleep(1);
        setValue(RED_LED, "0");
        sleep(1);
    }
    allOff();
}

void tamperingDetected()
{
    setValue(GREEN_LED, "0");
    for (int i = 0; i < 3; i++)
    {
        setValue(RED_LED, "1");
        usleep(500000);
        setValue(RED_LED, "0");
        usleep(500000);
    }
    allOff();
}