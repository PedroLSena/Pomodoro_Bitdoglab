#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "hardware/clocks.h"

#define I2C_PORT    i2c1
#define I2C_SDA     14
#define I2C_SCL     15
#define DISPLAY_ADDR 0x3C

#define LED_PIN_G   11
#define LED_PIN_R   13
#define BUTTON_START 5
#define BUZZER_PIN 21  // Define o pino do buzzer

#define WORK_TIME  1 * 60  // 25 minutos em segundos
#define BREAK_TIME 1 * 60  // 5 minutos em segundos

bool is_working = true;
int time_left = WORK_TIME;
bool running = false;

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
    running = !running;
    if (!running) {
        time_left = is_working ? WORK_TIME : BREAK_TIME;
    }
}

// Função para tocar o buzzer
void play_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    
    pwm_set_wrap(slice_num, 12500);
    pwm_set_clkdiv(slice_num, 64.0);
    pwm_set_gpio_level(BUZZER_PIN, 6250);
    pwm_set_enabled(slice_num, true);
    
    sleep_ms(500); // Tempo do som do buzzer
    pwm_set_enabled(slice_num, false);
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
    
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    while (true) {
        if (running) {
            if (time_left > 0) {
                time_left--;
                gpio_put(LED_PIN_G, is_working);
                gpio_put(LED_PIN_R, !is_working);
            } else {
                play_buzzer();  // Toca o buzzer quando o tempo acabar
                is_working = !is_working;
                time_left = is_working ? WORK_TIME : BREAK_TIME;
            }
        }
        
        update_display(&ssd);
        sleep_ms(1000);
    }
}
