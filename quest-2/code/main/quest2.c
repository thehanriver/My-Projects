//Quest 2
//Mario Han, Vivek Cherian, Hussain Valiuddin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "esp_vfs_dev.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 20  //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel1 = a1; //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_channel_t channel2 = a2; //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_channel_t channel3 = a3; //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

void init()
{
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    //Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel1, atten);
    adc1_config_channel_atten(channel2, atten);
    adc1_config_channel_atten(channel3, atten);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

// converts voltage to distances in in
static uint32_t voltage_to_distance(uint32_t reading)
{
    if (reading==0)
    {
        printf("returning 0");
        return 0;
    }
    else
    {
        // uint32_t dist = (1 / .2519685 * (reading))*pow(10,-1); // .2519685 mV per mm
        uint32_t dist = ((1 / 6.4 * (reading))-5)*2.54; // 6.4 mV per in
        return dist;
    }
}

static void thermistor()
{ // push button
    while (1)
    {
    }
}

static void IR_Range()
{ // push button
    while (1)
    {
        uint32_t adc_reading = 0;
        uint32_t reading = 0;

        int distance = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)channel2);
                vTaskDelay(100 / portTICK_RATE_MS);
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        double voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        voltage /= 1000;
        if (voltage > 2)
        {
            distance = (30 / (voltage - 1));
        }
        else
        {
            distance = (57 / (voltage - 0.08));
        }

        printf("Raw: %d\tVoltage: %fV\tDistance: %dcm\n", adc_reading, voltage, distance);
    }
}

static void ultra_sonic()
{ 
    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel3);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel3, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        
        // display voltage
        uint32_t distance = voltage_to_distance(voltage);
        printf("Raw: %d\tVoltage: %dmV\tDistance: %din\n", adc_reading, voltage,distance);
    }
}

void app_main()
{
    init(); // Initialize stuff

    xTaskCreate(thermistor, "thermistor", 4096, NULL, 7, NULL);
    xTaskCreate(IR_Range, "IR_Range", 4096, NULL, 5, NULL);
    xTaskCreate(ultra_sonic, "ultra_sonic", 4096, NULL, 9, NULL);
}