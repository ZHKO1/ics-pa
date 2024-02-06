#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <strings.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  size_t length = strlen(cmd);
  assert(length < 1000);
  
  char cmd_temp[1000] = "";
  strcpy(cmd_temp, cmd);
  char *cmd_c = cmd_temp;
  while (*cmd_c) {
    if(*cmd_c == '\n'){
      *cmd_c = ' ';
    }
    cmd_c++;
  }

  char *argv[100] = {NULL};
  const char s[2] = " ";

  char *token;
  size_t token_index = 0;
  token = strtok(cmd_temp, s);
  while( token != NULL ) {
    argv[token_index] = token;
    assert(token_index < 100);
    token = strtok(NULL, s);
    token_index++;
  }

  setenv("PATH", "/bin", 0);
  execvp(argv[0], argv);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
