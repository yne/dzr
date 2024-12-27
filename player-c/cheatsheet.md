## Funções de Inicialização:

- `initscr()`:
  - Descrição: Inicializa a biblioteca ncurses e cria uma janela de tela cheia.
  - Parâmetros: Nenhum.
  - Retorna: Um ponteiro para a janela de tela cheia.

- `endwin()`:
  - Descrição: Restaura o terminal para seu estado original e encerra o uso da biblioteca ncurses.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `refresh()`:
  - Descrição: Atualiza a tela, refletindo as alterações feitas nas janelas e painéis.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `wrefresh(window)`:
  - Descrição: Atualiza a janela especificada, refletindo as alterações feitas.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser atualizada.
  - Retorna: Nenhum.

- `raw()`:
  - Descrição: Define o modo bruto, desabilitando o processamento de caracteres de controle.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `cbreak()`:
  - Descrição: Define o modo de quebra de linha, permitindo que os caracteres de controle sejam interpretados como qualquer outro caractere.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `echo()`:
  - Descrição: Habilita o eco de caracteres digitados pelo usuário.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `noecho()`:
  - Descrição: Desabilita o eco de caracteres digitados pelo usuário.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `keypad(window, TRUE)`:
  - Descrição: Habilita o modo de teclado, permitindo a leitura de teclas de função na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `TRUE`: Habilita o modo de teclado.
  - Retorna: Nenhum.

- `halfdelay(tenths)`:
  - Descrição: Define o modo de atraso parcial, aguardando a entrada do usuário por uma fração de segundo especificada.
  - Parâmetros:
    - `tenths`: O número de décimos de segundo para aguardar a entrada.
  - Retorna: Nenhum.

## Funções Diversas:

- `clear()`:
  - Descrição: Limpa a janela de tela cheia.
  - Parâmetros: Nenhum.
  - Retorna: Nenhum.

- `wclear(window)`:
  - Descrição: Limpa a janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela a ser limpa.
  - Retorna: Nenhum.

- `move(y, x)`:
  - Descrição: Move o cursor para a posição especificada na janela de tela cheia.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
  - Retorna: Nenhum.

- `wmove(window, y, x)`:
  - Descrição: Move o cursor para a posição especificada na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
  - Retorna: Nenhum.

- `getmaxyx(window, y, x)`:
  - Descrição: Obtém as dimensões da janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: Um ponteiro para armazenar a altura da janela (número de linhas).
    - `x`: Um ponteiro para armazenar a largura da janela (número de colunas).
  - Retorna: Nenhum.

- `getyx(window, y, x)`:
  - Descrição: Obtém a posição atual do cursor na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `y`: Um ponteiro para armazenar a coordenada y (linha) do cursor.
    - `x`: Um ponteiro para armazenar a coordenada x (coluna) do cursor.
  - Retorna: Nenhum.

## Funções de Saída:

- `addch(ch)`:
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

- `mvprintw(y, x, format, ...)`:
  - Descrição: Move o cursor para a posição especificada e imprime uma string formatada.
  - Parâmetros:
    - `y`: A coordenada y (linha) para mover o cursor.
    - `x`: A coordenada x (coluna) para mover o cursor.
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

## Funções de Entrada:

- `getch()`:
  - Descrição: Lê um caractere do teclado, aguardando a entrada do usuário.
  - Parâmetros: Nenhum.
  - Retorna: O caractere lido como um inteiro.

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
  - Retorna: Nenhum.

## Funções de Janela de Entrada e Saída:

- `wprintw(window, format, ...)`:
  - Descrição: Imprime uma string formatada na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para substituir os espaços reservados.
  - Retorna: Nenhum.

- `waddstr(window, str)`:
  - Descrição: Adiciona uma string na posição atual do cursor na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: A string a ser adicionada.
  - Retorna: Nenhum.

- `waddch(window, ch)`:
  - Descrição: Adiciona um caractere na posição atual do cursor na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `ch`: O caractere a ser adicionado.
  - Retorna: Nenhum.

- `wscanw(window, format, ...)`:
  - Descrição: Lê dados formatados do teclado na janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `format`: Uma string de formato.
    - `...`: Argumentos opcionais para armazenar os dados lidos.
  - Retorna: Nenhum.

- `wgetstr(window, str)`:
  - Descrição: Lê uma string do teclado na janela especificada.**
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
  - Retorna: Nenhum.

- `wgetnstr(window, str, n)`:
  - Descrição: Lê uma string do teclado na janela especificada, com um limite de comprimento especificado.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
    - `str`: Um ponteiro para um buffer de caracteres onde a string lida será armazenada.
    - `n`: O limite de comprimento para a string lida.
  - Retorna: Nenhum.

## Funções de Janela Diversas:

- `wclear(window)`:
  - Descrição: Limpa a janela especificada.
  - Parâmetros:
    - `window`: Um ponteiro para a janela.
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
    - `window`: Um ponteiro para a janela.
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