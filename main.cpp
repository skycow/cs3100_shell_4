#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iomanip>

using namespace std;

string getInput()
{
  string input;
  cout << "[" << get_current_dir_name() <<  "]: " ;
  getline(cin, input);
  return input;
}

void myHandler(int signalId)
{
  getInput();
}

void printHistory(vector<string> history)
{
  for( int i=0; i < history.size(); i++ )
  {
    cout << i+1 << ": " << history[i] << endl;
  }
}

void printCommandError(string input)
{
  cout << "Error: " << input << " is an invalid command" << endl;
}

createandexec(string input)
{
  vector<string> newinputs;
  int first=0;

  while(input[0] == ' ')
  {
    input.erase(0,1);
  }
  while(input[input.size()-1] == ' ')
  {
    input.erase(input.end());
  }

  for(int i = 0; i < input.size(); i++)
  {
    if(input[i] == ' ')
    {
      newinputs.push_back(input.substr(first, i-first));
      if(i+1 < input.size())
      {
        int iter = 1;
        while(input[i+iter] == ' ')
        {
          input.erase(i+iter,1);
        }
        first = i+1;
      }
    }
  }
  newinputs.push_back(input.substr(first));

  char ** newargv = new char*[newinputs.size()+1];

  for(int i = 0; i < newinputs.size(); i++)
  {
    newargv[i] = new char[newinputs[i].size()+1];
    strcpy(newargv[i], newinputs[i].c_str());
  }
  newargv[newinputs.size()] = NULL;


  execvp(newargv[0], newargv);
  printCommandError(input);
  exit(0);
}

int execwithpipe(string input)
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

		createandexec(input.substr(0,input.find('|')))
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

		createandexec(input.substr(input.find('|')))
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
}

int main(int argc, char* argv[])
{
  vector<string> history;
  string input;
  std::chrono::duration<double> time(0);

  signal(SIGINT, myHandler);

  input = getInput();

  while(input != "exit")
  {
      bool error(false);
      if (input[0] == '^')
      {
        input = input.substr(2);
        int index = stoi(input)-1;
        if(index < history.size())
        {
          input = history[index];
        }
        else
        {
          cout << "Error: Command index " << input << " not in history." << endl;
          error = true;
        }
      }

      if(!error)
      {
    if(input == "history")
    {
      history.push_back(input);
      printHistory(history);
    }
    else if(input == "ptime")
    {
      history.push_back(input);
      cout << "Time spent executing child processes: " << fixed << setprecision(4) << time.count() << " seconds" << endl;
    }
    else if(input.substr(0,2) == "cd")
    {
      cout << "got in the cd section" << endl;
      history.push_back(input);

      chdir(input.substr(input.begin()+3));

    }
    else if(input.find('|') != std::string::npos)
    {
      execwithpipe(input);
    }
    else
    {

        history.push_back(input);

        //auto pid = fork();

        if(fork())
        {

          std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
          wait(NULL);
          std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();

          time += end - start;

        }
        else
        {
          createandexec(input);
        }
      }
    }

    input = getInput();
  }

  return 0;
}


/*
#include <iostream>
#include <functional>
#include <signal.h>

void myHandler(int signalId)
{

    std::cout << "The signal ID is: " << signalId << std::endl;
}

void indirection(void (*callMe)(int))
{

    callMe(30);
}

void indirection2(std::function<void(int)> callMe)
{

    callMe(50);
}

int main()
{
    //void (*func)(int) = myHandler;
    //std::function<void(int)> func2 = myHandler;

    //func(10);
    //myHandler(20);
    //indirection(myHandler);
    //func2(40);
    //indirection2(myHandler);

    signal(SIGINT, myHandler);
    signal(SIGKILL, myHandler);
    signal(SIGFPE, myHandler);

    int a = 1;
    int b = 0;

    int r = a / b;

    std::cout << "Enter anything to quit...";
    int input;
    std::cin >> input;

    return 0;
}

*/

////////////////////////////


/*
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
*/
