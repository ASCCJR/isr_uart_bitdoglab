#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h> // Para usar strcmp()

// Define o pino do LED
#define LED_VERMELHO 13

// Define o tamanho máximo para os comandos (ex: "OFF" + '\0')
#define MAX_COMMAND_LEN 10

// Variável global para contar caracteres recebidos
int g_char_count_nb = 0;

// Buffer para armazenar o comando sendo digitado
char g_command_buffer_nb[MAX_COMMAND_LEN];
int g_command_idx_nb = 0;

// --- Funções para os padrões de LED ---
void executar_padrao_A() {
    printf("LED: Padrão A (pisca 3 vezes rápido)\n");
    for (int i = 0; i < 3; i++) {
        gpio_put(LED_VERMELHO, 1);
        sleep_ms(100);
        gpio_put(LED_VERMELHO, 0);
        sleep_ms(100);
    }
}

void executar_padrao_B() {
    printf("LED: Padrão B (pisca 1 vez longo)\n");
    gpio_put(LED_VERMELHO, 1);
    sleep_ms(500);
    gpio_put(LED_VERMELHO, 0);
    sleep_ms(100); // Pequena pausa antes de apagar completamente
}

void executar_padrao_default() {
    printf("LED: Padrão Default (pisca uma vez)\n");
    gpio_put(LED_VERMELHO, 1);
    sleep_ms(200);
    gpio_put(LED_VERMELHO, 0);
}
// --- Fim das funções de padrão de LED ---

int main() {
    // Inicializa stdio para comunicação (USB ou UART)
    stdio_init_all();

    // Pequena pausa para dar tempo ao terminal serial de se conectar
    // (Especialmente útil ao alimentar o Pico e abrir o terminal rapidamente)
    for(int i=0; i<20; ++i) { // ~2 segundos de espera piscando o LED onboard se houver
        #ifdef PICO_DEFAULT_LED_PIN
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        #endif
        sleep_ms(100);
    }


    printf("\r\n Pico pronto (Recepção Não Bloqueante).\r\n");
    printf("Digite comandos (ex: A, B, ON, OFF) e pressione Enter.\r\n");

    // Inicializa o pino do LED VERMELHO
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0); // Começa com LED apagado

    while (true) {
        // Tenta ler um caractere da entrada serial com timeout de 0 (não bloqueante)
        int ch_int = getchar_timeout_us(0);

        // Verifica se um caractere foi realmente recebido
        if (ch_int != PICO_ERROR_TIMEOUT && ch_int != EOF && ch_int >= 0) {
            g_char_count_nb++; // Incrementa o contador global de caracteres
            char c = (char)ch_int;

            // Echo do caractere de volta para o terminal para melhor UX
            // (Exceto para CR que pode duplicar o efeito do LF)
            if (c == '\r') {
                // Não ecoar CR, pois LF (newline) já lida com a nova linha
            } else if (c == '\n') {
                putchar_raw('\r'); // Adiciona Carriage Return antes do Line Feed
                putchar_raw(c);    // Ecoa Line Feed
                fflush(stdout);    // Garante que o caractere seja enviado imediatamente
            } else if (c == 127 || c == 8) { // ASCII para DEL ou Backspace
                 if (g_command_idx_nb > 0) {
                    g_command_idx_nb--; // Remove o último caractere do buffer
                    // Envia backspace, espaço, backspace para apagar no terminal
                    putchar_raw('\b');
                    putchar_raw(' ');
                    putchar_raw('\b');
                    fflush(stdout);
                }
            }
            else {
                putchar_raw(c); // Ecoa o caractere normal
                fflush(stdout);
            }

            // Verifica se o caractere é um terminador de comando (Enter)
            if (c == '\n' || c == '\r') {
                if (g_command_idx_nb > 0) { // Verifica se há algo no buffer
                    g_command_buffer_nb[g_command_idx_nb] = '\0'; // Termina a string do comando
                    printf("Processando: \"%s\" | Total chars até agora: %d\r\n", g_command_buffer_nb, g_char_count_nb);

                    // Compara o comando com os comandos conhecidos
                    if (strcmp(g_command_buffer_nb, "ON") == 0) {
                        gpio_put(LED_VERMELHO, 1);
                        printf("LED: Ligado\r\n");
                    } else if (strcmp(g_command_buffer_nb, "OFF") == 0) {
                        gpio_put(LED_VERMELHO, 0);
                        printf("LED: Desligado\r\n");
                    } else if (strcmp(g_command_buffer_nb, "A") == 0) {
                        executar_padrao_A();
                    } else if (strcmp(g_command_buffer_nb, "B") == 0) {
                        executar_padrao_B();
                    }
                    // Adicione mais 'else if' aqui para outros comandos/padrões
                    else {
                        printf("Comando desconhecido: \"%s\"\r\n", g_command_buffer_nb);
                        executar_padrao_default(); // Executa um padrão default para comando não reconhecido
                    }
                    g_command_idx_nb = 0; // Reseta o índice do buffer para o próximo comando
                    memset(g_command_buffer_nb, 0, MAX_COMMAND_LEN); // Limpa o buffer
                } else {
                    // Se apenas Enter foi pressionado sem digitar nada
                    printf("Comando vazio recebido.\r\n");
                }
                 printf("Digite o próximo comando:\r\n"); // Prompt para novo comando
            } else if (c != 127 && c != 8) { // Se não for Enter nem Backspace/DEL
                // Adiciona o caractere ao buffer se houver espaço
                if (g_command_idx_nb < MAX_COMMAND_LEN - 1) {
                    g_command_buffer_nb[g_command_idx_nb++] = c;
                } else {
                    // Buffer cheio, informa o usuário e descarta o comando atual
                    printf("\r\nErro: Buffer de comando cheio (max %d chars). Comando descartado.\r\n", MAX_COMMAND_LEN - 1);
                    g_command_idx_nb = 0; // Reseta o buffer
                    memset(g_command_buffer_nb, 0, MAX_COMMAND_LEN);
                    printf("Digite o próximo comando:\r\n");
                }
            }
        }
    }
    return 0; // Esta linha normalmente não é alcançada em um sistema embarcado
}