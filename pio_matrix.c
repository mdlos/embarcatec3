#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"

// Inclusão do programa PIO para LEDs
#include "pio_matrix.pio.h"

// Configuração de parâmetros
#define NUM_PIXELS 25
#define BUZZER_PIN 21
#define LED_COUNT 25

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

// Definindo o padrão do coração para a matriz de LEDs (25 LEDs)
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

void execute_command(const char *command, PIO pio, uint sm) {
    if (strcmp(command, "A") == 0) {
        printf("Comando: A\n");
        // Ação para o comando A, apagar LEDs
    } else if (strcmp(command, "B") == 0) {
        printf("Comando: B\n");
        // Ação para o comando B
    } else if (strcmp(command, "C") == 0) {
        printf("Comando: C\n");
        // Ação para o comando C
    } else if (strcmp(command, "D") == 0) {
        printf("Comando: D\n");
        // Ação para o comando D
    } else if (strcmp(command, "#") == 0) {
        gpio_put(BUZZER_PIN, true);  // Liga o buzzer
        sleep_ms(500);  // Buzzer ativo por 500ms
        gpio_put(BUZZER_PIN, false); // Desliga o buzzer

        // Ação para o comando #
        printf("Comando: #\n");
        
        // Aciona todos os LEDs na cor branca (RGB: 1.0, 1.0, 1.0)
        acionar_leds(pio, sm, 1.0, 1.0, 1.0, 1.0);  // Cor branca com intensidade máxima
    } else if (strcmp(command, "0") == 0) {
        printf("Comando: 0 - Desenhando coração\n"); 
        // Desenho do padrão de coração
        desenhar_padrao(pio, sm, desenhoCoracao);
    } else if (strcmp(command, "*") == 0) {
        // Reboot do RP2040 (habilita o modo de gravação via software)
        printf("Reiniciando...\n");
        reset_usb_boot(0, 0);  // Inicia o reboot do sistema e coloca o RP2040 no modo de gravação
    } else {
        printf("Comando inválido: %s\n", command);
    }
}

// Função principal
int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    printf("Sistema inicializado. Aguarde comandos...\n");

    // Inicializa o PIO para controlar a matriz de LEDs
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
        char tecla = ler_tecla(); // Leitura da tecla

        if (tecla) {
            printf("Tecla pressionada: %c\n", tecla);  // Imprime a tecla pressionada no terminal serial

            switch (tecla) {
                case 'A': // LEDs apagados
                    execute_command("A", pio, sm);
                    acionar_leds(pio, sm, 0.0, 0.0, 0.0, 1.0); // Desliga os LEDs
                    break;
                case 'B': // LEDs azuis
                    execute_command("B", pio, sm);
                    acionar_leds(pio, sm, 0.0, 0.0, 1.0, 1.0); // Acende os LEDs azuis
                    break;
                case 'C': // LEDs vermelhos
                    execute_command("C", pio, sm);
                    acionar_leds(pio, sm, 1.0, 0.0, 0.0, 1.0); // Acende os LEDs vermelhos
                    break;
                case 'D': // LEDs verdes
                    execute_command("D", pio, sm);
                    acionar_leds(pio, sm, 0.0, 1.0, 0.0, 1.0); // Acende os LEDs verdes
                    break;
                case '0': // Desenha coração
                    execute_command("0", pio, sm);
                    desenhar_padrao(pio, sm, desenhoCoracao); // Desenha o padrão de coração
                    break;
                case '#': // Sinal sonoro
                    execute_command("#", pio, sm);
                    acionar_leds(pio, sm, 1.0, 1.0, 1.0, 1.0); // Acende todos os LEDs com whiter na intensidade 20
                    gerar_sinal_sonoro(BUZZER_PIN, 500); // Gera um sinal sonoro
                    break;
                case '*': // Reiniciar o sistema
                    execute_command("*", pio, sm);
                    break;
                default:
                    printf("Comando desconhecido.\n");
                    break;
            }
        }
        sleep_ms(200); // Aguarda um pouco antes de verificar novamente
    }

    return 0;
}
