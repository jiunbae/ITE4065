#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

//const unsigned BATCH_SIZE = 1000;
const unsigned long MAX_FAILED_QUERIES = 100;

// Print the usage instructions for the harness
void usage()
{
	std::cerr << "Usage: marker <init-file> <workload-file> <result-file> <test-executable>" << std::endl;
}

// Set a file descriptor to be non-blocking
int set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return flags;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Read a given number of bytes to the specified file descriptor
ssize_t read_bytes(int fd, void *buffer, size_t num_bytes)
{
	char *p = (char *)buffer;
	char *end = p + num_bytes;
	while (p != end)
	{
		ssize_t res = read(fd, p, end - p);
		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			return res;
		}
		p += res;
	}

	return num_bytes;
}

// Write a given number of bytes to the specified file descriptor
ssize_t write_bytes(int fd, const void *buffer, size_t num_bytes)
{
	const char *p = (const char *)buffer;
	const char *end = p + num_bytes;
	while (p != end)
	{
		ssize_t res = write(fd, p, end - p);
		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			return res;
		}
		p += res;
	}

	return num_bytes;
}

int main(int argc, char *argv[])
{
	// Check for the correct number of arguments
	if (argc != 5)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	std::vector<std::string> input_batches;
	std::vector<std::vector<std::string> > result_batches;

	// Load the workload and result files and parse them into batches
	{
		std::ifstream work_file(argv[2]);
		if (!work_file)
		{
			std::cerr << "Cannot open workload file" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::ifstream result_file(argv[3]);
		if (!result_file)
		{
			std::cerr << "Cannot open result file" << std::endl;
			exit(EXIT_FAILURE);
		}

		bool eof = false;
		while (!eof)
		{
			std::string input_chunk;
			input_chunk.reserve(1000000);

			std::vector<std::string> result_chunk;
			//result_chunk.reserve(BATCH_SIZE);

			std::string line;
			while(1)//for (unsigned long line_no = 0; line_no != BATCH_SIZE; ++line_no)
			{
				eof = !std::getline(work_file, line);
				if (eof)
					break;
				input_chunk += line;
				input_chunk += '\n';

				if (line.length() > 0 && (line[0] == 'Q' || line[0] == 'q'))
				{
					std::string result;
                                       
					std::getline(result_file,result);

					result_chunk.push_back(result);
				}
			}

			if (!input_chunk.empty())
			{
				input_batches.push_back(input_chunk); // copy to avoid accumulation of unused space in each chunk
				result_batches.push_back(result_chunk); // copy may be avoided in C++ 11 with std::move
			}
		}
	}

	// Create pipes for child communication
	int stdin_pipe[2];
	int stdout_pipe[2];
	if (pipe(stdin_pipe) == -1
			|| pipe(stdout_pipe) == -1)
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	// Start the test executable
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		dup2(stdin_pipe[0], STDIN_FILENO);
		close(stdin_pipe[0]);
		close(stdin_pipe[1]);
		dup2(stdout_pipe[1], STDOUT_FILENO);
		close(stdout_pipe[0]);
		close(stdout_pipe[1]);
		execlp(argv[4], argv[4], (char *) NULL);
		perror("execlp");
		exit(EXIT_FAILURE);
	}
	close(stdin_pipe[0]);
	close(stdout_pipe[1]);

	// Open the file and feed the initial graph
	int init_file = open(argv[1], O_RDONLY);
	if (init_file == -1)
	{
		std::cerr << "Cannot open init file" << std::endl;
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		char buffer[4096];
		ssize_t bytes = read(init_file, buffer, sizeof(buffer));
		if (bytes < 0)
		{
			if (errno == EINTR)
				continue;
			perror("read");
			exit(EXIT_FAILURE);
		}
		if (bytes == 0)
			break;
		ssize_t written = write_bytes(stdin_pipe[1], buffer, bytes);
		if (written < 0)
		{
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	close(init_file);

	char status_buffer[2];
	ssize_t status_bytes = read_bytes(stdout_pipe[0], status_buffer, sizeof(status_buffer));
	if (status_bytes < 0)
	{
		perror("read");
		exit(EXIT_FAILURE);
	}

	if (status_bytes != 2
			|| (status_buffer[0] != 'R' && status_buffer[0] != 'r')
			|| status_buffer[1] != '\n')
	{
		std::cerr << "Test program did not return ready status" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Use select with non-blocking files to read and write from the child process, avoiding deadlocks
	if (set_nonblocking(stdout_pipe[0]) == -1)
	{
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	if (set_nonblocking(stdin_pipe[1]) == -1)
	{
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	// Start the stopwatch
	struct timeval start;
	gettimeofday(&start, NULL);

	unsigned long query_no = 0;
	unsigned long failure_cnt = 0;

	// Loop over all batches
	for (unsigned long batch = 0; batch != input_batches.size() && failure_cnt < MAX_FAILED_QUERIES; ++batch)
	{
		std::string output; // raw output is collected here
		output.reserve(1000000);

		size_t input_ofs = 0; // byte position in the input batch
		size_t output_read = 0; // number of lines read from the child output

		while (input_ofs != input_batches[batch].length() || output_read != result_batches[batch].size())
		{
			fd_set read_fd, write_fd;
			FD_ZERO(&read_fd);
			FD_ZERO(&write_fd);

			if (input_ofs != input_batches[batch].length())
				FD_SET(stdin_pipe[1], &write_fd);

			if (output_read != result_batches[batch].size())
				FD_SET(stdout_pipe[0], &read_fd);

			int retval = select(std::max(stdin_pipe[1], stdout_pipe[0]) + 1, &read_fd, &write_fd, NULL, NULL);
			if (retval == -1)
			{
				perror("select");
				exit(EXIT_FAILURE);
			}

			// Read output from the test program
			if (FD_ISSET(stdout_pipe[0], &read_fd))
			{
				char buffer[4096];
				int bytes = read(stdout_pipe[0], buffer, sizeof(buffer));
				if (bytes < 0)
				{
					if (errno == EINTR)
						continue;
					perror("read");
					exit(1);
				}
				// Count how many lines were returned
				for (size_t j = 0; j != size_t(bytes); ++j)
				{
					if (buffer[j] == '\n')
						++output_read;
				}
				output.append(buffer, bytes);
			}

			// Feed another chunk of data from this batch to the test program
			if (FD_ISSET(stdin_pipe[1], &write_fd))
			{
				int bytes = write(stdin_pipe[1], input_batches[batch].data() + input_ofs, input_batches[batch].length() - input_ofs);
				if (bytes < 0)
				{
					if (errno == EINTR)
						continue;
					perror("write");
					exit(EXIT_FAILURE);
				}
				input_ofs += bytes;
			}
		}

		// Parse and compare the batch result
		std::stringstream result(output);
                
		for (unsigned i = 0; i != result_batches[batch].size() && failure_cnt < MAX_FAILED_QUERIES; ++i)
		{
			std::string val;
                        
			//result >> val;
                        std::getline(result,val);
			if (!result)
			{
				std::cerr << "Incomplete batch output for batch " << batch << std::endl;
				exit(EXIT_FAILURE);
			}

			bool matched = val == result_batches[batch][i];
			if (!matched)
			{
				std::cerr << "Result mismatch for query " << query_no << ", expected: " << result_batches[batch][i] << ", actual: " << val << std::endl;
				++failure_cnt;
			}
                        /*if (matched)
                        {
                            std::cout << std::endl << val << std::endl <<  std::endl << result_batches[batch][i];
                        }*/
			++query_no;
		}
	}

	struct timeval end;
	gettimeofday(&end, NULL);

	if (failure_cnt == 0)
	{
		// Output the elapsed time in milliseconds
        double elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
		std::cout << "Passed!" << std::endl;
        std::cout << "Elapsed time:" << (long) (elapsed_sec * 1000) << " ms"<< std::endl;
		return EXIT_SUCCESS;
	} else
    {
		std::cout << "Failed!" << std::endl;
    }

	return EXIT_FAILURE;
}
