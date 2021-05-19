#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int history_size = 0, background_size = 0;

// A node structure for history linked list
struct History
{
	int number;
	char contents[256]; 
	struct History* next;
};

// A node structure to store currently running background processes
struct Processes
{
	pid_t pid;
	int number;
	char process_name[256];
	struct Processes* next;
};

// Initializer a new background process linked list
struct Processes* init_background()
{
	// Create the starting node in the background process linked list
	struct Processes* bg_head = (struct Processes*) malloc(sizeof(struct Processes));
	bg_head->number = 0;
	bg_head->next = NULL;
	bg_head->pid = 0;
	strcpy(bg_head->process_name, " ");

	// Let our program know the history linked list is empty	
	background_size = 0;
	
	return bg_head;
}

// Inserts a given command into the history linked list
void add_process(struct Processes* head, pid_t pid, char* program)
{
	if(history_size == 0)
	{
		head->pid = pid;
		head->number = 0;
		strcpy(head->process_name, program);
		background_size++;
		return;
	}

	struct Processes* curr_item = head;
	struct Processes* new_entry = (struct Processes*) malloc(sizeof(struct Processes));
	new_entry->pid = pid;
	new_entry->number = background_size;
	strcpy(new_entry->process_name, program);	
		
	while(curr_item->next != NULL)
	{
		curr_item = curr_item->next;
	}
	
	curr_item->next = new_entry;
	background_size++;	
}

// Print every item in the history linked list
void display_processes(struct Processes* head)
{
	struct Processes* curr_item = head;

	while(curr_item != NULL)
	{
		printf("%d:  %s:  process number %d\n", curr_item->number, curr_item->process_name, curr_item->pid);
		curr_item = curr_item->next;
	}
}

// Initializes a new history linked list
struct History* init_history()
{
	// Check to see if a history file already exists
	FILE *fp;
	if(access("history.txt", F_OK) == 0)
	{
	}
	else
	{
		// Creates our history.txt
		fclose(fopen("history.txt", "w"));
	}


	// Create the starting node in the history linked list
	struct History* head = (struct History*) malloc(sizeof(struct History));
	head->next = NULL;

	// Let our program know the history linked list is empty	
	history_size = 0;
	
	return head;
}

// Frees all nodes in the linked list and deletes the old history.txt
struct History* clear_history(struct History* head)
{
	// Deletes all contents of history.txt
	fclose(fopen("history.txt", "w"));
	
	// Loop from head to tail and free all of the linked list pointers
	struct History* original = head;
	struct History* curr_item = head;
	struct History* next_item = NULL;
	
	while(curr_item != NULL)
	{
		if(curr_item == original)
		{
			strcpy(head->contents, " ");
			next_item = curr_item->next;
			curr_item = next_item;
			continue;
		}
		next_item = curr_item->next;
		free(curr_item);
		curr_item = next_item;
	}

	return original;
}

// Print every item in the history linked list
void display_history(struct History* head)
{
	struct History* curr_item = head;

	while(curr_item != NULL)
	{
		printf("%d: %s\n", curr_item->number, curr_item->contents);
		curr_item = curr_item->next;
	}
}

// Inserts a given command into the history linked list
void add_history(struct History* head, char input[256])
{
	if(history_size == 0)
	{
		strcpy(head->contents, input);
		history_size++;
		return;
	}

	struct History* curr_item = head;
	struct History* new_entry = (struct History*) malloc(sizeof(struct History));
	strcpy(new_entry->contents, input);	
		
	while(curr_item->next != NULL)
	{
		curr_item = curr_item->next;
	}
	
	curr_item->next = new_entry;
	new_entry->number = history_size;
	history_size++;	
}

// Pulls history from history.txt and loads it into the linked list
struct History* load_history(struct History* head)
{
	// Open the history file
	FILE *fp = fopen("history.txt", "r");
	char line[256];

	while(fgets(line, sizeof(line), fp))
	{
		line[strlen(line) - 1] = '\0'; // strip newline char from 
		add_history(head, line);
    }
	
	fclose(fp);
}

