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

int work_time = 1 * 60;  
int break_time = 1 * 60;  
bool is_working = true;
int time_left;
bool running = false;
bool paused = false;

void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 255);  // Resolução de 8 bits (0-255)
    pwm_set_clkdiv(slice_num, 64.0);
    pwm_set_enabled(slice_num, true);
}

void set_led_brightness(uint pin, uint8_t brightness) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_gpio_level(pin, brightness);
}

void update_display(ssd1306_t* ssd) {
    char buffer[20];
    int minutes = time_left / 60;
    int seconds = time_left % 60;
    
    ssd1306_fill(ssd, false);
    sprintf(buffer, "%s", is_working ? "Trabalho" : "Descanso");
    ssd1306_draw_string(ssd, buffer, 30, 10);
    
    sprintf(buffer, "%02d:%02d", minutes, seconds);
    ssd1306_draw_string(ssd, buffer, 45, 30);
    
    ssd1306_send_data(ssd);
}

void button_irq(uint gpio, uint32_t events) {
    if (gpio == BUTTON_START) {
        if (!running) {
            running = true;
            time_left = is_working ? work_time : break_time;
        } else {
            running = false;
            time_left = is_working ? work_time : break_time;
        }
    } 
    else if (gpio == BUTTON_PAUSE) {
        paused = !paused;
        running = !paused;
    }
}

void play_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);  // Configura o pino do buzzer para PWM
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    pwm_set_wrap(slice_num, 12500);                
    pwm_set_clkdiv(slice_num, 64.0);               
    pwm_set_gpio_level(BUZZER_PIN, 6250);          
    pwm_set_enabled(slice_num, true);              

    sleep_ms(500);  

    pwm_set_enabled(slice_num, false);            
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_SIO); 
    gpio_put(BUZZER_PIN, 0);                      
}

void check_joystick() {
    if (!running || paused) {  // Permitir ajuste apenas antes de iniciar ou durante a pausa
        adc_select_input(1);  
        uint16_t joy_y = adc_read();
        
        if (joy_y > 3000) {
            if (is_working) {
                work_time += 60;  
                time_left = work_time;  
            } else {
                break_time += 60;  
                time_left = break_time;  
            }
            sleep_ms(300);
        } else if (joy_y < 1000) {
            if (is_working && work_time > 60) {
                work_time -= 60;
                time_left = work_time;
            } else if (!is_working && break_time > 60) {
                break_time -= 60;
                time_left = break_time;
            }
            sleep_ms(300);
        }
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
    
    setup_pwm(LED_PIN_G);
    setup_pwm(LED_PIN_R);
    
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
    
    time_left = work_time;  

    while (true) {
        check_joystick();
        
        if (paused) {
            set_led_brightness(LED_PIN_G, 128);  // 50% do brilho
            set_led_brightness(LED_PIN_R, 128);
        } else if (running) {
            if (time_left > 0) {
                time_left--;
                set_led_brightness(LED_PIN_G, is_working ? 50 : 0);
                set_led_brightness(LED_PIN_R, is_working ? 0 : 50);
            } else {
                play_buzzer();
                is_working = !is_working;
                time_left = is_working ? work_time : break_time;
            }
        } else {
            set_led_brightness(LED_PIN_G, 0);
            set_led_brightness(LED_PIN_R, 0);
        }
        
        update_display(&ssd);
        sleep_ms(1000);
    }
}