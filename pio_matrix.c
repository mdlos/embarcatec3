#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

// Inclusão do programa PIO para LEDs
#include "pio_matrix.pio.h"

// Configuração de parâmetros
#define NUM_PIXELS 25
#define BUZZER_PIN 21

// Mapeamento do teclado matricial
const uint8_t colunas[4] = {4, 3, 2, 1}; // Pinos das colunas
const uint8_t linhas[4] = {5, 6, 7, 8};  // Pinos das linhas
const char teclado[4][4] = 
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

double desenhoCoracao[25] = {
    1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 0.0,
    1.0, 1.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0
};

// Função para converter valores RGB e intensidade em formato compatível com LEDs WS2812B
uint32_t matrix_rgb(double r, double g, double b, double intensity) {
    unsigned char R = r * 255 * intensity;
    unsigned char G = g * 255 * intensity;
    unsigned char B = b * 255 * intensity;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para acionar os LEDs
void acionar_leds(PIO pio, uint sm, double r, double g, double b, double intensity) {
    uint32_t cor = matrix_rgb(r, g, b, intensity);
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, cor);
    }
}

// Função para desenhar um padrão no painel de LEDs
void desenhar_padrao(PIO pio, uint sm, double padrao[NUM_PIXELS]) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        double intensity = padrao[i];
        uint32_t cor = matrix_rgb(1.0, 0.0, 0.0, intensity); // Vermelho com intensidade do padrão
        pio_sm_put_blocking(pio, sm, cor);
    }
}

// Função para configurar o teclado matricial
void configurar_teclado() {
    for (int i = 0; i < 4; i++) {
        gpio_init(linhas[i]);
        gpio_set_dir(linhas[i], GPIO_OUT);
        gpio_put(linhas[i], 1);
        gpio_init(colunas[i]);
        gpio_set_dir(colunas[i], GPIO_IN);
        gpio_pull_up(colunas[i]);
    }
}

// Função para ler a tecla pressionada
char ler_tecla() {
    for (int i = 0; i < 4; i++) {
        gpio_put(linhas[i], 0); // Ativa a linha
        for (int j = 0; j < 4; j++) {
            if (!gpio_get(colunas[j])) {
                gpio_put(linhas[i], 1); // Restaura a linha
                return teclado[i][j];
            }
        }
        gpio_put(linhas[i], 1); // Restaura a linha
    }
    return 0; // Nenhuma tecla pressionada
}

// Função para gerar sinal sonoro
void gerar_sinal_sonoro(uint buzzer_pin, int duration_ms) {
    gpio_put(buzzer_pin, 1);
    sleep_ms(duration_ms);
    gpio_put(buzzer_pin, 0);
}

// Função principal
int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, 9); // Pino de saída para LEDs

    // Configuração do teclado matricial
    configurar_teclado();

    // Configuração do buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    while (true) {
        char tecla = ler_tecla();

        if (tecla) {
            printf("Tecla pressionada: %c\n", tecla);

            switch (tecla) {
                case 'A': // Desliga todos os LEDs
                    acionar_leds(pio, sm, 0.0, 0.0, 0.0, 0.0);
                    break;

                case 'B': // LEDs azul, 100% intensidade
                    acionar_leds(pio, sm, 0.0, 0.0, 1.0, 1.0);
                    break;

                case 'C': // LEDs vermelho, 80% intensidade
                    acionar_leds(pio, sm, 1.0, 0.0, 0.0, 0.8);
                    break;

                case 'D': // LEDs verde, 50% intensidade
                    acionar_leds(pio, sm, 0.0, 1.0, 0.0, 0.5);
                    break;

                case '#': // LEDs branco, 20% intensidade
                    acionar_leds(pio, sm, 1.0, 1.0, 1.0, 0.2);
                    break;

                case '*': // Reboot no modo de gravação
                    printf("Entrando no modo de gravação...\n");
                    sleep_ms(500);
                    reset_usb_boot(0, 0);
                    break;

                case '0': // Mostra o padrão do coração
                    desenhar_padrao(pio, sm, desenhoCoracao);
                    break;

                default:
                    // Opcional: Adicione outros casos conforme necessário
                    break;
            }

            // Gera sinal sonoro ao acionar qualquer tecla
            gerar_sinal_sonoro(BUZZER_PIN, 100); // 100ms de som

            sleep_ms(200); // Pequeno atraso para evitar múltiplas leituras
        }
    }

    return 0;
} 