// Initiated during byebye command, this function loads the history linked list into a text file
void store_history(struct History* head)
{
	FILE *fp;
	fp = fopen("history.txt", "w");
	struct History* temp = head;

	for(int i = 0; i < history_size - 1; i++)
	{
		fprintf(fp, temp->contents, i + 1);
		fprintf(fp, "\n", i + 1);
		temp = temp->next;
	}

	fclose(fp);
}

// Traverse linked list and return the command stored at the nth posiiton
char* find_command(struct History* head, char* n)
{
	char *token;
	int number = atoi(n);
	struct History* curr_item = head;
	
	while(curr_item != NULL)
	{
		if(curr_item->number == number)
		{
			token = strtok(curr_item->contents, " "); // Tokenize user input
			return token;
		}

		curr_item = curr_item->next;
	}
	
	printf("That command number does not exist.\n");
	return NULL; 
}

// Path type 1 is direct path, path type 2 is relative path
void start_program(char* program, int path_type)
{
	printf("Attempting to start: %s\n", program);

	char* args[] = {program, NULL};
	pid_t pid = fork(); 
	
	// Check to make sure fork() worked
	if(pid < 0)
	{
		perror("FORK FAILED");
		return;
	}

	// Check to see if we are at the child process
	else if(pid == 0)
	{
		// Direct Path
		if(path_type == 1)
		{
			printf("starting process\n");
			execv(args[0], args);
		}
		// Relative Path
		else if(path_type == 2)
		{
			printf("starting process\n");
			execv(args[0], args);
		}
		else
			perror("Something went wrong.\n");
	}

	// Check to see if we are at the parent process
	if(pid > 0)
	{
		// printf("Parent PID: %d\n", getpid());
		waitpid(pid, NULL, 0);
	}

	if(pid == 0)
		perror("The given program could not be found\n");

	return;
}

// Use start_program() to run an executable and also print the PID of the program
void background_program(char* program, int path_type, struct Processes* background)
{
	printf("Attempting to start in back: %s\n", program);

	char* args[] = {program, NULL};
	pid_t pid = fork(); 
	
	// Check to make sure fork() worked
	if(pid < 0)
	{
		perror("FORK FAILED");
		return;
	}

	// Check to see if we are at the child process
	else if(pid == 0)
	{
		// Direct Path
		if(path_type == 1)
		{
			printf("starting process\n");
			execv(args[0], args);
		}
		// Relative Path
		else if(path_type == 2)
		{
			printf("starting process\n");
			execv(args[0], args);
		}
		else
			perror("Something went wrong.\n");
	}

	// Check to see if we are at the parent process
	if(pid > 0)
	{
		// printf("Parent PID: %d\n", getpid());
	}

	if(pid == 0)
		perror("The given program could not be found\n");

	printf("%s was started with a PID of %d\n", program, pid);

	// Add this pid to our list of background processes
	add_process(background, pid, program);

	return;
}

// Terminate a program that is provided by a PID
// Print the success or failure of the opperation
// UNDER CONSTRUCTION
void exterminate_list(char* pid_number, struct Processes* background)
{
	int pid = atoi(pid_number);
	pid_t current_pid;
	struct Processes* curr_item = background;

	while(curr_item->next != NULL)
	{
		if(curr_item->number == pid)
		{
			current_pid = curr_item->pid;
			printf("Curent PID: %d\n", current_pid);
			break;
		}

		curr_item = curr_item->next;
	}

	printf("Killing PID: %d\n", curr_item->pid);
	kill(curr_item->pid, SIGKILL);

	return;
}

void exterminate_pid(char* pid_number, struct Processes* background)
{
	int pid = atoi(pid_number);
	pid_t current_pid = pid;

	printf("Killing PID: %d\n", current_pid);
	kill(current_pid, SIGKILL);

	return;
}

// Create a new file in the current directory with the word "Draft" in it
void make_file(char* file_name, char* cur_dir)
{
	// Creates the path to our new file
	char new_file[256];
	strcpy(new_file, cur_dir);
	strcat(new_file, "/");
	strcat(new_file, file_name);

	// Check to see if the new file already exists
	if(access(new_file, F_OK) == 0)
	{
		printf("Sorry, a file with that name already exists\n");
	}
	else
	{		
		FILE *fp;
		fp = fopen(new_file, "w");
		fprintf(fp, "Draft", 1);
		fclose(fp);
	}
}

