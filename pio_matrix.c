//Embarcatec 
//Tarefa 1 - Aula Sincrona - 20/01/2025
//Discentes: Alexsami
//           João Paulo
//           Sara 
//           Paola
//           Pedro
//           Jvrsoare
//           Márcio de Arruda Fonseca
//           Moises Amorim
//           Ramom Silva

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

//arquivo .pio
#include "pio_matrix.pio.h"

//número de LEDs
#define NUM_PIXELS 25

//pino de saída
#define OUT_PIN 7

#define DEBOUNCE_DELAY 5 // Delay para debounce de teclas em milissegundos

#define FRAME_COUNT 5
#define FRAME_SIZE 25
#define NUM_PIXELS 25

#define PIXEL_INTENSITY 0.2 // 1 equivale a 100% e 0.5 a 50%, se for testar na placa não ponha em 1 pois a intensidade é muito alta e pode prejudicar a visão

//botão de interupção
const uint button_0 = 5;
const uint button_1 = 14;

// Mapeamento do teclado matricial
const uint8_t colunas[4] = {4, 3, 2, 1}; // Pinos das colunas
const uint8_t linhas[4] = {5, 6, 8, 9};  // Pinos das linhas
const char teclado[4][4] = 
{
  {'1', '2', '3', 'A'}, 
  {'4', '5', '6', 'B'}, 
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

/*
//vetor para criar imagem na matriz de led - 2
double desenho2[25] =   {1.0, 0.0, 0.0, 0.0, 1.0,
                        0.0, 1.0, 0.0, 1.0, 0.0, 
                        0.0, 0.0, 1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 1.0, 0.0,
                        1.0, 0.0, 0.0, 0.0, 1.0};
*/

//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255 * PIXEL_INTENSITY;
  G = g * 255 * PIXEL_INTENSITY;
  B = b * 255 * PIXEL_INTENSITY;
  return (R << 24) | (G << 16) | (B << 8);
}

uint32_t matrix_rgb_dois(double b, double r, double g, double intensity)
{
  unsigned char R, G, B;
  R = r * 255 * intensity;
  G = g * 255 * intensity;
  B = b * 255 * intensity;
  return (R << 24) | (G << 16) | (B << 8);
}

// Função para acionar a matriz de LEDs - ws2812b
void desenho_pio(double frame[FRAME_SIZE][3], uint32_t valor_led, PIO pio, uint sm) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        double r = frame[i][0];
        double g = frame[i][1];
        double b = frame[i][2];
        valor_led = matrix_rgb(r, g, b);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

void acender_leds_pio(double frame[FRAME_SIZE][3], uint32_t valor_led, PIO pio, uint sm, double intensity) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        double r = frame[i][0];
        double g = frame[i][1];
        double b = frame[i][2];
        valor_led = matrix_rgb_dois(r, g, b, intensity);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para configurar o teclado
void configurar_teclado() {
    for (int i = 0; i < 4; i++) {
        gpio_init(colunas[i]);
        gpio_set_dir(colunas[i], GPIO_OUT);
        gpio_put(colunas[i], 1); // Inicializa as colunas com 1 (desligado)
    }

    for (int i = 0; i < 4; i++) {
        gpio_init(linhas[i]);
        gpio_set_dir(linhas[i], GPIO_IN);
        gpio_pull_up(linhas[i]); // Habilita pull-up para evitar leituras erradas
    }
}

// Função para ler o teclado
char leitura_teclado() {
    char tecla = 'n'; // Inicializa com 'n' (nenhuma tecla pressionada)

    for (int coluna = 0; coluna < 4; coluna++) {
        gpio_put(colunas[coluna], 0); // Ativa a coluna (coloca como 0)

        for (int linha = 0; linha < 4; linha++) {
            if (gpio_get(linhas[linha]) == 0) { // Se uma tecla foi pressionada
                tecla = teclado[3 - linha][coluna]; // Mapeia a tecla pressionada
                while (gpio_get(linhas[linha]) == 0) { // Espera a tecla ser liberada (debounce)
                    sleep_ms(10); // Atraso para evitar múltiplas leituras
                }
                break;
            }
        }

        gpio_put(colunas[coluna], 1); // Desativa a coluna (coloca como 1)

        if (tecla != 'n') break; // Sai do laço se uma tecla foi pressionada
    }

    return tecla; // Retorna a tecla pressionada
}


