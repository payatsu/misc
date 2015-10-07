#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<sys/wait.h>
int main(int argc, char* argv[])
{
	int query[2];
	if(pipe(query)==-1){
		std::perror(argv[0]);
		std::exit(EXIT_FAILURE);
	}
	int response[2];
	if(pipe(response)==-1){
		std::perror(argv[0]);
		std::exit(EXIT_FAILURE);
	}
	pid_t questioner_pid;
	if((questioner_pid=fork())==-1){
		std::perror(argv[0]);
		std::exit(EXIT_FAILURE);
	}else if(questioner_pid == 0){
		dup2(query[0], 0);
		close(query[1]);
		dup2(response[1], 1);
		close(response[0]);
		execl("./bin/questioner", "questioner", nullptr);
	}else{
		pid_t solver_pid;
		if((solver_pid=fork())==-1){
			std::perror(argv[0]);
			std::exit(EXIT_FAILURE);
		}else if(solver_pid == 0){
			dup2(response[0], 0);
			close(response[1]);
			dup2(query[1], 1);
			close(query[0]);
			execl("./bin/solver", "solver", nullptr);
		}else{
			int status;
			close(query[0]);
			close(query[1]);
			close(response[0]);
			close(response[1]);
			wait(&status);
			wait(&status);
		}
	}
	return 0;
}