// Check to see if a file exists or if it is actually a directory
void dwelt(char* file_name, char* cur_dir)
{
	// Creates the path to our new file
	int num_forwardslash = 0;
	char new_file[256], directory[256];

	// If the path starts with a /, it is an absolute path
	if(file_name[0] == 47)
	{
		strcpy(new_file, file_name);
	}
	else // We were given a relative path
	{
		strcpy(new_file, cur_dir);
		strcat(new_file, "/");
		strcat(new_file, file_name);
	}

	// Check to see if the new file already exists
	DIR *dir = opendir(new_file);
	if(dir)
	{
		printf("Abode is\n");
		return;
	}
	if(access(new_file, F_OK) == 0)
	{
		printf("Dwelt Indeed\n");
		return;
	}
	else
	{
		printf("Dwelt not\n");
		return;
	}
}

// Copy a file in the current directory to another directory
void copyTo(char* source, char* destination, char* cur_dir)
{
	int num_forwardslash = 0, exit = 0;
	char new_file_destination[256], new_file_source[256], dest_path[256], ch;
	memset(new_file_destination, 0, sizeof(new_file_destination));
	memset(new_file_source, 0, sizeof(new_file_source));
	memset(dest_path, 0, sizeof(dest_path));

	// If the path starts with a /, it is ready to go
	if(source[0] == 47)
	{
		strcpy(new_file_source, source);
	}
	else // We were given a relative path, so we need to append it to the current directory
	{
		strcpy(new_file_source, cur_dir);
		strcat(new_file_source, "/");
		strcat(new_file_source, source);
	}

	// Get the number of / in the given path
	for(int i = 0; i < strlen(destination); i++)
		if(destination[i] == 47)
			num_forwardslash++;

	// If the number of / is greater than 0, we were given an absolute path, not a relative one
	if(num_forwardslash > 0)
	{
		for(int i = 0; i < strlen(destination); i++)
		{
			if(num_forwardslash == 1 && destination[i] == 47)
				break;

			if(destination[i] == 47)
				num_forwardslash--;

			dest_path[i] = destination[i];
		}
	}

	if(destination[0] == 47)
	{
		strcpy(new_file_destination, destination);
	}
	else
	{
		strcpy(new_file_destination, cur_dir);
		strcat(new_file_destination, "/");
		strcat(new_file_destination, destination);
	}
	
	// Check to see if the source file exists
	if(access(new_file_source, F_OK) != 0)
	{
		printf("The source file %s does not exist.\n", new_file_source);

		exit = 1;
	}

	// Check to see if a file with the given name already exists in the destination
	if(access(destination, F_OK) == 0)
	{
		printf("Sorry, a file with that name already exists in the given destination.\n");
		exit = 1;
	}

	if(num_forwardslash > 0)
	{
		// Check to see if destination exists
		DIR *dir = opendir(dest_path);
		if(!dir)
		{
			printf("The given destination does not exist.\n");
			exit = 1;
		}
	}

	// If any of our flags were triggered above, we cannot copy the file
	if(exit)
		return;

	FILE *sourceFile;
	FILE *destFile;

	sourceFile = fopen(new_file_source, "r");
    destFile = fopen(new_file_destination, "w");

    ch = fgetc(sourceFile);
    while (ch != EOF)
    {
        fputc(ch, destFile);
        ch = fgetc(sourceFile);
    }

    printf("\nFiles copied successfully.\n\n");

    fclose(sourceFile);
    fclose(destFile);

	return;
}

