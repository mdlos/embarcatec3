#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"

// Arquivo .pio
#include "pio_matrix.pio.h"

// Definições para o teclado matricial
const uint ROWS[] = {1, 2, 3, 4}; // GPIO pins for rows
const uint COLS[] = {5, 6, 8, 9}; // GPIO pins for columns
const char keys[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

#define NUM_ROWS 4
#define NUM_COLS 4

// Número de LEDs
#define NUM_PIXELS 25

// Pino de saída
#define OUT_PIN 7

// Brilho fixo para todas as letras
#define BRILHO 0.1
double r = BRILHO, g = 0.0, b = 0.0; // Cor vermelha com brilho 0.1

// Matrizes das letras "EMBARCATECH" + ♥
double letra_E[25] = {0.1, 0.1, 0.1, 0.1, 0.0,
                      0.0, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.0,
                      0.0, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.0};

double letra_M[25] = {0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.0, 0.1, 0.1,
                      0.1, 0.0, 0.1, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1};

double letra_B[25] = {0.1, 0.1, 0.1, 0.1, 0.0,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.0,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.0};

double letra_A[25] = {0.0, 0.1, 0.1, 0.1, 0.0,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1};

double letra_R[25] = {0.1, 0.1, 0.1, 0.1, 0.1,
                      0.0, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.0,
                      0.0, 0.0, 0.1, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1};

double letra_C[25] = {0.1, 0.1, 0.1, 0.1, 0.1,
                      0.0, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.0,
                      0.0, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.1};

double letra_T[25] = {0.1, 0.1, 0.1, 0.1, 0.1,
                      0.0, 0.0, 0.1, 0.0, 0.0,
                      0.0, 0.0, 0.1, 0.0, 0.0,
                      0.0, 0.0, 0.1, 0.0, 0.0,
                      0.0, 0.0, 0.1, 0.0, 0.0};

double letra_H[25] = {0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1,
                      0.1, 0.0, 0.0, 0.0, 0.1};

double coracao[25] = {0.0, 0.1, 0.0, 0.1, 0.0,
                      0.1, 0.1, 0.1, 0.1, 0.1,
                      0.1, 0.1, 0.1, 0.1, 0.1,
                      0.0, 0.1, 0.1, 0.1, 0.0,
                      0.0, 0.0, 0.1, 0.0, 0.0};

 double vazio[25] = {0.0, 0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0, 0.0};

// Sequência de letras "EMBARCATECH" + ♥ 
double *letras[] = {letra_E, letra_M, letra_B, letra_A, letra_R, letra_C, letra_A, letra_T, letra_E, letra_C, letra_H, coracao, vazio, coracao, vazio, coracao, vazio, coracao, coracao, coracao, vazio};
int total_letras = 21;

// Função para definir a cor do LED (vermelho com brilho fixo)
uint32_t matrix_rgb()
{
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para exibir uma letra na matriz de LEDs
void desenho_pio(double *desenho, PIO pio, uint sm)
{
    uint32_t valor_led = matrix_rgb(); // Vermelho com brilho 0.1
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (desenho[24 - i] == 0.1)
            pio_sm_put_blocking(pio, sm, valor_led); // Acende o LED
        else
            pio_sm_put_blocking(pio, sm, 0); // Mantém apagado
    }
}

// Função para ler o teclado matricial com debounce
char get_key(void)
{
    char key = '\0';
    for (int row = 0; row < NUM_ROWS; row++) {
        gpio_put(ROWS[row], 1);
        for (int col = 0; col < NUM_COLS; col++) {
            if (gpio_get(COLS[col]) == 1) {
                key = keys[row][col];
                // Debounce básico
                sleep_ms(200);
                return key;
            }
        }
        gpio_put(ROWS[row], 0);
    }
    return key;
}

char comando[10]; // Array para armazenar a entrada do usuário
int current_letter_index = 0; // Índice da letra atual na animação
bool animating = false;       // Flag para controlar se estamos em animação
bool command_entered = true;  // Flag para indicar se um comando foi digitado

int main() {
    PIO pio = pio0;
    bool ok;

    // Define o clock para 128 MHz
    ok = set_sys_clock_khz(128000, false);

    // Inicializa a comunicação serial
    stdio_init_all();

    // Configuração da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    // Configuração do teclado matricial
    for (int i = 0; i < NUM_ROWS; i++) {
        gpio_init(ROWS[i]);
        gpio_set_dir(ROWS[i], GPIO_OUT);
        gpio_put(ROWS[i], 0);
    }
    for (int i = 0; i < NUM_COLS; i++) {
        gpio_init(COLS[i]);
        gpio_set_dir(COLS[i], GPIO_IN);
        gpio_pull_down(COLS[i]);
    }

    double leds_apagados[25] = {0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0};

    // Exibe a mensagem de solicitação de comando assim que o programa começa
    printf("Digite um comando ('0' para animação, 'A' ou 'a' para apagar): ");
    fflush(stdout);

    while (true) {
        // Verifica entrada a cada ciclo
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            comando[0] = ch;
            comando[1] = '\0'; // Garantir que comando é uma string de 1 caractere
            printf("Comando digitado: %s\n", comando);
            command_entered = true;  // Marca que um comando foi digitado

            if (ch == 'A' || ch == 'a') {
                desenho_pio(leds_apagados, pio, sm);  // Apaga LEDs imediatamente
                animating = false;  // Para a animação
                current_letter_index = 0;  // Reinicia a animação para o caso de começar novamente
            } else if (ch == '0') {
                animating = true;   // Inicia a animação
            }
        }

        // Se estiver animando, exibe a letra atual
        if (animating && current_letter_index < total_letras) {
            desenho_pio(letras[current_letter_index], pio, sm);
            current_letter_index++;  // Avança para a próxima letra
            // Aguarda 1 segundo para passar para a próxima letra, mas verifica entrada antes de continuar
            for (int i = 0; i < 10; ++i) {  // Loop para verificar a entrada a cada 100ms (10 vezes em 1 segundo)
                sleep_ms(100);
                int check_ch = getchar_timeout_us(0);
                if (check_ch == 'A' || check_ch == 'a') {
                    animating = false;
                    current_letter_index = 0;
                    desenho_pio(leds_apagados, pio, sm);  // Apaga LEDs se 'A' for pressionado durante animação
                    break;
                }
            }
        } else if (!animating) {
            // Se não estiver animando, mantém os LEDs apagados
            desenho_pio(leds_apagados, pio, sm);
            current_letter_index = 0;  // Reinicia a animação para o próximo '0'
        }

        // Se a animação terminou, para de animar
        if (current_letter_index >= total_letras) {
            animating = false;
            current_letter_index = 0;  // Reinicia para o próximo ciclo
        }

        // Mostra a mensagem de solicitação de comando após um comando ter sido dado
        if (command_entered) {
            printf("Digite um comando ('0' para animação, 'A' ou 'a' para apagar): ");
            fflush(stdout);
            command_entered = false;
        }

        // Adicionando um pequeno delay para evitar leituras rápidas múltiplas
        sleep_ms(10);  // Um pequeno delay para não saturar a entrada
    }
    return 0;
}