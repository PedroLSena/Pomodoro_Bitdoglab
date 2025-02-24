#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "hardware/clocks.h"

#define I2C_PORT    i2c1
#define I2C_SDA     14
#define I2C_SCL     15
#define DISPLAY_ADDR 0x3C

#define LED_PIN_G   11
#define LED_PIN_R   13
#define BUTTON_START 5
#define BUTTON_PAUSE 6
#define BUZZER_PIN  21

#define JOY_Y 27  // Pino do eixo Y do joystick

#define WORK_TIME  1 * 60  
#define BREAK_TIME 1 * 60  

bool is_working = true;
int time_left = WORK_TIME;
bool running = false;
bool paused = false;

void update_display(ssd1306_t* ssd) {
    char buffer[20];
    int minutes = time_left / 60;
    int seconds = time_left % 60;
    
    ssd1306_fill(ssd, false);
    sprintf(buffer, "%s", is_working ? "Trabalho" : "Descanso");
    ssd1306_draw_string(ssd, buffer, 10, 10);
    
    sprintf(buffer, "%02d:%02d", minutes, seconds);
    ssd1306_draw_string(ssd, buffer, 40, 30);
    
    ssd1306_send_data(ssd);
}

void button_irq(uint gpio, uint32_t events) {
    if (gpio == BUTTON_START) {
        if (!running) {
            running = true;
            time_left = is_working ? WORK_TIME : BREAK_TIME;
        } else {
            running = false;
            time_left = is_working ? WORK_TIME : BREAK_TIME;
        }
    } 
    else if (gpio == BUTTON_PAUSE) {
        paused = !paused;
        running = !paused;
    }
}

void play_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    
    pwm_set_wrap(slice_num, 12500);
    pwm_set_clkdiv(slice_num, 64.0);
    pwm_set_gpio_level(BUZZER_PIN, 6250);
    pwm_set_enabled(slice_num, true);
    
    sleep_ms(500);
    pwm_set_enabled(slice_num, false);
}

void check_joystick() {
    adc_select_input(1);  // Seleciona o canal ADC correspondente ao eixo Y
    uint16_t joy_y = adc_read();
    
    if (joy_y > 3000) {
        time_left += 60;  // Adiciona um minuto
        sleep_ms(300);
    } else if (joy_y < 1000 && time_left > 60) {
        time_left -= 60;  // Diminui um minuto, garantindo que não fique negativo
        sleep_ms(300);
    }
}

int main() {
    stdio_init_all();
    
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    
    gpio_init(BUTTON_START);
    gpio_set_dir(BUTTON_START, GPIO_IN);
    gpio_pull_up(BUTTON_START);
    gpio_set_irq_enabled_with_callback(BUTTON_START, GPIO_IRQ_EDGE_FALL, true, &button_irq);
    
    gpio_init(BUTTON_PAUSE);
    gpio_set_dir(BUTTON_PAUSE, GPIO_IN);
    gpio_pull_up(BUTTON_PAUSE);
    gpio_set_irq_enabled_with_callback(BUTTON_PAUSE, GPIO_IRQ_EDGE_FALL, true, &button_irq);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    
    adc_init();
    adc_gpio_init(JOY_Y);
    
    while (true) {
        check_joystick();
        
        if (running && !paused) {
            if (time_left > 0) {
                time_left--;
                gpio_put(LED_PIN_G, is_working);
                gpio_put(LED_PIN_R, !is_working);
            } else {
                play_buzzer();
                is_working = !is_working;
                time_left = is_working ? WORK_TIME : BREAK_TIME;
            }
        }
        
        update_display(&ssd);
        sleep_ms(1000);
    }
}
