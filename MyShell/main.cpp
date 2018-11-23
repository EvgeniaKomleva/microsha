#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>
#include <wait.h>
#include <time.h>
#include <list>
#include <fcntl.h>
#include <fnmatch.h>

using namespace std;

int cd(vector<string> & args) {
    if (chdir(args[1].c_str()) != 0) {
        perror("micro:cd");
    }
    return 1;
}

void time(int in, int out,  vector<string>& conveer )
{
    clock_t start = clock();
    pid_t pid = fork();
    int * fd = new int[(conveer.size()-1 ) * 2];
    if(pid == 0) {
        char ** args = new char *[conveer.size() + 1];
        for(int i = 0; i < conveer.size(); i++){
            args[i] = &(conveer[i][0]);
        }
        args[conveer.size()] = NULL;
        if(in != 0) {
            dup2(in, 0);
        }
        if(out != 1){
            dup2(out, 1);
        }
        for(int i = 0; i < (conveer.size() ) * 2; i++) {
            if(fd[i] != in && fd[i] != out) close(fd[i]);
        }
        execvp(conveer[0].c_str(), args);
        for(int i = 0; i < conveer.size(); i++){
            delete [] args[i];
        }
        delete [] args;
        exit(0);
    }
    clock_t end = clock();
    dprintf(2, "%.3lf secs\n",
            (double)(end - start) / CLOCKS_PER_SEC);
}

string WorkingDir() { //текущая директория
    char buff[FILENAME_MAX];
    getcwd( buff, FILENAME_MAX );
    string working_dir(buff);
    return working_dir;
}

int pwd(vector<string> & args){
    cout << "You are here! : " << WorkingDir()<< endl;
    return 1;
}

int star(vector<string> &conveer, string comand) {
    int fd[2];
    pipe(fd);
    pid_t id = fork();
    if (id == 0) {
        close(fd[0]);
        close(1);
        dup2(fd[1], 1);
        execlp("/bin/ls", "ls", NULL);
    } else {
        auto *buff = (char *) malloc(sizeof(char) * 1000);
        close(fd[1]);
        read(fd[0], buff, 1000);
        string buffs(buff);
        vector<string> res, dir;
        res.push_back(conveer[0]);
        while (buffs.find('\n') != -1) {
            dir.push_back(buffs.substr(0, buffs.find('\n')));
            buffs = buffs.substr(buffs.find('\n') + 1, buffs.size());
        }
        for (int i = 0; i < dir.size(); ++i) {
            if (!fnmatch(comand.c_str(), dir[i].c_str(), 0)) {
                res.push_back(dir[i]);
            }
        }
        free(buff);
        vector<char *> arg;
        for (int i = 0; i < res.size(); ++i) {
            arg.push_back((char *) res[i].c_str());
        }
        arg.push_back(NULL);
        pid_t pid = fork();
        if (pid == 0) {
            execvp(conveer[0].c_str(), &arg[0]);
        } else {
            int info;
            waitpid(pid, &info, 0);
        }
    }
}

int exec_cod(vector<string> & conveer, int in, int out, int convSize, int * fd) {
    pid_t pid = fork();
    if(pid == 0) {
        char ** args = new char *[conveer.size() + 1];
        for(int i = 0; i < conveer.size(); i++){
            args[i] = &(conveer[i][0]);
        }
        args[conveer.size()] = NULL;
        if(in != 0) {
            dup2(in, 0);
        }
        if(out != 1){
            dup2(out, 1);
        }
        for(int i = 0; i < (convSize ) * 2; i++) {
            if(fd[i] != in && fd[i] != out) close(fd[i]);
        }
        execvp(conveer[0].c_str(), args);
        for(int i = 0; i < conveer.size(); i++){
            delete [] args[i];
        }
        delete [] args;
        exit(0);
    }
    return 0;
}