//imprimir valor binário
void imprimir_binario(int num) {
 int i;
 for (i = 31; i >= 0; i--) {
  (num & (1 << i)) ? printf("1") : printf("0");
 }
}

//rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){
    printf("Interrupção ocorreu no pino %d, no evento %d\n", gpio, events);
    printf("HABILITANDO O MODO GRAVAÇÃO");
	reset_usb_boot(0,0); //habilita o modo de gravação do microcontrolador
}


// Vetores com os dados da animação (intensidade de cada cor para cada pixel)
static double frames_alexsami[FRAME_COUNT][FRAME_SIZE][3] = {
// Frame 1
    {
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.953},
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950},
        {0.918, 0.455, 0.059}, {0.973, 0.976, 0.973}, {0.070, 0.710, 0.949}, {0.918, 0.459, 0.059}, {0.976, 0.973, 0.976},
        {0.973, 0.976, 0.976}, {0.973, 0.976, 0.973}, {0.070, 0.710, 0.949}, {0.976, 0.973, 0.973}, {0.976, 0.973, 0.973},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.950}
        
    },
    // Frame 2
    {
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950},
        {0.973, 0.976, 0.973}, {0.922, 0.455, 0.059}, {0.070, 0.710, 0.949}, {0.976, 0.976, 0.976}, {0.922, 0.459, 0.059},
        {0.976, 0.976, 0.976}, {0.973, 0.973, 0.976}, {0.070, 0.710, 0.950}, {0.976, 0.976, 0.976}, {0.973, 0.976, 0.973},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950}
        
    },
    // Frame 3
    {
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.950},
        {0.070, 0.710, 0.953}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953},
        {0.976, 0.973, 0.976}, {0.973, 0.973, 0.976}, {0.070, 0.710, 0.949}, {0.973, 0.976, 0.973}, {0.976, 0.976, 0.973},
        {0.922, 0.455, 0.059}, {0.976, 0.976, 0.976}, {0.070, 0.710, 0.949}, {0.922, 0.455, 0.059}, {0.973, 0.976, 0.973},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.953}
        
    },
    // Frame 4
    {
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949},
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.949},
        {0.976, 0.976, 0.976}, {0.973, 0.973, 0.976}, {0.070, 0.710, 0.950}, {0.973, 0.973, 0.976}, {0.976, 0.973, 0.976},
        {0.976, 0.976, 0.976}, {0.918, 0.459, 0.059}, {0.070, 0.710, 0.949}, {0.973, 0.976, 0.973}, {0.922, 0.455, 0.059},
        {0.070, 0.710, 0.953}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.950}
        
    },
    // Frame 5
    {
        {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950}, {0.126, 0.104, 0.693}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.950},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949}, {0.070, 0.710, 0.949},
        {0.973, 0.976, 0.973}, {0.973, 0.973, 0.976}, {0.070, 0.710, 0.949}, {0.973, 0.973, 0.976}, {0.973, 0.976, 0.973},
        {0.922, 0.455, 0.059}, {0.973, 0.976, 0.973}, {0.070, 0.710, 0.950}, {0.918, 0.455, 0.059}, {0.973, 0.973, 0.976},
        {0.070, 0.710, 0.949}, {0.070, 0.710, 0.950}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.953}, {0.070, 0.710, 0.949}
        
    }

};

// Vetores com os dados da animação (intensidade de cada cor para cada pixel)
static double frames_coracao[5][FRAME_SIZE][3] = {
    // frame 1
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}
        
    },
    // frame 2
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}
        
    },
    // frame 3
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}
        
    },
    // frame 4
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}
        
    },
    // frame 5
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}
        
    },
};


// Animação feita por José Vinicius, ao apertar a tecla 0, irá aparecer o nome "EMBARCATECH"

