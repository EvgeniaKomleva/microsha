#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>
#include <wait.h>
using namespace std;

void PrintVector(vector<string>& v) { //пригодится в хозяйстве
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << endl;
    }
}

int GetStatus () {
    return 0;
}

string WorkingDir() { //текущая директория
    char buff[FILENAME_MAX];
    getcwd( buff, FILENAME_MAX );
    string working_dir(buff);
    return working_dir;
}

vector<string> GetCods () { //разбивает команду на слова по пробелам и записывает в вектор
    vector<string> arr;//(похоже не нужна, но пусть останется - в хозяйстве пригодится!)
    string str;
    (getline(cin, str));
    istringstream iss(str);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
              istream_iterator<string>(),
              back_inserter<vector<string> >(tokens));
    return tokens;
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
        /*do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status));//ждем все процессы*/
        waitpid(pid, &status, WUNTRACED);
    }

    return 1;

}
//обьединение встроенных и внешний ф-ций, запускает либо встроенный, либо наш процесс
int execute(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }

    /*for (int i = 0; i < lsh_num_builtins(); i++) { //если вызванавстроенная фция
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }*/

    return new_process(args);
}

//чтение неограниченного обьема текста
char *  read_line(void)
{
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
char **split_line(char *line)
{
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