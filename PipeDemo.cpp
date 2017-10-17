#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

const int PIPE_COUNT = 2;
const int PIPE_READ_END = 0;
const int PIPE_WRITE_END = 1;

const int STDIN = 0;
const int STDOUT = 1;

int main()
{
	int pids[PIPE_COUNT];
	pipe(pids);

	int savedStdout = dup(STDOUT);
	int savedStdin = dup(STDIN);

	//
	// First child will output the source code to this program to the pipe
	pid_t pid = fork();
	if (pid == 0)
	{
		dup2(pids[PIPE_WRITE_END], STDOUT);

		char** argv = new char*[3];
		argv[0] = new char[4];
		strcpy(argv[0], "cat");
		argv[1] = new char[13];
		strcpy(argv[1], "PipeDemo.cpp");
		argv[2] = NULL;

		execvp(argv[0], argv);
	}

	//
	// Second child will 'more' whatever input comes down over the pipe
	pid_t pid2 = fork();
	if (pid2 == 0)
	{
		dup2(pids[PIPE_READ_END], STDIN);
		//
		// This is key, in order to terminate the input from the pipe
		// have to close off the write end, otherwise the 'more' command
		// will continue to wait for additional data.
		close(pids[PIPE_WRITE_END]);

		char** argv = new char*[2];
		argv[0] = new char[5];
		strcpy(argv[0], "more");
		argv[1] = NULL;

		execvp(argv[0], argv);
	}

	//
	// Wait for the first child to finish
	int status;
	waitpid(pid, &status, 0);

	//
	// Fully close down the pipe, and yes, for whatever reason, it requires
	// the parent process to close both ends, even though the second child
	// already closed the write end...not sure I fully understand this.
	close(pids[PIPE_WRITE_END]);
	close(pids[PIPE_READ_END]);

	waitpid(pid2, &status, 0);

	//
	// Restore standard out and in, so our program will be back to normal when complete
	dup2(savedStdout, STDOUT);
	dup2(savedStdin, STDIN);

	std::cout << "Enter something: ";
	std::string input;
	std::getline(std::cin, input);
	std::cout << "You entered..." << input << std::endl;

	return 0;
}