// Gives the user a list of all known commands and how to use them
void display_commands()
{
	printf("WORKING:\n\n");
	printf("-h: 			     Help menu.\n");
	printf("byebye:			     Quit mysh.\n");
	printf("whoami:   		     Print user name.\n");
	printf("history: 		     A list of all recently used commands. Use the -c flag to clear.\n");
	printf("whereami:                    Print the current working directory.\n");
	printf("replay n:		     Re-execute the nth command in history.\n");
	printf("dalek PID:		     Uses SIGKILL to terminate a program specified by a PID.\n");
	printf("start /path/to/exe:          Start assumes you provided a path to the program exe. Can accept parameters.\n");
	printf("start program_name:          Start assumes the program is in the current directory. Can accept parameters.\n");
	printf("movetodir /path/to/dir:      Changes your current directory to the newly provided one.\n");
	printf("background /path/to/exe:     Starts a program in the background and displays the PID to the user.\n");
	printf("background program_name:     Starts a program in the background and displays the PID to the user.\n");
	printf("\nNEW ! ! !\n");
	printf("maik:			     Makes a file with the word 'Draft' written inside. Can accept relative or absolute path.\n");
	printf("dwelt:			     Checks to see if a file or directory exists. Can accept relative or absolute path.\n");
	printf("coppy:			     Copies a file from one location to another. Can accept relative or absolute path.\n");
	printf("\n\nIN PRODUCTION:\n\n");
	printf("repeat n command:            Runs a command an n amount of times.\n");
	printf("dalekall:		     Terminates all programs previously started by mysh.\n\n");
	printf("\nFor ABSOLUTE paths, please begin the path with '/'. For RELATIVE paths, please do NOT begin the path with '/'.\n\n");
}

