//Quest 2
//Mario Han, Vivek Cherian, Hussain Valiuddin
#include <stdio.h>
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_types.h"
#include "esp_attr.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "math.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 10  //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel1 = ADC_CHANNEL_3; //Thermistor GPIO 36 A3
static const adc_channel_t channel2 = ADC_CHANNEL_6; //IR GPIO 39 A2
static const adc_channel_t channel3 = ADC_CHANNEL_0; //ultrasonic GPIO 34 A4
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static double temperature;
static int ultrasonic;
static int ir_rangefinder;

// void init()
// {
//     //Configure ADC
//     adc1_config_width(ADC_WIDTH_BIT_12);
//     adc1_config_channel_atten(channel1, atten);
//     adc1_config_channel_atten(channel2, atten);
//     adc1_config_channel_atten(channel3, atten);

//     //Characterize ADC
//     adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
//     esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
// }

static void thermistor()
{ // push button
    while (1)
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)channel1);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        float temp, rtherm;
        rtherm = adc_reading;
        rtherm = (1000 * ((4096 / rtherm) - 1));
        temp = rtherm / 10000;
        temp = log(temp);
        temp /= 3435;
        temp += 1.0 / (20 + 273.15);
        temp = 1.0 / temp;
        temp -= 273.15;
        temperature = temp;
        /*
        float Kel, Cel;
        float R0 = 5700;
        float Rt = R0 * ((adcMAX/adc_reading)-1);
        float mult =log ( Rt );
        float calc = invT0 + (invBeta* mult);
        Kel = 1.00 / calc;
        Cel = Kel - 273.15;                      // convert to Celsius
  */
        //printf("Raw: %d\t volt: %d\t Cel: %f\n", adc_reading, voltage, temp);
    }
}

static void IR_Range()
{ // push button

    while (1)
    {
        uint32_t adc_reading = 0;
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
        ir_rangefinder = distance;
        //printf("Raw: %d\tVoltage: %fV\tDistance: %dcm\n", adc_reading, voltage, distance);
    }
}

static void printstate()
{
    while (1)
    {
        printf("Temp:%.1f, Dist1:%d, Dist2:%d\n", temperature, ultrasonic, ir_rangefinder);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

static void ultra_sonic()
{
    //Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw((adc1_channel_t)channel3);
                vTaskDelay(100 / portTICK_RATE_MS);
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        // display voltage
        uint32_t dist = ((1 / 6.4 * (voltage)) - 5) * 2.54;
        // uint32_t distance2 = ultrasonic_voltage_to_distance(voltage);
        ultrasonic = dist;
        //printf("Raw: %d\tVoltage: %dmV\tDistance: %din\n", adc_reading, voltage, dist);
    }
}

void app_main()
{
    //init(); // Initialize stuff

    //Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel1, atten);
    adc1_config_channel_atten(channel2, atten);
    adc1_config_channel_atten(channel3, atten);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    xTaskCreate(thermistor, "thermistor", 4096, NULL, 5, NULL);
    xTaskCreate(IR_Range, "IR_Range", 4096, NULL, 6, NULL);
    xTaskCreate(printstate, "printstate", 4096, NULL, 8, NULL);
    xTaskCreate(ultra_sonic, "ultra_sonic", 4096, NULL, 7, NULL);
}
