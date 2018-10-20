#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>
#include <wait.h>
using namespace std;

int GetStatus () { //узнать привелегерованный пользователь или нет
    return 0;
}

int cd(char **args) {
    if (chdir(args[1]) != 0) {
        perror("lsh");
    }
    return 1;
}

int time (char **args){
    return 1;
}

int pwd(char **args){
    return 1;
}

string WorkingDir() { //текущая директория
    char buff[FILENAME_MAX];
    getcwd( buff, FILENAME_MAX );
    string working_dir(buff);
    return working_dir;
}

/*порождение процессов */
int new_process(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
    } else {
        waitpid(pid, &status, WUNTRACED);
    }

    return 1;

}
//обьединение встроенных и внешний ф-ций, запускает либо встроенный, либо наш процесс
int execute(char **args) {
    if (strcmp(args[0], "cd") == 0) {
            return cd(args);
    } else if (strcmp(args[0], "time") == 0){
        return time(args);
    }else if (strcmp(args[0], "pwd") == 0) {
        return pwd(args);
    }else {
        return new_process(args);
    }
}

//чтение неограниченного обьема текста
char *  read_line(void) {
    int bufsize = 1000;
    int position = 0;//число прочитанных символов
    char *buffer = (char*)malloc(sizeof(char) * bufsize);
    int c;
    while (1) {
        c = getchar();
        if (c == EOF) {
            break;
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufsize) {
            bufsize += 1000;
            buffer = (char*)realloc(buffer, bufsize);
        }
    }
}

//разделение команд
char **split_line(char *line) {
    int bufsize = 50;
    int position = 0;
    char **tokens = (char**)malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += 50;
            tokens = (char**)realloc(tokens, bufsize * sizeof(char*));
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

void Start() {
    char *line;
    char **args;
    int status;
    do {
        cout << WorkingDir() << "> ";
        line = read_line();
        args = split_line(line);
        status = execute(args);
        free(line);
        free(args);
    } while (status);
}


int main() {
    Start();
    return 0;
}

//как реализовать метасимволы