/*порождение процессов */
int new_process(list<vector<string>> & conveer) {
    vector<string> in_out = *(conveer.begin());
    int in = 0, out = 1;
    int * fd = new int[(conveer.size()-1 ) * 2];
    for(int i = 0; i < conveer.size(); i++) {
        pipe(fd + i * 2);
    }
    if(conveer.size() == 1) {
        auto it = conveer.begin();
        exec_cod(*it, in, out, conveer.size(), fd);
    } else {
        auto it = conveer.end();
        it--;
        for(int i = 0; i < conveer.size() ; i++, it--) {
            if(i == 0) {
                exec_cod(*it, in, fd[1], conveer.size(), fd);
                continue;
            }
            if(i == conveer.size() -1) {
                exec_cod(*it, fd[2 * i - 2], out, conveer.size(), fd);
                continue;
            }
            exec_cod(*it, fd[2 * i - 2], fd[2 * i + 1], conveer.size(), fd);
        }
    }
    for(int i = 0; i < (conveer.size() ) * 2; i++){
        close(fd[i]);
    }
    delete [] fd;
    int info;
    while(wait(&info) > 0);
    return 1;
}


void in_out(string &cod, vector<char *> &args, string &filename, int sign) {
    if (sign) {
        pid_t pid = fork();
        if (pid == 0) {
            close(1);//out descriptor
            const char *name = filename.c_str();
            open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
            string path;
            execvp(cod.c_str(), &args[0]);
        } else {
            int info;
            waitpid(pid, &info, 0);
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            close(0);
            const char *name = filename.c_str();
            open(name, O_RDWR | O_CREAT, 0666);
            execvp(cod.c_str(), &args[0]);
        } else {
            int info;
            waitpid(pid, &info, 0);
        }
    }
}


//обьединение встроенных и внешний ф-ций, запускает либо встроенный, либо наш процесс
int execute(list<vector<string>> & conv) {
    for (auto it = conv.begin(); it != conv.end(); it++) {
        vector<string> conveer = *it;
        //cout << "first "<< conveer[0] << "JJJJJJJJJJJJJJJJJJJJ";
        string input;
       if (conveer.size()>1){
           input = conveer[conveer.size()-2];
       }
       for (int i = 0; i < conveer.size();i++) {
           string comand = conveer[i];
           if (comand.find('*') != -1 || comand.find('?') != -1) {
               return star(conveer, comand);
           }
       }
        if (input.find('>') != -1 || input.find('<') != -1) {
            string str = conveer[0];
            vector<char *> arg;
            for (int i = 0; i < conveer.size()-2; ++i) {
                arg.push_back((char *) conveer[i].c_str());
            }
            arg.push_back(NULL);
            string k = conveer[conveer.size()-1];
            if (input.find('>') != -1) {
                string s = conveer[conveer.size()-4] + ' ' +conveer[conveer.size()-3];
                in_out(str,arg,k ,1);
            } else {
                string s2 = input.substr(input.find('<') + 1, input.size());
                in_out(str, arg, s2, 0);
            }
        }else if(conveer[0]== "cd" ){
            return cd(conveer);
        }else if (conveer[1]=="time"){
            time(0,1,conveer);
        }else if (conveer[0] == "pwd") {
            return pwd(conveer);
        }else {
            return new_process(conv);
        }
    }
}


//разделение команд
int split_line(string line, list<vector<string>> & comand) {
    comand = list<vector<string>>();

    comand.push_front(vector<string>());
    char *token;
    char *cstr = new char[line.length() + 1];
    strcpy(cstr, line.c_str());
    token = strtok(cstr, " \t\r\n\a");
    while (token != NULL)
    {
        string s = token;
        if (s == "|"){
            //cout <<"AAAAA"<< endl;
            comand.push_front(vector<string>());
        }
        else {
            comand.front().push_back(s);
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    return 0;
}

void Start() {
    string line;
    list<vector<string>> comand;
    while(1) {
        cout << WorkingDir() << "> ";
        if(getline(cin, line)) {
            split_line(line, comand);
        }
        if (line.size() == 0){ break;}
        execute(comand);
    }
}

int main() {
    Start();
    return 0;
}