static double frames_embarcatech[11][FRAME_SIZE][3] = {
    // Frame 'E'
    {
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
    // Frame 'M'
    {
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Frame 'B'
    {
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Frame 'A'
    {
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}
    },
   // Frame 'R'
    {
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Frame 'C'
    {
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Segundo 'A'
    {
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}
    },
   // Frame 'T'
    {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
    // Segundo 'E'
    {
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Segundo 'C'
    {
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    },
   // Frame 'H'
    {
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}
    }
};

// Animações feitas por Moises
static double frames_estrela[1][FRAME_SIZE][3] = {
        // Frames 1 
        {

          {0, 0, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1, 0}, {0, 0, 0},
          {0, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1, 0}, {0, 1, 0},
          {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1},
          {0, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1, 0}, {0, 1, 0},
          {0, 0, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1, 0}, {0, 0, 0}
        },

        
   };



    // Frames 1 
    /*{
        0.0, 0.0, 0.6, 0.0, 0.0, // Topo da estrela
        0.6, 0.0, 0.0, 0.0, 0.6, // Braços laterais superiores
        0.0, 0.6, 0.0, 0.6, 0.0, // Centro vazio
        0.6, 0.0, 0.0, 0.0, 0.6, // Braços laterais inferiores
        0.0, 0.0, 0.6, 0.0, 0.0  // Base da estrela
    },
     //Frames 2
    {
        0.0, 0.0, 1.0, 0.0, 0.0, // Topo cheio
        1.0, 1.0, 1.0, 1.0, 1.0, // Braços laterais superiores preenchidos
        0.0, 1.0, 1.0, 1.0, 0.0, // Centro totalmente preenchido
        1.0, 1.0, 1.0, 1.0, 1.0, // Braços laterais inferiores preenchidos
        0.0, 1.0, 0.0, 1.0, 0.0  // Base preenchida
    },
};*/

//Animações de operações matemáticas feitas por Paola Fagundes
static double frames_operacoes_paola[5][5][3] = {
    // Frame 1 - Soma
    {{2148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},

    // Frame 2 - Subtração
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},

    // Frame 3 - Multiplicação
    {{255, 0, 0}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {255, 0, 0}},
    {{148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}},
    {{255, 0, 0}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {255, 0, 0}},

    // Frame 4 - Divisão
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},

    // Frame 5 - Igual
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}}
};

// Matriz de frames para a animação do rosto
static double frames_piscar_olho[7][25][3] = {
    // Frame 1
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 2 - fechando olho
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 3 - olho quase fechado
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 4 (Olho fechado)
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 5 (Reabrindo)
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 6 (Quase aberto)
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    },
    // Frame 7 (Aberto total)
    {
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
    }
};

static double leds_vermelhos[1][FRAME_SIZE][3] = {
    {
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}
    } 
};

// Função para exibir animação das letras "EMBARCATECH"
void exibir_animacao_embarcatech(PIO pio, uint sm) {
    for (int i = 0; i < 11; i++) { // 11 letras em "EMBARCATECH"
        desenho_pio(frames_embarcatech[i], 0, pio, sm);
        sleep_ms(1500); // Delay de 1 segundo entre cada letra
    }
}

// Função para acionar a matriz de LEDs - ws2812b
void apagar_matriz_leds( PIO pio, uint sm) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        uint32_t valor_led = matrix_rgb(0, 0, 0);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para exibir animação
void exibir_animacao_alexsami(PIO pio, uint sm) {
  int frame_idx = 0;
    for (frame_idx = 0; frame_idx < FRAME_COUNT; frame_idx++) {
        desenho_pio(frames_alexsami[frame_idx], 0, pio, sm);
        sleep_ms(130); // Pausa entre os frames
    }
}

void exibir_animacao_sara(PIO pio, uint sm){
    for (int i = 0; i < 5; i++) {
        desenho_pio(frames_coracao[i], 0, pio, sm);
        sleep_ms(600); 
    }
}

void exibir_animacao_estrela(PIO pio, uint sm) {
    for (int i = 0; i < 1; i++) { // Repetir animação 3 vezes
        desenho_pio(frames_estrela[i], 0, pio, sm); 
        sleep_ms(1000);
        
    }
}

void exibir_animacao_operacoes_paola(PIO pio, uint sm) {
    for (int i = 0; i < 5; i++) {  // Exibir cada um dos 5 frames
        desenho_pio(frames_operacoes_paola[i], 0, pio, sm);  // Passa os dados de cada frame
        sleep_ms(600);  // Pausa entre os frames
    }
}

