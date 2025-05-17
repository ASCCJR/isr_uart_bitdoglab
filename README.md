# isr_uart_bitdoglab


# Controle Interativo de LED via Serial no Raspberry Pi Pico

Este projeto demonstra como controlar um LED conectado a um Raspberry Pi Pico através de comandos enviados via terminal serial (USB). O programa utiliza recepção não bloqueante, permitindo uma interface de usuário interativa com eco de caracteres, suporte a backspace e feedback sobre os comandos processados.

## 🎯 Funcionalidades Solicitadas e Implementadas

Este projeto foi desenvolvido a partir de um código base (apresentado na seção seguinte) com o objetivo de adicionar as seguintes funcionalidades, que foram implementadas na versão final:

1.  **Padrões de LED Variados:** O LED pisca em diferentes padrões dependendo do comando recebido.
2.  **Contagem de Caracteres:** O sistema conta e exibe o total de caracteres recebidos pela interface serial.
3.  **Comandos Específicos ON/OFF:** Implementados comandos "ON" para acender o LED continuamente e "OFF" para apagá-lo.
4.  **Recepção Não Bloqueante:** A entrada de comandos do usuário é feita de forma não bloqueante.
5.  **Melhorias na Interação:**
    * Eco dos caracteres digitados pelo usuário.
    * Suporte à tecla Backspace para corrigir a entrada de comandos.
    * Mensagens claras sobre o status e os comandos processados.

## Código Base Original

O desenvolvimento partiu do seguinte código mais simples, que lia caracteres da serial de forma bloqueante e piscava o LED uma única vez para cada caractere recebido:

```c
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_VERMELHO 13  // LED vermelho da BitDogLab (ânodo comum)

int main() {
    stdio_init_all();        // Inicializa USB CDC
    sleep_ms(2000);          // Aguarda USB estabilizar
    printf("🟢 Pico pronto. Digite algo no terminal USB...\n");

    // Configura LED como saída (inicialmente apagado: nível alto)
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);  // Apagado (ânodo comum)

    while (true) {
        // Espera bloqueando até receber um caractere
        int ch = getchar();
        char c = (char)ch;

        // Mostra o caractere recebido e seu código ASCII
        printf("🔹 Recebido: '%c' (ASCII: %d)\n", c, ch);

        // Acende LED por 200ms
        gpio_put(LED_VERMELHO, 0);  // Liga (nível baixo)
        sleep_ms(200);
        gpio_put(LED_VERMELHO, 1);  // Desliga (nível alto)
    }

    return 0;
}
````

*Nota sobre o Código Base: O comentário original indica "ânodo comum", o que implica uma lógica de LED ativo-baixo (nível lógico BAIXO no GPIO acende o LED). A lógica de acionamento no loop do código base (`gpio_put(LED_VERMELHO, 0);` para ligar) é consistente com isso. O código final (descrito neste README) utiliza uma lógica ativo-ALTO para o GPIO 13 (`gpio_put(LED_VERMELHO, 1);` para ligar), o que é compatível com um LED de cátodo comum, como o da placa BitDogLab no GPIO 13.*

## ✨ Recursos Principais (do Código Final)

  * **Controle de LED via Comandos:**
      * `ON`: Liga o LED vermelho permanentemente.
      * `OFF`: Desliga o LED vermelho permanentemente.
      * `A`: Ativa o "Padrão A" (3 piscadas rápidas).
      * `B`: Ativa o "Padrão B" (1 piscada longa).
      * Qualquer outro comando: Ativa um "Padrão Default" (1 piscada).
  * **Recepção Não Bloqueante:** Utiliza `getchar_timeout_us(0)` para ler a entrada serial sem bloquear o loop principal.
  * **Feedback Interativo ao Usuário:**
      * Os caracteres digitados são ecoados de volta no terminal.
      * A tecla Backspace funciona para corrigir a digitação do comando.
      * Mensagens informativas são exibidas, incluindo o comando processado e o total de caracteres recebidos.
  * **Contador de Caracteres:** Mantém uma contagem de todos os caracteres recebidos via serial (incluindo Enter e outros caracteres de controle).
  * **Sinalização de Inicialização:** Durante os primeiros 2 segundos, o LED onboard do Pico (se disponível, geralmente GPIO 25) pisca para indicar que o dispositivo está inicializando e pronto para a conexão serial.

## 🔌 Hardware Necessário e Conexões (para o Código Final)

  * Raspberry Pi Pico.
  * 1x LED Vermelho.
  * 1x Resistor limitador de corrente para o LED (ex: 220Ω, consistente com a configuração da BitDogLab para GPIO 13).
  * Cabo Micro USB para conectar o Pico ao computador.

| Pino Pico         | Componente          | Detalhe                                     |
|-------------------|---------------------|---------------------------------------------|
| GPIO 13           | LED Vermelho (Ânodo) | Conectado ao ânodo do LED. Cátodo ao GND via resistor. (Para LED RGB de cátodo comum da BitDogLab, GPIO 13 é o pino R, e é acionado com nível ALTO). |
| `PICO_DEFAULT_LED_PIN` (Geralmente GPIO 25) | LED Onboard do Pico | Usado para sinalização de inicialização.    |
| Pinos USB         | Computador          | Para comunicação serial e alimentação.     |

*(Nota: O código final acende o LED em GPIO 13 aplicando nível lógico ALTO (`gpio_put(LED_VERMELHO, 1)`). Isso é compatível com um LED onde o ânodo está no GPIO e o cátodo no GND (via resistor), ou com um LED RGB de cátodo comum onde o GPIO 13 controla o canal vermelho.)*

## 🧠 Estrutura do Código (Final Evoluído)

  * **`main()`:**
      * Inicializa `stdio_init_all()` para comunicação serial.
      * Realiza uma pausa inicial com sinalização no LED onboard.
      * Configura o `LED_VERMELHO` (GPIO 13) como saída.
      * Entra em um loop infinito (`while (true)`):
          * Verifica por novos caracteres da serial de forma não bloqueante (`getchar_timeout_us(0)`).
          * Se um caractere é recebido:
              * Incrementa o contador `g_char_count_nb`.
              * Ecoa o caractere para o terminal (com tratamento para Enter e Backspace).
              * Se o caractere for Enter (`\n` ou `\r`), processa o comando acumulado no `g_command_buffer_nb`.
              * Caso contrário, adiciona o caractere ao buffer de comando.
  * **Funções de Padrão de LED:**
      * `executar_padrao_A()`: Define a sequência de 3 piscadas rápidas.
      * `executar_padrao_B()`: Define a sequência de 1 piscada longa.
      * `executar_padrao_default()`: Define a sequência de 1 piscada padrão.
  * **Variáveis Globais:**
      * `g_char_count_nb`: Contador de caracteres.
      * `g_command_buffer_nb`, `g_command_idx_nb`: Buffer e índice para montar os comandos recebidos.
  * **Defines:**
      * `LED_VERMELHO`: Define o pino GPIO para o LED externo.
      * `MAX_COMMAND_LEN`: Define o tamanho máximo do buffer de comando.

### Uso

1.  Após carregar o firmware, o Pico iniciará a comunicação serial via USB.
2.  Abra um programa de terminal serial no seu computador (ex: PuTTY, Minicom, Serial Monitor do Arduino IDE, Thonny).
3.  Configure a conexão serial para a porta COM correspondente ao Pico. A taxa de transmissão (baud rate) padrão é **115200 bps**.
4.  Você deverá ver a mensagem: ` Pico pronto (Recepção Não Bloqueante).`
5.  Digite um dos comandos disponíveis (`ON`, `OFF`, `A`, `B`) e pressione Enter.
6.  Observe o comportamento do LED vermelho e as mensagens de feedback no terminal.

## 📌 Notas Adicionais (sobre o Código Final)

  * **Contagem de Caracteres:** Lembre-se que o "Total chars até agora" inclui todos os caracteres recebidos, incluindo a tecla Enter (que pode ser `\n`, `\r`, ou ambos) e outros caracteres de controle.
  * **Buffer de Comando:** O buffer de comando tem um tamanho máximo definido por `MAX_COMMAND_LEN` (10 caracteres). Comandos mais longos serão descartados.
  * **Comportamento do Terminal:** Diferentes programas de terminal podem enviar diferentes sequências de caracteres para a tecla Enter (`\n`, `\r`, ou `\r\n`). O código tenta lidar com `\n` e `\r` como terminadores de comando.
  * **LED Onboard:** A piscada inicial do LED onboard (geralmente GPIO 25) é útil para confirmar que o Pico ligou e o programa está começando, antes mesmo da conexão serial ser estabelecida.

## Propósito

Este projeto foi desenvolvido com fins estritamente educacionais e aprendizado durante a residência em sistemas embarcados pelo embarcatech
<!-- end list -->
