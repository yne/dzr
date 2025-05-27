# NCurses Cheatsheet

## Índice

- [Inicialização e Finalização](#inicialização-e-finalização)
- [Gerenciamento de Janelas](#gerenciamento-de-janelas)
- [Entrada e Saída](#entrada-e-saída)
- [Atributos e Cores](#atributos-e-cores)
- [Exemplos Práticos](#exemplos-práticos)

## Inicialização e Finalização

### Funções Básicas

- `initscr()`:
  - **Descrição**: Inicializa a biblioteca ncurses e cria uma janela de tela cheia (stdscr).
  - **Parâmetros**: Nenhum.
  - **Retorno**: Um ponteiro para a janela de tela cheia.
  - **Exemplo**:

    
```c
WINDOW *stdscr = initscr();
if (stdscr == NULL) {
    fprintf(stderr, "Erro ao inicializar ncurses\n");
    return 1;
}
```


- `endwin()`:
  - **Descrição**: Restaura o terminal para seu estado original e encerra o uso da biblioteca ncurses.
  - **Parâmetros**: Nenhum.
  - **Retorno**: `OK` em caso de sucesso, `ERR` em caso de erro.
  - **Exemplo**:

    
```c
int result = endwin();
if (result == ERR) {
    fprintf(stderr, "Erro ao finalizar ncurses\n");
}
```


### Modos de Entrada/Saída

- `raw()` e `cbreak()`:
  - **Descrição**:
    - `raw()`: Desabilita o processamento de caracteres de controle (Ctrl+C, Ctrl+Z, etc.).
    - `cbreak()`: Habilita o modo de quebra de linha, mas ainda processa caracteres de controle.
  - **Uso típico**:

    
```c
raw();  // Para jogos que precisam de controle total sobre a entrada
// ou
cbreak();  // Para aplicações que precisam de processamento de caracteres de controle
```


- `echo()` e `noecho()`:
  - **Descrição**: Controla se os caracteres digitados são ecoados para a tela.
  - **Uso típico**:

    
```c
noecho();  // Normalmente usado em jogos e interfaces personalizadas
// ou
echo();    // Útil para entrada de texto em formulários
```


- `keypad(window, TRUE)`:
  - **Descrição**: Habilita o processamento de teclas de função e teclas especiais (setas, F1-F12, etc.).
  - **Exemplo**:

    
```c
keypad(stdscr, TRUE);  // Habilita teclas de função na janela padrão
int ch = getch();
if (ch == KEY_UP) { /* tratar seta para cima */ }
```


## Gerenciamento de Janelas

### Criação e Destruição

- `newwin(nlines, ncols, begin_y, begin_x)`:
  - **Descrição**: Cria uma nova janela.
  - **Parâmetros**:
    - `nlines`: Número de linhas da janela.
    - `ncols`: Número de colunas da janela.
    - `begin_y`: Linha inicial.
    - `begin_x`: Coluna inicial.
  - **Exemplo**:

    
```c
WINDOW *win = newwin(10, 30, 5, 10);  // Janela 10x30 começando em (5,10)
if (win == NULL) {
    // Tratar erro
}
```


- `delwin(window)`:
  - **Descrição**: Libera a memória alocada para uma janela.
  - **Importante**: Nunca delete stdscr com esta função.

### Atualização de Tela

- `refresh()` e `wrefresh(win)`:
  - **Dica de desempenho**: Minimize chamadas a essas funções, pois são custosas.
  - **Padrão comum**:

    ```c
    // Atualizar múltiplas janelas
    wrefresh(win1);
    wrefresh(win2);
    // Ou para a janela padrão
    refresh();
    ```

## Entrada e Saída

### Funções de Saída

- `printw` e variantes:
  - **Equivalência**: Funcionam como `printf`, mas para janelas ncurses.
  - **Exemplo**:

    ```c
    int x = 10, y = 5;
    mvwprintw(win, y, x, "Pontuação: %d", score);
    ```

- `box(win, ver, hor)`:
  - **Descrição**: Desenha uma borda ao redor da janela.
  - **Exemplo**:

    ```c
    box(win, 0, 0);  // Usa caracteres padrão
    // Ou com caracteres personalizados
    box(win, '|', '-');
    ```

## Atributos e Cores

### Inicialização

- `start_color()`:
  - **Obrigatório**: Deve ser chamado antes de usar cores.
  - **Exemplo**:

    ```c
    if (has_colors()) {
        start_color();
        // Inicializar pares de cores
    }
    ```

### Pares de Cores

- `init_pair(pair, fg, bg)`:
  - **Limites**: Números de pares vão de 1 a COLOR_PAIRS-1.
  - **Exemplo**:

    ```c
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    wattron(win, COLOR_PAIR(1));
    wprintw(win, "Texto em branco sobre azul");
    wattroff(win, COLOR_PAIR(1));
    ```

## Exemplos Práticos

### Estrutura Básica de um Programa

```c
#include <ncurses.h>

int main() {
    // Inicialização
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    // Verificar suporte a cores
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
    }
    
    // Lógica principal
    printw("Pressione qualquer tecla para continuar...");
    refresh();
    getch();
    
    // Finalização
    endwin();
    return 0;
}
```

### Criando um Menu Simples

```c
void show_menu(WINDOW *win) {
    char *options[] = {"Opção 1", "Opção 2", "Sair"};
    int n_options = 3;
    int highlight = 0;
    int ch;
    
    while (1) {
        for (int i = 0; i < n_options; i++) {
            if (i == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 1, "%s", options[i]);
            wattroff(win, A_REVERSE);
        }
        
        ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10:  // Enter
                if (highlight == n_options - 1) return;
                // Processar seleção
                break;
        }
    }
}
```

## Dicas de Desempenho

### Atualizações de Tela

- Use `wnoutrefresh()` seguido de `doupdate()` para atualizações em lote
- Atualize apenas as partes da tela que mudaram

### Gerenciamento de Memória

- Sempre libere janelas com `delwin()`
- Use `clearok(stdscr, TRUE)` para forçar uma limpeza completa quando necessário

### Portabilidade

- Teste em diferentes terminais
- Use constantes como `COLS` e `LINES` para dimensões da tela

## Funções de Entrada Avançadas

### Leitura de Teclado

- `getch()` e `wgetch(win)`:
  - **Descrição**: Lê um único caractere do teclado.
  - **Retorno**: O caractere lido ou `ERR` em caso de erro.
  - **Exemplo**:
    ```c
    int ch;
    nodelay(stdscr, TRUE);  // Modo não-bloqueante
    while ((ch = getch()) == ERR) {
        // Espera por entrada
    }
    ```

- `getnstr(str, n)` e `wgetnstr(win, str, n)`:
  - **Descrição**: Lê uma string com limite de tamanho.
  - **Parâmetros**:
    - `str`: Buffer para armazenar a string.
    - `n`: Número máximo de caracteres a serem lidos.
  - **Exemplo**:
    ```c
    char nome[50];
    echo();  // Habilita eco
    getnstr(nome, 49);
    ```

### Manipulação de Janelas

- `subwin(orig, nlines, ncols, begin_y, begin_x)`:
  - **Descrição**: Cria uma subjanela dentro de outra janela.
  - **Exemplo**:
    ```c
    WINDOW *sub = subwin(stdscr, 5, 20, 10, 10);
    box(sub, 0, 0);
    wrefresh(sub);
    ```

- `mvwin(win, y, x)`:
  - **Descrição**: Move uma janela para uma nova posição na tela.
  - **Atenção**: Não pode mover janelas que se estendam além dos limites da tela.

## Gerenciamento de Cores

### Inicialização de Cores

- `start_color()`:
  - **Deve ser chamado** antes de qualquer outra função de cor.
  - **Verificação**:
    ```c
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal não suporta cores!\n");
        return 1;
    }
    start_color();
    ```

### Pares de Cores Personalizados

- `init_color(color, r, g, b)`:
  - **Descrição**: Define uma cor personalizada no terminal.
  - **Valores**: r, g, b variam de 0 a 1000.
  - **Exemplo**:
    ```c
    init_color(COLOR_MAGENTA, 300, 0, 300);
    init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
    ```

## Funções de Atributos

- `attron(attrs)` e `wattron(win, attrs)`:
  - **Atributos comuns**:
    - `A_NORMAL`: Atributo normal (desativa todos os outros)
    - `A_STANDOUT`: Máxima visibilidade
    - `A_UNDERLINE`: Sublinhado
    - `A_REVERSE`: Cores de primeiro e segundo plano trocadas
    - `A_BLINK`: Texto piscante
    - `A_DIM`: Texto com metade do brilho
    - `A_BOLD`: Texto em negrito

## Exemplo de Painel (Panel)

```c
#include <panel.h>

int main() {
    initscr();
    cbreak();
    noecho();
    
    // Criar janelas
    WINDOW *win1 = newwin(10, 30, 5, 10);
    WINDOW *win2 = newwin(10, 30, 8, 15);
    
    // Criar painéis
    PANEL *panel1 = new_panel(win1);
    PANEL *panel2 = new_panel(win2);
    
    // Desenhar conteúdo
    box(win1, 0, 0);
    box(win2, 0, 0);
    mvwprintw(win1, 1, 1, "Painel 1");
    mvwprintw(win2, 1, 1, "Painel 2");
    
    // Atualizar a pilha de exibição
    update_panels();
    doupdate();
    
    getch();
    endwin();
    return 0;
}
```

## Dicas de Desempenho

1. **Atualizações de Tela**
   - Use `wnoutrefresh()` seguido de `doupdate()` para atualizações em lote
   - Atualize apenas as partes da tela que mudaram

2. **Gerenciamento de Memória**
   - Sempre libere janelas com `delwin()`
   - Use `clearok(stdscr, TRUE)` para forçar uma limpeza completa quando necessário

3. **Portabilidade**
   - Teste em diferentes terminais
   - Use constantes como `COLS` e `LINES` para dimensões da tela

## Referências

- [NCurses Programming HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- `man ncurses` para documentação detalhada
- `curses.h` - Arquivo de cabeçalho completo com todas as definições

---
*Documentação atualizada em 26/05/2024*

  - `top`: Caractere para a borda superior.
  - `bottom`: Caractere para a borda inferior.
  - `tl_corner`: Caractere para o canto superior esquerdo.
  - `tr_corner`: Caractere para o canto superior direito.
  - `bl_corner`: Caractere para o canto inferior esquerdo.
  - `br_corner`: Caractere para o canto inferior direito.
- **Retorno**: Nenhum.
  - Descrição: Adiciona um caractere na posição atual do cursor na janela de tela cheia.
  - Parâmetros:
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `mvaddch(y, x, ch)`:
  - Descrição: Move o cursor para a posição especificada e adiciona um caractere.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `waddch(window, ch)`:
  - Descrição: Adiciona um caractere na posição atual do cursor na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `mvwaddch(window, y, x, ch)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona um caractere.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `printw(format, ...)`:
  - Descrição: Imprime uma string formatada na janela de tela cheia.
  - Parâmetros:
    - `format`: Uma string de formato, semelhante à função `printf`.
    - `...`: Argumentos opcionais para substituir os espaços reservados na string de formato.
  - Retorna: Nenhum.

    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para substituir os espaços reservados.
  - Retorna: Nenhum.

- `wprintw(window, format, ...)`:
  - Descrição: Imprime uma string formatada na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para substituir os espaços reservados.
  - Retorna: Nenhum.

- `mvwprintw(window, y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e imprime uma string formatada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para substituir os espaços reservados.
  - Retorna: Nenhum.

- `addstr(str)`:
  - Descrição: Adiciona uma string na posição atual do cursor na janela de tela cheia.
  - Parâmetros:
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `mvaddstr(y, x, str)`:
  - Descrição: Move o cursor para a posição especificada e adiciona uma string.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `waddstr(window, str)`:
  - Descrição: Adiciona uma string na posição atual do cursor na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `mvwaddstr(window, y, x, str)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona uma string.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

## Funções de Entrada

### `getch()` e `wgetch(win)`

- **Descrição**: Lê um único caractere do teclado.
- **Parâmetros**:
  - `win`: Janela para ler (apenas para `wgetch`).
- **Retorno**: O caractere lido ou `ERR` se nenhuma entrada estiver disponível.

**Exemplo**:

```c
int ch = getch();
if (ch == 'q') {
    // Sair do programa
} else if (ch == KEY_UP) {
    // Tecla de seta para cima pressionada
}
```

### `getstr(str)` e `wgetstr(win, str)`

- **Descrição**: Lê uma string do teclado até encontrar uma nova linha ou EOF.
- **Atenção**: Não verifica estouro de buffer. Use `getnstr` para segurança.

**Exemplo**:

```c
char nome[50];
echo();  // Habilita eco
getstr(nome);
```

### `getnstr(str, n)` e `wgetnstr(win, str, n)`

- **Descrição**: Versão segura de `getstr` com limite de caracteres.

**Exemplo**:

```c
char senha[20];
noecho();  // Desabilita eco para senhas
getnstr(senha, 19);
```

### `scanw(format, ...)` e `wscanw(win, format, ...)`

- **Descrição**: Lê dados formatados do teclado, similar a `scanf`.

**Exemplo**:

```c
int idade;
scanw("Digite sua idade: %d", &idade);
```

## Gerenciamento de Janelas Avançado

### Criação e Destruição

#### `newwin(nlines, ncols, begin_y, begin_x)`

- **Descrição**: Cria uma nova janela.
- **Parâmetros**:
  - `nlines`, `ncols`: Dimensões da janela.
  - `begin_y`, `begin_x`: Posição do canto superior esquerdo.
- **Retorno**: Ponteiro para a nova janela ou `NULL` em caso de erro.

#### `delwin(win)`

- **Descrição**: Libera a memória de uma janela.
- **Atenção**: Não use em `stdscr` ou janelas já deletadas.

### `subwin(orig, nlines, ncols, begin_y, begin_x)`

- **Descrição**: Cria uma subjanela que compartilha memória com a janela original.
- **Dica**: Útil para criar painéis dentro de janelas.

## Manipulação de Janelas

### `mvwin(win, y, x)`

- **Descrição**: Move uma janela para novas coordenadas.
- **Limitação**: Não pode mover além dos limites da tela.

### `wresize(win, lines, cols)`

- **Descrição**: Redimensiona uma janela.
- **Compatibilidade**: Disponível apenas em algumas implementações do NCurses.

### `overlay(src, dest)` e `overwrite(src, dest)`

- **Descrição**: Copia o conteúdo de uma janela para outra.
- **Diferença**: `overlay` preserva espaços em branco, `overwrite` não.

## Atributos e Cores

### Gerenciamento de Atributos

### `attron(attrs)` e `wattron(win, attrs)`

- **Atributos comuns**:
  - `A_NORMAL`: Atributo normal
  - `A_STANDOUT`: Máxima visibilidade
  - `A_UNDERLINE`: Texto sublinhado
  - `A_REVERSE`: Cores invertidas
  - `A_BLINK`: Texto piscante
  - `A_DIM`: Texto com brilho reduzido
  - `A_BOLD`: Texto em negrito

### `init_pair(pair, fg, bg)`

- **Descrição**: Define uma combinação de cores.

**Exemplo**:

```c
start_color();
init_pair(1, COLOR_RED, COLOR_BLACK);
attron(COLOR_PAIR(1));
printw("Texto em vermelho");
attroff(COLOR_PAIR(1));
```

## Funções de Tempo

### `napms(ms)`

- **Descrição**: Pausa a execução por milissegundos.

**Exemplo**:

```c
napms(500);  // Pausa por 500ms
```

### `timeout(delay)` e `wtimeout(win, delay)`

- **Descrição**: Define um tempo limite para entrada.
- **Valores especiais**:
  - `-1`: Bloqueia até a entrada estar disponível
  - `0`: Não bloqueia
  - `> 0`: Espera por no máximo `delay` milissegundos

## Exemplos Práticos

### Menu Interativo

```c
int show_menu(WINDOW *win, char *items[], int n_items) {
    int highlight = 0;
    int ch;
    
    keypad(win, TRUE);
    noecho();
    
    while (1) {
        for (int i = 0; i < n_items; i++) {
            if (i == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 1, "%s", items[i]);
            wattroff(win, A_REVERSE);
        }
        
        ch = wgetch(win);
        switch (ch) {
            case KEY_UP:    highlight = (highlight - 1 + n_items) % n_items; break;
            case KEY_DOWN:  highlight = (highlight + 1) % n_items; break;
            case '\n':     return highlight;
            case 'q':      return -1;  // Sair
        }
    }
}
```

### Barra de Progresso

```c
void draw_progress_bar(WINDOW *win, int y, int x, int width, float progress) {
    int filled = (int)(progress * width);
    mvwaddch(win, y, x, '[');
    for (int i = 0; i < width - 2; i++) {
        mvwaddch(win, y, x + 1 + i, i < filled ? '=' : ' ');
    }
    mvwaddch(win, y, x + width - 1, ']');
    wrefresh(win);
}
```

## Referências Adicionais

### Constantes Úteis

- **Cores**: `COLOR_BLACK`, `COLOR_RED`, `COLOR_GREEN`, `COLOR_YELLOW`, `COLOR_BLUE`, `COLOR_MAGENTA`, `COLOR_CYAN`, `COLOR_WHITE`
- **Teclas Especiais**: `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`, `KEY_HOME`, `KEY_END`, `KEY_NPAGE`, `KEY_PPAGE`
- **Atributos**: `A_NORMAL`, `A_STANDOUT`, `A_UNDERLINE`, `A_REVERSE`, `A_BLINK`, `A_DIM`, `A_BOLD`

### Macros Úteis

- `getmaxyx(win, y, x)`: Obtém as dimensões da janela
- `getbegyx(win, y, x)`: Obtém a posição da janela
- `getyx(win, y, x)`: Obtém a posição do cursor

---
*Documentação atualizada em 26/05/2024*

  - **Descrição**: Lê um único caractere do teclado.
  - **Parâmetros**:
    - `win`: Janela para ler (apenas para `wgetch`).
  - **Retorno**: O caractere lido ou `ERR` se nenhuma entrada estiver disponível.
  - **Exemplo**:
    ```c
    int ch = getch();
    if (ch == 'q') {
        // Sair do programa
    } else if (ch == KEY_UP) {
        // Tecla de seta para cima pressionada
    }
    ```

- `mvgetch(y, x)`:
  - Descrição: Move o cursor para a posição especificada e lê um caractere.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
  - Retorna: O caractere lido como um inteiro.

- `wgetch(window)`:
  - Descrição: Lê um caractere do teclado na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
  - Retorna: O caractere lido como um inteiro.

- `mvwgetch(window, y, x)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê um caractere.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
  - Retorna: O caractere lido como um inteiro.

- `scanw(format, ...)`:
  - Descrição: Lê dados formatados do teclado na janela de tela cheia.
  - Parâmetros:
    - `format`: Uma string de formato, semelhante à função `scanf`.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

- `mvscanw(y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada e lê dados formatados.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

- `wscanw(window, format, ...)`:
  - Descrição: Lê dados formatados do teclado na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

- `mvwscanw(window, y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê dados formatados.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

## Funções de Atributo:

- `attron(attrs)`:
  - Descrição: Habilita os atributos especificados.
  - Parâmetros:
    - `attrs`: Os atributos a serem habilitados (usando operadores bitwise OR).
  - Retorna: Nenhum.

- `wattron(window, attrs)`:
  - Descrição: Habilita os atributos especificados na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem habilitados.
  - Retorna: Nenhum.

- `attrset(attrs)`:
  - Descrição: Define os atributos especificados, substituindo os atributos anteriores.
  - Parâmetros:
    - `attrs`: Os atributos a serem definidos.
  - Retorna: Nenhum.

- `wattrset(window, attrs)`:
  - Descrição: Define os atributos especificados na janela especificada, substituindo os atributos anteriores.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem definidos.
  - Retorna: Nenhum.

- `attroff(attrs)`:
  - Descrição: Desabilita os atributos especificados.
  - Parâmetros:
    - `attrs`: Os atributos a serem desabilitados.
  - Retorna: Nenhum.

- `wattroff(window, attrs)`:
  - Descrição: Desabilita os atributos especificados na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem desabilitados.
  - Retorna: Nenhum.

- `standend()`:
  - Descrição: Desabilita todos os atributos, restaurando o modo normal.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `attr_get(attrs, pair, opts)`:
  - Descrição: Obtém os atributos e a combinação de cores atuais.
  - Parâmetros:
    - `attrs`: Um ponteiro para armazenar os atributos atuais.
    - `pair`: Um ponteiro para armazenar a combinação de cores atual.
    - `opts`: Opções adicionais (geralmente NULL).
  - Retorna: Nenhum.

- `wattr_get(window, attrs, pair, opts)`:
  - Descrição: Obtém os atributos e a combinação de cores atuais da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Um ponteiro para armazenar os atributos atuais.
    - `pair`: Um ponteiro para armazenar a combinação de cores atual.
    - `opts`: Opções adicionais (geralmente NULL).
  - Retorna: Nenhum.

- `chgat(n, attrs, color)`:
  - Descrição: Altera os atributos e a cor de caracteres já exibidos na tela.
  - Parâmetros:
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os atributos a serem aplicados.
    - `color`: A cor a ser aplicada.
  - Retorna: Nenhum.

- `mvchgat(y, x, n, attrs, color)`:
  - Descrição: Move o cursor para a posição especificada e altera os atributos e a cor de caracteres.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os atributos a serem aplicados.
    - `color`: A cor a ser aplicada.
  - Retorna: Nenhum.

- `wchgat(window, n, attrs, color)`:
  - Descrição: Altera os atributos e a cor de caracteres na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os atributos a serem aplicados.
    - `color`: A cor a ser aplicada.
  - Retorna: Nenhum.

## Funções de Atributos e Cores de Texto:

- `mvwchgat(window, y, x, n, attrs, color)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e altera os atributos e a cor de caracteres.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os atributos a serem aplicados.
    - `color`: A cor a ser aplicada.
  - Retorna: Nenhum.

## Funções de Janela:

- `box(window, vertical, horizontal)`:
  - Descrição: Desenha uma borda em torno da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `vertical`: O caractere a ser usado para a borda vertical.
    - `horizontal`: O caractere a ser usado para a borda horizontal.
  - Retorna: Nenhum.

- `newwin(lines, cols, begin_y, begin_x)`:
  - Descrição: Cria uma nova janela com as dimensões especificadas.
  - Parâmetros:
    - `lines`: O número de linhas da janela.
    - `cols`: O número de colunas da janela.
    - `begin_y`: A coordenada y inicial da janela.
    - `begin_x`: A coordenada x inicial da janela.
  - Retorna: Um ponteiro para a nova janela.

- `create_newwin(lines, cols, begin_y, begin_x)`:
  - Descrição: Cria uma nova janela com as dimensões especificadas e desenha uma borda em torno dela.
  - Parâmetros:
    - `lines`: O número de linhas da janela.
    - `cols`: O número de colunas da janela.
    - `begin_y`: A coordenada y inicial da janela.
    - `begin_x`: A coordenada x inicial da janela.
  - Retorna: Um ponteiro para a nova janela.

- `delwin(window)`:
  - Descrição: Desaloca a memória associada à janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser desalocada.
  - Retorna: Nenhum.

- `destroy_win(window)`:
  - Descrição: Apaga a janela especificada da tela e desaloca a memória associada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser apagada e desalocada.
  - Retorna: Nenhum.

- `wborder(window, left, right, top, bottom, tl_corner, tr_corner, bl_corner, br_corner)`:
  - Descrição: Desenha uma borda em torno da janela especificada com os caracteres especificados.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `left`: O caractere para a borda esquerda.
    - `right`: O caractere para a borda direita.
    - `top`: O caractere para a borda superior.
    - `bottom`: O caractere para a borda inferior.
    - `tl_corner`: O caractere para o canto superior esquerdo.
    - `tr_corner`: O caractere para o canto superior direito.
    - `bl_corner`: O caractere para o canto inferior esquerdo.
    - `br_corner`: O caractere para o canto inferior direito.
  - Retorna: Nenhum.

## Funções de Cor:

- `start_color()`:
  - Descrição: Inicializa o uso de cores.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `has_colors()`:
  - Descrição: Verifica se o terminal suporta cores.
  - Parâmetros: Nenhum.
  - Retorna: Um valor booleano (true ou false).

- `init_pair(pair, f, b)`:
  - Descrição: Define uma combinação de cores com o número de par especificado e as cores de primeiro plano e fundo.
  - Parâmetros:
    - `pair`: O número da combinação de cores.
    - `f`: A cor de primeiro plano.
    - `b`: A cor de fundo.
  - Retorna: Nenhum.

- `init_color(color, r, g, b)`:
  - Descrição: Define as cores RGB para uma cor específica.
  - Parâmetros:
    - `color`: O número da cor a ser definida.
    - `r`: O valor RGB vermelho (0-1000).
    - `g`: O valor RGB verde (0-1000).
    - `b`: O valor RGB azul (0-1000).
  - Retorna: Nenhum.

- `can_change_color()`:
  - Descrição: Verifica se o terminal pode alterar as cores.
  - Parâmetros: Nenhum.
  - Retorna: Um valor booleano (true ou false).

## Funções de Entrada e Saída:

- `getstr(str)`:
  - Descrição: Lê uma string do teclado na janela de tela cheia.
  - Parâmetros:
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
  - Retorna: Nenhum.

- `mvgetstr(y, x, str)`:
  - Descrição: Move o cursor para a posição especificada e lê uma string.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
  - Retorna: Nenhum.

- `wgetstr(window, str)`:
  - Descrição: Lê uma string do teclado na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
  - Retorna: Nenhum.

- `mvwgetstr(window, y, x, str)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê uma string.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
  - Retorna: Nenhum.

- `wgetnstr(window, str, n)`:
  - Descrição: Lê uma string do teclado na janela especificada, com um limite de comprimento especificado.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
    - `n`: O limite de comprimento para a string lida.
  - Retorna: Nenhum.

- `mvwgetnstr(window, y, x, str, n)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê uma string com um limite de comprimento especificado.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
    - `n`: O limite de comprimento para a string lida.
  - Retorna: Nenhum.

## Funções de Entrada e Saída de Janela:

- `mvwprintw(window, y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e imprime uma string formatada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para substituir os espaços reservados.
  - Retorna: Nenhum.

- `mvwaddstr(window, y, x, str)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona uma string.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `mvwaddch(window, y, x, ch)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona um caractere.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `mvwscanw(window, y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê dados formatados.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

## Funções de Janela Diversas:

- `wclear(window)`:
  - Descrição: Limpa a janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser limpada.
  - Retorna: Nenhum.

- `wmove(window, y, x)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
  - Retorna: Nenhum.

- `wrefresh(window)`:
  - Descrição: Atualiza a janela especificada, refletindo as alterações feitas.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser atualizada.
  - Retorna: Nenhum.

- `wborder(window, left, right, top, bottom, tl_corner, tr_corner, bl_corner, br_corner)`:
  - Descrição: Desenha uma borda em torno da janela especificada com os caracteres especificados.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `left`: O caractere para a borda esquerda.
    - `right`: O caractere para a borda direita.
    - `top`: O caractere para a borda superior.
    - `bottom`: O caractere para a borda inferior.
    - `tl_corner`: O caractere para o canto superior esquerdo.
    - `tr_corner`: O caractere para o canto superior direito.
    - `bl_corner`: O caractere para o canto inferior esquerdo.
    - `br_corner`: O caractere para o canto inferior direito.
  - Retorna: Nenhum.

## Funções de Entrada e Saída de Janela Diversas:

- `mvwaddstr(window, y, x, str)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona uma string.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `mvwaddch(window, y, x, ch)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e adiciona um caractere.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `mvwscanw(window, y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e lê dados formatados.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

## Funções de Janela de Atributo:

- `wattron(window, attrs)`:
  - Descrição: Habilita os atributos especificados na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem habilitados.
  - Retorna: Nenhum.

- `wattroff(window, attrs)`:
  - Descrição: Desabilita os atributos especificados na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem desabilitados.
  - Retorna: Nenhum.

- `wattrset(window, attrs)`:
  - Descrição: Define os atributos especificados na janela especificada, substituindo os atributos anteriores.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem definidos.
  - Retorna: Nenhum.

- `wattr_get(window, attrs, pair, opts)`:
  - Descrição: Obtém os atributos e a combinação de cores atuais da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Um ponteiro para armazenar os atributos atuais.
    - `pair`: Um ponteiro para armazenar a combinação de cores atual.
    - `opts`: Opções adicionais (geralmente NULL).
  - Retorna: Nenhum.


- `wchgat(window, n, attrs, color)`:
  - Descrição: Altera os atributos e a cor de caracteres já exibidos na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os novos atributos a serem aplicados.
    - `color`: A nova cor a ser aplicada.
  - Retorna: Nenhum.

- `mvwchgat(window, y, x, n, attrs, color)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e altera os atributos e a cor de caracteres.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os novos atributos a serem aplicados.
    - `color`: A nova cor a ser aplicada.
  - Retorna: Nenhum.

## Funções de Janela de Cor:

- `wcolor_set(window, color, attrs)`:
  - Descrição: Define a cor e os atributos da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `color`: A cor a ser definida.
    - `attrs`: Os atributos a serem definidos.
  - Retorna: Nenhum.

- `wattr_set(window, attrs, color_pair, opts)`:
  - Descrição: Define os atributos e a combinação de cores atuais da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Os atributos a serem definidos.
    - `color_pair`: A combinação de cores a ser definida.
    - `opts`: Opções adicionais (geralmente NULL).
  - Retorna: Nenhum.

- `wattr_get(window, attrs, pair, opts)`:
  - Descrição: Obtém os atributos e a combinação de cores atuais da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `attrs`: Um ponteiro para armazenar os atributos atuais.
    - `pair`: Um ponteiro para armazenar a combinação de cores atual.
    - `opts`: Opções adicionais (geralmente NULL).
  - Retorna: Nenhum.

- `wchgat(window, n, attrs, color)`:
  - Descrição: Altera os atributos e a cor de caracteres já exibidos na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os novos atributos a serem aplicados.
    - `color`: A nova cor a ser aplicada.
  - Retorna: Nenhum.

- `mvwchgat(window, y, x, n, attrs, color)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada e altera os atributos e a cor de caracteres.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
    - `n`: O número de caracteres a serem alterados.
    - `attrs`: Os novos atributos a serem aplicados.
    - `color`: A nova cor a ser aplicada.
    - `right`: O caractere para a borda direita.
    - `top`: O caractere para a borda superior.
    - `bottom`: O caractere para a borda inferior.
    - `tl_corner`: O caractere para o canto superior esquerdo.
    - `tr_corner`: O caractere para o canto superior direito.
    - `bl_corner`: O caractere para o canto inferior esquerdo.
    - `br_corner`: O caractere para o canto inferior direito.
  - Retorna: Nenhum.