// Start of the program
int main(void)
{	
	// Important vaiables
	char* token;
	char s[256], temp[256];
	char cur_dir[256] = "/home/dillon/Documents/School/21SP/OS/HW/myshv2", temp_dir[256], cur_usr[50] = "user";
	char location[256], destination[256];
	int token_len = 0, new_token_len = 0;
	int active = 1, is_command = 0, is_replay = 0;
	
	struct History* head; // Create the head of the history linked list
	head = init_history(); // Initialize our history data structure
	load_history(head); // Load the history.txt file into the linked list

	struct Processes* background_head; // Create the head of the background process linked list
	background_head = init_background(); // Initialize the background data 
	
	printf("\nPlease use '-h' at any time to see a list of commands.\n\n");
		
	// Main loop to take in user input while mysh is running
	while(active == 1)
	{
		printf("[%s %s] # ", cur_usr, cur_dir); 
		scanf("%[^\n]%*c", s); // Take in user command and arguments
		token_len = strlen(s); // Capture the length of the tokenized user input
		new_token_len = token_len; // Copy the length of the tokenized user input
		token = strtok(s, " "); // Tokenize user input

		// Parse the tokenized user input
		while (token != NULL)
		{
			is_command = 0;
			is_replay = 0;
			
			if(strcmp(token, "replay") == 0)
			{
				add_history(head, token);
				token = find_command(head, token+7);
				add_history(head, token);
				is_replay = 1;
				is_command = 1;
			}

			if(strcmp(token, "-h") == 0)
			{
				add_history(head, token);
				display_commands();
				is_command = 1;
			}
			
			if(strcmp(token, "movetodir") == 0)
			{
				char backslash[256];
				strcpy(backslash, token+10);

				// Direct Path
				if(backslash[0] == 47) // 47 is the numeric value of /
				{
					DIR *dir = opendir(token+10);
					if(dir)
						strcpy(cur_dir, token+10);
					else
						printf("It looks like that directory doesn't exist.\n");
				
					// Logic for concatenating 2 parts of the token to pass to history
					strcpy(temp, token+10);
					strcat(token, " ");
					strcat(token, temp);
					add_history(head, token);
					is_command = 1;
				}

				// Check for relative path
				else
				{
					strcpy(temp_dir, cur_dir);
					strcat(temp_dir, "/");
					strcat(temp_dir, token+10);

					DIR *dir = opendir(temp_dir);
					if(dir)
						strcpy(cur_dir, temp_dir);
					else
						printf("It looks like that directory doesn't exist.\n");
					
					add_history(head, token);
					is_command = 1;
				}
			}
			
			if(strcmp(token, "whereami") == 0)
			{
				printf(cur_dir);
				printf("\n");
				add_history(head, token);
				is_command = 1;
			}

			if(strcmp(token, "whoami") == 0)
			{
				printf(cur_usr);
				printf("\n");
				add_history(head, token);
				is_command = 1;
			}
			
			if(strcmp(token, "history") == 0)
			{
				if(strcmp(token+8, "-c") == 0)
				{
					// printf("-- History Cleared --\n");
					head = clear_history(head);
					head->next = NULL;
					history_size = 0;
				}
					
				else
					display_history(head);
				
				add_history(head, token);
				strcpy(token, " ");
				is_command = 1;
			}
			
			if(strcmp(token, "byebye") == 0)
			{
				active = 0;
				add_history(head, token);
				store_history(head);
				is_command = 1;
				printf("Exiting Mysh\n");
				exit(1);
			}
			
			if(strcmp(token, "start") == 0)
			{
				char backslash[256];
				strcpy(backslash, token+10);

				// Direct Path
				if(backslash[0] == 47) // 47 is the numeric value of '/'
				{
					start_program(token+6, 1);
					add_history(head, token);
					is_command = 1;
				}

				// Relative Path
				else
				{
					strcpy(temp_dir, cur_dir);
					strcat(temp_dir, "/");
					strcat(temp_dir, token+6);

					start_program(temp_dir, 2);
					add_history(head, temp_dir);
					is_command = 1;
				}

				/*
					Logic to capture parameters

					while(token_len > 0)
					{
					}
				*/
			}
			
			if(strcmp(token, "background") == 0)
			{
				if(strcmp(token+11, "-ls") == 0)
				{
					display_processes(background_head);
					token = NULL;
					continue;
				}

				else
				{
					char backslash[256];
					printf("Attempting to start: %s\n", token+11);

					strcpy(backslash, token+11);

					// Direct Path
					if(backslash[0] == 47) // 47 is the numeric value of '/'
					{
						background_program(token+11, 1, background_head);
						sleep(1);
						add_history(head, token);
						is_command = 1;
					}

					// Relative Path
					else
					{
						strcpy(temp_dir, cur_dir);
						strcat(temp_dir, "/");
						strcat(temp_dir, token+11);

						background_program(temp_dir, 2, background_head);
						sleep(1);
						add_history(head, temp_dir);
						is_command = 1;
					}
				}
			}
			
			if(strcmp(token, "dalek") == 0)
			{
				exterminate_pid(token+6, background_head);
				is_command = 1;
			}

			if(strcmp(token, "maik") == 0)
			{
				make_file(token+5, cur_dir);
				is_command = 1;
			}
			
			if(strcmp(token, "dwelt") == 0)
			{
				dwelt(token+6, cur_dir);
				is_command = 1;
			}

			if(strcmp(token, "coppy") == 0)
			{
				int space = 0, dest;
				char source_location[50], history_item[256];
				memset(location, 0, sizeof(location));
				memset(source_location, 0, sizeof(source_location));
				strcpy(location, token+6); // skip past the word 'coppy'
				strcpy(location, location+5); // remove the word 'from'

				// put in safegaurds to check for 'from-'
				if(token[6] != 'f' || token[7] != 'r' || token[8] != 'o' || token[9] != 'm' || token[10] != '-')
				{
					printf("Could not find keyword: from-\n");
					break;
				}
				if(token[11] == ' ')
				{
					printf("That is not a valid source location, please try again.\n");
					break;
				}

				// Parse out the source location from the token
				for(int i = 0; i < (strlen(location)); i++)
				{
					if(location[i] == 32) // space is 32
						space++;

					// Until we hit a space, we are getting the source location
					if(space == 0)
						source_location[i] = location[i];
				}

				dest = 15 + strlen(source_location); // 15 because: 6 to skip 'coppy', 5 to skip 'from', for to skip 'to'
				copyTo(source_location, token + dest, cur_dir);

				// zero out our source location string
				memset(source_location, 0, sizeof(source_location));

				strcpy(history_item, token);
				strcat(history_item, " ");
				strcat(history_item, token + 6);
				add_history(head, history_item);

				is_command = 1;
			}

			if(is_replay != 1)
				token = strtok(NULL, " ");
		}
	}

	return 0;

}