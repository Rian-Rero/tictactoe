#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <cstdlib>
#include <ctime>

// Classe TicTacToe
class TicTacToe
{
private:
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
  char current_player;                      // Jogador atual ('X' ou 'O')
  bool game_over;                           // Estado do jogo
  char winner;                              // Vencedor do jogo
  std::mutex game_mutex;                    // Protege todo o estado do jogo
  std::condition_variable turn_cv;          // Coordena alternancia de turnos

  void display_board_unlocked()
  {
    std::system("clear");
    for (int i = 0; i < 3; i++)
    {
      std::cout << board[i][0] << "|" << board[i][1] << "|" << board[i][2] << std::endl;
      if (i != 2)
      {
        std::cout << "-----" << std::endl;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  bool check_win_unlocked(char player)
  {
    for (int i = 0; i < 3; i++)
    {
      if (player == board[i][0] && player == board[i][1] && player == board[i][2])
      {
        winner = player;
        return 1;
      }
    }
    for (int i = 0; i < 3; i++)
    {
      if (player == board[0][i] && player == board[1][i] && player == board[2][i])
      {
        winner = player;
        return 1;
      }
    }
    if (player == board[0][0] && player == board[1][1] && player == board[2][2])
    {
      winner = player;
      return 1;
    }
    if (player == board[0][2] && player == board[1][1] && player == board[2][0])
    {
      winner = player;
      return 1;
    }
    return 0;
  }

  bool check_draw_unlocked()
  {
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (board[i][j] == ' ')
        {
          return 0;
        }
      }
    }
    return 1;
  }

  bool is_game_over_unlocked(char player)
  {
    if (check_win_unlocked(player))
    {
      return 1;
    }
    else if (check_draw_unlocked())
    {
      winner = 'D';
      return 1;
    }
    else
    {
      winner = '-';
      return 0;
    }
  }

public:
  TicTacToe()
  {
    // Inicializar o tabuleiro e as variáveis do jogo
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        board[i][j] = ' ';
      }
    }
    winner = '-';
    game_over = 0;
    // sorteia jogador inicial entre 'X' e 'O'
    static std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
    static std::uniform_int_distribution<int> distr(0, 1);
    current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
  }

  void display_board()
  {
    // Exibir o tabuleiro no console com acesso sincronizado
    std::lock_guard<std::mutex> lock(game_mutex);
    display_board_unlocked();
  }

  bool make_move(char player, int row, int col)
  {
    // Espera pela vez do jogador e faz a jogada com exclusao mutua
    std::unique_lock<std::mutex> lock(game_mutex);
    turn_cv.wait(lock, [&]()
                 { return game_over || player == current_player; });

    if (game_over)
    {
      return 1;
    }

    if (board[row][col] != 'X' && board[row][col] != 'O')
    {
      board[row][col] = player;
      display_board_unlocked();
      game_over = is_game_over_unlocked(player);
      if (!game_over)
      {
        current_player = (player == 'O') ? 'X' : 'O';
      }
      turn_cv.notify_all();
      return 1;
    }

    return 0;
  }

  bool check_win(char player)
  {
    // Verificar se o jogador atual venceu o jogo
    std::lock_guard<std::mutex> lock(game_mutex);
    return check_win_unlocked(player);
  }

  bool check_draw()
  {
    // Verificar se houve um empate
    std::lock_guard<std::mutex> lock(game_mutex);
    return check_draw_unlocked();
  }

  bool is_game_over()
  {
    // Retornar se o jogo terminou
    std::lock_guard<std::mutex> lock(game_mutex);
    return game_over;
  }

  char get_winner()
  {
    // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
    std::lock_guard<std::mutex> lock(game_mutex);
    return winner;
  }
};

// Classe Player
class Player
{
private:
  TicTacToe &game;      // Referência para a instância do jogo
  char symbol;          // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador

public:
  Player(TicTacToe &g, char s, std::string strat)
      : game(g), symbol(s), strategy(strat) {}

  void play()
  {
    // Executar jogadas de acordo com a estratégia escolhida
    while (game.get_winner() == '-')
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if (strategy == "sequential")
      {
        play_sequential();
      }
      else
      {
        play_random();
      }
    }
  }

private:
  void play_sequential()
  {
    // Implementar a estratégia sequencial de jogadas
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (game.make_move(symbol, i, j))
        {
          return;
        }
      }
    }
  }

  void play_random()
  {
    // Implementar a estratégia aleatória de jogadas
    int l;
    int c;
    bool fim = 0;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distr(0, 2);
    while (!fim)
    {
      l = distr(gen);
      c = distr(gen);
      fim = game.make_move(symbol, l, c);
    }
  }
};

// Função principal
int main()
{
  // Inicializar o jogo e os jogadores
  TicTacToe tabuleiro;
  tabuleiro.display_board();
  Player X(tabuleiro, 'X', "sequential");
  Player O(tabuleiro, 'O', "random");

  // Criar as threads para os jogadores
  std::thread Jogador1(&Player::play, &X);
  std::thread Jogador2(&Player::play, &O);

  // Aguardar o término das threads
  Jogador1.join();
  Jogador2.join();

  // Exibir o resultado final do jogo
  char vencedor = tabuleiro.get_winner();
  if (vencedor == 'D')
  {
    std::cout << " Empate!\n";
  }
  else
  {
    std::cout << " Vencedor: " << vencedor << "\n";
  }

  return 0;
}
