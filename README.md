# Timer Pomodoro - BitDogLab

Vídeo de ensaio:[https://youtu.be/GTeZVxj5ex0](https://youtu.be/GTeZVxj5ex0)

Este projeto implementa um Timer Pomodoro utilizando a placa de desenvolvimento **BitDogLab**, com um display OLED, botões para controle e um joystick para ajuste do tempo.

## 🎯 Funcionalidades
- **Modo Pomodoro**: Alterna entre períodos de trabalho e descanso.
- **Controle de Tempo**: O tempo pode ser ajustado utilizando um joystick.
- **Botões de Controle**:
  - Iniciar/Parar o timer
  - Pausar/Retomar
- **Feedback Visual e Sonoro**:
  - LEDs indicam o estado do timer
  - Um buzzer emite um som ao final de cada ciclo
- **Display OLED**:
  - Exibe o tempo restante e o estado atual do cronômetro

## 🛠 Tecnologias Utilizadas
- **Placa BitDogLab** (RP2040)
- **Linguagem C**
- **Bibliotecas**:
  - `pico/stdlib.h`
  - `hardware/i2c.h`
  - `hardware/gpio.h`
  - `hardware/pwm.h`
  - `hardware/adc.h`
  - `ssd1306.h` (para o display OLED)

## 🔧 Configuração e Uso
### 1️⃣ Requisitos
- Placa **BitDogLab** (RP2040)
- Display OLED (SSD1306, via I2C)
- Buzzer ativo/passivo
- Joystick analógico
- Botões de controle

### 2️⃣ Conexões de Hardware
| Componente   | Pino RP2040 |
|-------------|------------|
| Display OLED (SDA) | GP14 |
| Display OLED (SCL) | GP15 |
| LED Verde   | GP11 |
| LED Vermelho | GP13 |
| Botão Start | GP5 |
| Botão Pause | GP6 |
| Buzzer      | GP21 |
| Joystick Y  | GP27 (ADC1) |

### 3️⃣ Compilação e Upload
1. Clone este repositório:
   ```sh
   git clone https://github.com/seu-usuario/timer-pomodoro-bitdoglab.git
   cd timer-pomodoro-bitdoglab
   ```
2. Compile o código usando o SDK do RP2040:
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```
3. Envie o arquivo `.uf2` para a placa **BitDogLab**.

## 🚀 Como Usar
1. **Pressione o botão Start** para iniciar o timer.
2. **Pressione Pause** para pausar e retomar o temporizador.
3. **Use o joystick** para ajustar o tempo antes de iniciar.
4. **O buzzer tocará** quando o tempo acabar e alternará entre trabalho e descanso.

## 📜 Licença
Este projeto está sob a licença MIT. Sinta-se livre para modificar e compartilhar!

## 📩 Contato
Se tiver dúvidas ou sugestões, entre em contato:
- 📧 Email: pes.ex20000@gmail.com
- 🔗 GitHub: [PedroLSena](https://github.com/PedroLSena)

---
🛠 **Desenvolvido por [Pedro Sena]** 🚀