// Função para exibir a animação
void exibir_animacao_piscar_olho_ramom(PIO pio, uint sm) {
    for(int i = 0; i < 3; i++) {
        for (int frame_idx = 0; frame_idx < 7; frame_idx++) {
            desenho_pio(frames_piscar_olho[frame_idx], 0, pio, sm);
            sleep_ms(200); // Pausa entre os frames
        }
        sleep_ms(500); // Pausa entre as piscadas
    }
}

void ligar_todos_leds_vemelhos_intessidade_alta_ramom(PIO pio, uint sm) {
    
    acender_leds_pio(leds_vermelhos[0], 0, pio, sm, 0.8);
}
// Função para exibir LEDs verdes com 50% de intensidade
void exibir_pedro(PIO pio, uint sm) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        // Define o valor do LED com 50% de intensidade para a cor verde
        uint32_t valor_led = matrix_rgb(0, 0.5, 0); // RGB: Verde
        pio_sm_put_blocking(pio, sm, valor_led);
    }
    sleep_ms(1000);
}


//função principal
int main()
{
  PIO pio = pio0; 
  bool ok;
  uint16_t i;
  uint32_t valor_led;
  double r = 0.0, b = 0.0 , g = 0.0;
  bool apagar = true;

  //coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
  ok = set_sys_clock_khz(128000, false);

  // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
  stdio_init_all();

  printf("iniciando a transmissão PIO");
  if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

  //configurações da PIO
  uint offset = pio_add_program(pio, &pio_matrix_program);
  uint sm = pio_claim_unused_sm(pio, true);
  pio_matrix_program_init(pio, sm, offset, OUT_PIN);

  //inicializar o botão de interrupção - GPIO5
  gpio_init(button_1);
  gpio_set_dir(button_1, GPIO_IN);
  gpio_pull_up(button_1);

  //inicializar o botão de interrupção - GPIO5
  gpio_init(button_1);
  gpio_set_dir(button_1, GPIO_IN);
  gpio_pull_up(button_1);

  //interrupção da gpio habilitada
  gpio_set_irq_enabled_with_callback(button_1, GPIO_IRQ_EDGE_FALL, 1, & gpio_irq_handler);

  configurar_teclado();
  while (true) {

    char tecla = leitura_teclado();

    if (tecla != 'n') { // Verifica se uma tecla foi pressionada
      printf("Tecla pressionada: %c\n", tecla);

      switch (tecla) {
        case 'A':
            apagar = true;
        break;
        case 'B':
            //desenho_pio(desenho2, valor_led, pio, sm, r, g, b);
            sleep_ms(1000);
            break;
        case 'C':
            ligar_todos_leds_vemelhos_intessidade_alta_ramom(pio, sm);
            apagar = false;
            break;
        case 'D':
            exibir_pedro(pio,sm);
            break;
        case '#':
            apagar_matriz_leds(pio, sm);
            break;
        case '*':
            reset_usb_boot(0,0);
            break;
        case '0':
            exibir_animacao_embarcatech(pio, sm);
            sleep_ms(1000);
            apagar = true;
            break;
        case '1':
            exibir_animacao_piscar_olho_ramom(pio, sm);
            sleep_ms(1000);
            break;
        case '2':
               exibir_animacao_estrela(pio, sm);
               sleep_ms(1000);
               apagar = true;
            break;
        case '3':

            break;    
        case '4':

            break;
        case '5':
		exibir_animacao_operacoes_paola(pio, sm);
            	sleep_ms(1000);
                apagar = true;
		break;
        case '6':

            break;
        case '7':
            exibir_animacao_alexsami(pio, sm);
            sleep_ms(1000);
            apagar = true;
            break;
        case '8':
            exibir_animacao_sara(pio, sm);
            sleep_ms(1000);
            apagar = true;
            break;
        case '9':

            break;
        default:
            //desenho_pio(desenho2, valor_led, pio, sm, r, g, b);
            break;
      }
    }
    sleep_ms(DEBOUNCE_DELAY); // Delay para debounce
    //exibir_animacao_alexsami(pio, sm); //botão USADO PARA TESTES NA PLACA FÍSICA
    if(apagar){
        apagar_matriz_leds(pio, sm);
    }

  
  }
  return 0;
}