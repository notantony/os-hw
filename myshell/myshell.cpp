#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cctype>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

using std::string;
using std::cin;
using std::cerr;
using std::cout;
using std::vector;
using std::swap;
using std::endl;

vector<char *> split(string &s) {
	vector<char *> v;
	for (size_t i = 0; i < s.size(); i++) {
		while (i < s.size() && isspace(s[i])) i++; 
		if (i < s.size()) {
			size_t l = i;
			while (i < s.size() && !isspace(s[i])) {
				i++;
			}
			char *one = new char[i - l + 1];
			strcpy(one, s.substr(l, i - l).c_str()); 
			v.push_back(one);
		}
	}
	v.push_back(NULL);
	return v;
}

void clear(vector<char *> v) {
	for (size_t i = 0; i < v.size(); i++) {
		delete [](v[i]);
	}
}

int main(int args, char **argv) {
	string s;
	while (true) {
		if (!std::getline(cin, s)) {
			cout << "Reading was interrupted" << endl;
			return 0;
		}

		vector<char *> parsed = split(s);
		if (parsed.size() == 1) {
			continue;
		}
		if (parsed.size() == 2 && !strcmp(parsed[0], "exit")) {
			cout << "Closing shell" << endl;
			clear(parsed);
			return 0;
		}

		char *cmd = parsed[0];
		pid_t pid = fork();
		if (pid < 0) {
			cerr << "Error occurred while starting execution. Error code " << errno << endl;
		} else if (pid == 0) {
			if (execve(cmd, &parsed[0], NULL) == -1) {
				exit(errno);
			}
			exit(0);
		} else {
			int code;
			waitpid(pid, &code, 0);
			cout << "Execution ended with return code " << WEXITSTATUS(code) << endl;
		}
		clear(parsed);
	}
	return 0;
}