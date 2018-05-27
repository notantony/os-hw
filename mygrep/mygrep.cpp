#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h> 
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <map>
#include <queue>

using std::string;
using std::map;
using std::queue;
using std::pair;

const long long p = 100;
const long long mod = 1e9 + 7;
long long hash, deg;
int len;

const size_t BUFFER_SIZE = 1024, MAX_PATH_LENGTH = 4096;
char buf[BUFFER_SIZE * 2], path[MAX_PATH_LENGTH * 2];

void pre(const string &s) {
	len = s.size();
	hash = 0;
	deg = 1;
	for (size_t i = 0; i < s.size(); i++) {
		hash = (hash * p + s[i]) % mod;
		deg = (deg * p) % mod;
	}
}
int ask(int where) {
	return where * BUFFER_SIZE;
} 

int read_file(const string &file) {
	int fd = open(file.c_str(), 0, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open file: %s\n", file.c_str());
		return 0;
	}

	long long cur_hash = 0;

	ssize_t read_count = 0, total = 0;
	int where = 0;

	while ((read_count = read(fd, buf + BUFFER_SIZE * where, BUFFER_SIZE)) > 0) {
		for (ssize_t i = 0; i < read_count; i++, total++) {
			cur_hash = (cur_hash * p + buf[i + ask(where)]) % mod;
			if (total >= len) {
				cur_hash -= buf[i - len < 0 ? i - len + BUFFER_SIZE + ask(where ^ 1) : i - len + ask(where)] * deg;
				cur_hash += mod * mod;
				cur_hash %= mod;
			}
			if (total >= len - 1 && hash == cur_hash) {
				return 1;
			}
		}
		where ^= 1;
	}
	return 0;
}

queue<string *> q;
map<pair<dev_t, ino_t>, string> mp;


void push_dir(struct stat &st) {
	
}

void walk(string &path) {
	DIR *at;
    dirent *entry;
	
    if ((at = opendir(path.c_str())) == NULL) {
		fprintf(stderr, "Cannot open directory: %s\n", path.c_str());    	
        return;
    }

    while ((errno = 0, entry = readdir(at)) != NULL) {
		string new_path = path + "/" + string(entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			struct stat st;
			if (stat(new_path.c_str(), &st) == -1) {
				fprintf(stderr, "Cannot get file stat for file: %s\n", new_path.c_str());
				continue;
			}
			pair<dev_t, ino_t> p = { st.st_dev, st.st_ino };
			if (mp.find(p) == mp.end()) {
				mp[p] = new_path;
				q.push(&mp[p]);
			}
        } else if (entry->d_type == DT_REG) {
			if (read_file(new_path)) {
				printf("%s\n", new_path.c_str());
			}
        } else if (entry->d_type == DT_LNK) {
			struct stat st;
			if (stat(new_path.c_str(), &st) == -1) {
				fprintf(stderr, "Cannot get file stat for file: %s\n", new_path.c_str());
				continue;
			}
			if (S_ISDIR(st.st_mode)) {
				pair<dev_t, ino_t> p = { st.st_dev, st.st_ino };
				if (mp.find(p) == mp.end()) {
					mp[p] = new_path;
					q.push(&mp[p]);
				}
			} else if(S_ISREG(st.st_mode)) {
				if (read_file(new_path)) {
					printf("%s\n", new_path.c_str());
				}
			}
        }
    }
    closedir(at);
	if (errno != 0) {
		fprintf(stderr, "Error occured while reading directory: %s\n", path.c_str());
	}
}

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		printf("Usage: mygrep [STRING]\n");
		return 0;
	}

	struct stat st;

	pre(argv[1]);

	string home = ".";
	if (stat(home.c_str(), &st) == -1) {
		fprintf(stderr, "Cannot get file stat for file: %s\n", home.c_str());
		return EXIT_FAILURE;
	}
	q.push(&home);
	
	while (q.size()) {
		walk(*q.front());
		q.pop();
	}
	
	return 0;
}