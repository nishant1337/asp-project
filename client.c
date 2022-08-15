#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define COMMAND(cmd) strcmp(command, cmd)==0

int main(int argc,char *argv[])
{
	char choice[256], arg[256]; //Declaring array of two variable 
	FILE *fd; //Declaring File Descriptor FD
	struct stat obj; // Obj is the variable to keep the count
	char buf[100], command[5], filename[20], *f; //Character Arrays to handle client file and command and *f to navigate and store
	int k, size, status; //Variable to file size and send status
	int filehandle; //Variable filehandle to file between command

	int sock = socket(AF_INET, SOCK_STREAM, 0); // Here, SOCK_Stream is used for TCP Protocol
	if(sock == -1) //Checking if socket was unable to create
	{
		printf("Unable to create Socket");
		exit(1); //Exiting
	}

	struct sockaddr_in server_address; //For taking the Socket address
	server_address.sin_family = AF_INET; //Address family that is used to communicate with Socket
	server_address.sin_addr.s_addr = INADDR_ANY; //Setting the IP address to (INADDR_ANY)Adress integer
	server_address.sin_port = htons(64000); //Setting the socket port number as 64000
	int e = connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)); //To connect to the server
	
	if(e == -1) // Checking if unable to create to server
	{
        printf("Unable to connect to the srever \n");
        exit(1);
    }
    printf("Client has been sucessfully connected with Server\n");
	int i = 1;

	while(1)
	{
		printf("-----------List of Commands------------ \n");
		printf("1-USER: To display the user of system \n");
		printf("2-CWD:  Change working directory \n");
		printf("3-QUIT: To quit from client and server \n");
		printf("4-RETR: Retrive file from Server \n");
		printf("5-STOR: To store file into server \n");
		printf("6-APPE: To append two files and store in server \n");
		printf("7-RNTO: Rename the file to \n");
		printf("8-ABOR: TO exit \n");
		printf("9-DELE: TO Delete any file \n");
		printf("10-RMD: To remove any directory \n");
		printf("11-MKD: To make directory \n");
		printf("12-PWD: To print present working directory \n");
		printf("13-LIST: List all files in given directory and create file which contains files name \n");
		printf("Please Enter your choice: ");
		scanf("%s", choice);

		if(strcmp(choice, "USER") == 0){

			int name = system("whoami"); //TO get the user 
			printf("User name Validated \n");
		}

		else if(strcmp(choice, "RETR") == 0)
		{
			printf("Please enter Filename: ");
			scanf("%c", filename); //To get input from the user
			strcpy(buf, "RETR "); // Copy the string to the buf var
			strcat(buf, filename); // Concatenate the filename to buf
			send(sock, buf, 100, 0); //Sending the BUF to the server for acknowledgment
			recv(sock, &size, sizeof(int), 0); //Get the file size 
			
			if(!size) //Checking for if file not present
			{
				printf("File cannot be found \n");
				break;
			}
			f = (char *)malloc(size); //Dynamically allocating the size
			recv(sock, f, size, 0);
			while(1)
			{
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666); //Creating the file 
				if(filehandle == -1)
				{
					sprintf(filename + strlen(filename), "%d", i);//Checking if same dir is in both client and server
				}
				else 
				{
					break;
				}
			}
			write(filehandle, f, size); //Writing into the file 
			close(filehandle);
			
			strcpy(buf, "cat "); //Copying the buffer 
			strcat(buf, filename); //Concat it 
			printf("Got the file sucessfully \n");
			system(buf);
		}

		else if(strcmp(choice, "STOR") == 0){
			printf("Please enter the filename that you want ot store: ");
			scanf("%s", filename); //User input
			filehandle = open(filename, O_RDONLY); //Opening file in read only mode
			if(filehandle == -1) //Checking if file is n present 
			{
				printf("No such file found \n");
				break;
			}
			strcpy(buf, "STOR "); //Copying the command using strcpy
			strcat(buf, filename); //COncatanating the file name
			send(sock, buf, 100, 0); //Sending the buffer to the server 
			stat(filename, &obj);
			size = obj.st_size; //Getting the file szie
			send(sock, &size, sizeof(int), 0); //Sending the fileszie to the server 
			sendfile(sock, filehandle, NULL, size); //Sending the whole file to server
			recv(sock, &status, sizeof(int), 0); //Recieve on client side
			if(status)
				printf("File stored has been stored on server \n");
			else
				printf("Error has occured while storing file \n");
			
			remove(filename);
		}

		else if(strcmp(choice, "APPE") == 0) //Appending two files into one 
		{
			printf("Enter filename to STOR to server: ");
			scanf("%s", filename); //Getting input
			filehandle = open(filename, O_RDONLY); //Opening file in read only mode
			if(filehandle == -1) //Checking if file is present or not
			{
				printf("No such file found \n");
				break;
			}
			strcpy(buf, "APPE "); //Copying command
			strcat(buf, filename); // Concatenate
			send(sock, buf, 100, 0); //Sending the buf 
			stat(filename, &obj); //Stating the object
			size = obj.st_size; //Getting the file size 
			send(sock, &size, sizeof(int), 0); //Sending the file size to derver
			sendfile(sock, filehandle, NULL, size); //Sending the whole file to the server 
			recv(sock, &status, sizeof(int), 0); //Receiving the file 
			if(status)
				printf("File Appended successfully \n");
			else
				printf("Error occured while Appending \n");
			
			remove(filename);
		}
		else if(strcmp(choice, "RNTO") == 0) //This command is for Rename to 
		{
			strcpy(buf, "RNTO"); //Copying Command
			send(sock, buf, 100, 0); //Sending the buf
			printf("Enter the name of old directory to rename from:");
			scanf("%s", filename); //Taking user input
			send(sock, filename, 100, 0); //Sending the file
			bzero(filename, sizeof(filename)); //Getting the file size 
			printf("Enter the new directory name to rename: ");
			scanf("%s", filename); //Taking user input 
			send(sock, filename, 100, 0); //Sending file to 
			bzero(buf, sizeof(buf));  //Receivng the file size 
			recv(sock, buf, 100, 0); //Receivng file the buf 
			printf("The path of the directory is: %s\n", buf);
		}

		else if(strcmp(choice, "PWD") == 0) //THis command is to print present working Dir
		{
			strcpy(buf, "PWD"); //Copying Command
			send(sock, buf, 100, 0); //Sending the buf
			recv(sock, buf, 100, 0); //Getting the working directory
			printf("Present Working Directory: %s \n", buf); //
		}

		else if(strcmp(choice, "MKD") == 0) //This command is for making dir 
		{

			strcpy(buf, "MKD"); //Copying command
			send(sock, buf, 100, 0); //Sending command to server
			printf("Please enter the directory name to create: \n");
			scanf("%s", filename); //Taking user input
			send(sock, filename, 100, 0); //Sending file name 
			bzero(buf, sizeof(buf)); //Getting size of
			recv(sock, buf, 100, 0); //Receiving the buf from 
			printf("Dir path is: %s\n", buf);
		}

		else if(strcmp(choice, "RMD") == 0) //This command is to remove dir
		{
			strcpy(buf, "RMD"); //Copying command
			send(sock, buf, 100, 0); //Sending command to server
			printf("Please enter the directory name to remove: \n");
			scanf("%s", filename); //Taking user input
			send(sock, filename, 100, 0); //Sending file name
			bzero(buf, sizeof(buf)); //Getting size of
			recv(sock, buf, 100, 0); //Receiving the buf from 
			printf("Directory has been removed sucessfully\n");
		}

		else if(strcmp(choice, "DELE") == 0) //This command is to delete any dir 
		{
			strcpy(buf, "DELE"); //Copying command
			send(sock, buf, 100, 0); //Sending command to server
			printf("Please enter the file name to delete: \n");
			scanf("%s", filename); //Taking user input
			send(sock, filename, 100, 0); //Sending file name
			bzero(buf, sizeof(buf)); //Getting the size of 
			recv(sock, buf, 100, 0); //receiving the buf from
			printf("Directory deleted sucessfully \n");
		}

		else if(strcmp(choice, "LIST") == 0) //THis comand is used to list all the directory
		{
			strcpy(buf, "LIST"); //Sending command to server
			send(sock, buf, 100, 0); //Sending the buffer variable to the server 
			recv(sock, buf, 1024, 0); //Receiving the buffer variable from server
			printf("The remote directory listing is as follows:\n %s\n\n", buf);
		}

		else if(strcmp(choice, "CWD") == 0) //This command is used to change the current working dir 
		{
			strcpy(buf, "CWD "); //Copying command
			printf("Please enter the path to change the directory: \n");
			scanf("%s", buf + 3); //getting the user input 
			send(sock, buf, 100, 0); //Sending the buffer variable to the server
			recv(sock, &status, sizeof(int), 0); //Receiving the status from server
			if(status) //Checking the status
				printf("Directory has been changed sucessfully \n");
			else
				printf("Error occured while changing the directory\n");
		}


		else if(strcmp(choice, "QUIT") == 0) //This command is used to quit from client and server 
		{
			strcpy(buf, "QUIT"); //Copying command
			send(sock, buf, 100, 0); //Sending the buffer variable to the server
			recv(sock, &status, 100, 0); //Receiving the status from server
			if(status) //Checking the status
			{
				printf("Connection has been TERMINATED \n");
				close(sock); //Clossing the socket 
				exit(0);
			}
			printf("Error occured while executing QUIT command\n");
		}
	}	
	return 0;
}