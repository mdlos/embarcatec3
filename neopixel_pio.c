#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Matrizes das operações matemáticas
int soma[5][5][3] = {
    {{2148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}}};

int subtracao[5][5][3] = {
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}}};

int divisao[5][5][3] = {
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}}};

int igual[5][5][3] = {
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}}};

int multiplicacao[5][5][3] = {
    {{255, 0, 0}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {255, 0, 0}},
    {{148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}},
    {{148, 0, 211}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {148, 0, 211}},
    {{148, 0, 211}, {255, 0, 0}, {148, 0, 211}, {255, 0, 0}, {148, 0, 211}},
    {{255, 0, 0}, {148, 0, 211}, {148, 0, 211}, {148, 0, 211}, {255, 0, 0}}};

// Função para exibir uma matriz de LEDs
void exibirMatriz(int matriz[5][5][3]) {
  for (int linha = 0; linha < 5; linha++) {
    for (int coluna = 0; coluna < 5; coluna++) {
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
    }
  }
  npWrite();
}

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); 
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

int getIndex(int x, int y) {
  if (y % 2 == 0) {
    return 24 - (y * 5 + x); // Linha par.
  } else {
    return 24 - (y * 5 + (4 - x)); // Linha ímpar.
  }
}

int main() {
  stdio_init_all();
  npInit(LED_PIN);
  npClear();

  char input;

  while (true) {
    input = getchar(); // Captura apenas um caractere
    getchar();

    // Processa o comando
    if (input == '0') {
      exibirMatriz(soma);
      sleep_ms(3000);
      exibirMatriz(subtracao);
      sleep_ms(3000);
      exibirMatriz(divisao);
      sleep_ms(3000);
      exibirMatriz(igual);
      sleep_ms(3000);
      exibirMatriz(multiplicacao);
      sleep_ms(3000);
    } else if (input == 'b') {
      for (int i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 0, 0, 255);
      }
      npWrite();
    } else if (input == 'a') {
      npClear();
      npWrite();
    }

    printf("Digite um comando: ");
  }
}
