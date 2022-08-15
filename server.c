#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
struct stat st = {0}; 

int main(int argc,char *argv[])
{
	struct sockaddr_in server, client; //Variable used Server and Client for socket Address
	struct stat obj; // Obj is the variable to keep the count 
	socklen_t sock2, len; //Variable len to keep count for length and sock2 to listen to client 
	char buf[100], command[5], filename[20]; //Character Arrays to handle client file and command
	int k, i, size, c; //Variable to handle client commands 
	int filehandle, sock1; //Varible filehandle to handles file between commands and sock1 to handle socket
 
	sock1 = socket(AF_INET, SOCK_STREAM, 0); // Here, SOCK_Stream is used for TCP Protocol
	if(sock1 == -1) //Checking if socket was unable to create
	{
		printf("Unable to create Socket"); 
		exit(1); //Exiting 
	}

	struct sockaddr_in server_address; //For taking the Socket address
	server_address.sin_family = AF_INET; //Address family that is used to communicate with Socket
	server_address.sin_port = htons(64000); //Setting the socket port number as 64000
	server_address.sin_addr.s_addr = INADDR_ANY; //Setting the IP address to (INADDR_ANY)Adress integer

	k = bind(sock1,(struct sockaddr*)&server_address, sizeof(server_address)); //Here, I am biding the socket
	if(k == -1) //Checking if unable to bind
	{
		printf("Unable to Bind the Socket");  
		exit(1); //Exiting
	}

	k = listen(sock1,5); //Listening to the connection and at a time 5 clents can be opened simultaneously 
	if(k == -1) //Checking if unable to listen
	{
		printf("Unable to listen to Client");
		exit(1);
	}

	while(1) //If the condtion are ture or 1 then executing the function
	{
		printf("Waiting for the Clients to connect");
		len = sizeof(client);
		sock2 = accept(sock1,(struct sockaddr*)&client, &len); //Allowing all the clients(5) to connect 
		i = 1;
		while(1) //If the client are accepted then allowing client's to execute the command using CLI
		{
			recv(sock2, buf, 100, 0); 
			sscanf(buf, "%s", command);

			if(!strcmp(command,"RETR")) //This command retrive any file from server
			{
				
				sscanf(buf, "%s%s", command, filename); //Seprating the command and the filename given by client
				stat(filename, &obj);

				filehandle = open(filename, O_RDONLY); //Opeaning file in Read only
				size = obj.st_size; //Using the system fun to get the file size 
				if(filehandle == -1) //checking If file dosen't exist
				{	
					size = 0;
				}
				send(sock2, &size, sizeof(int), 0); //Sending the size of file to client

				if(size) //Sending the file to client
				{
					sendfile(sock2, filehandle, NULL, size);
				}
			}

			else if(!strcmp(command, "STOR")) //TO store client's file into server
			{
				int c = 0, len;
				sscanf(buf+strlen(command), "%s", filename); //Seprating the command and the filename given by client
				recv(sock2, &size, sizeof(int), 0); //Receving the file size from client
				i = 1;
				while(1) 
				{
					filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666); //Creating the file using the permission
					if(filehandle == -1)  
					{
						sprintf(filename + strlen(filename), "%d", i); 
					}
					else
					{
						break;
					}
				}

				char f = (char)malloc(size); //Dynamically allocating file size
				recv(sock2, f, size, 0); //Recieve the file from client
				c = write(filehandle, f, size); //Writing the given file into server
				close(filehandle);
				send(sock2, &c, sizeof(int), 0); //Sending response to client
				printf("Given file has been transered sucessfully \n");
			}

			else if(!strcmp(command, "APPE")) //Appending the file 
			{
				int c = 0, len;
				sscanf(buf+strlen(command), "%s", filename); //Seprating the command and the filename given by client
				recv(sock2, &size, sizeof(int), 0); //Receving the file size from client
				i = 1;
				while(1)
				{
					filehandle = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666); //Opeaning file in Append mode only
					if(filehandle == -1) //No file exists
					{
						sprintf(filename + strlen(filename), "%d", i); 
					}
					else {
						break;
					}
				}
				char f = (char)malloc(size*2); //Dynamically allocating file size
				recv(sock2, f, size, 0); //Recieve the file from client
				c = write(filehandle, f, size); //Writing the given file into server
				close(filehandle);
				send(sock2, &c, sizeof(int), 0); //Sending response to client
				printf("Given files has been Appended sucesfully\n");
			}

			else if(!strcmp(command, "RNTO")) // It renames the file 
			{
				char old_name[100], new_name[100];
				recv(sock2, old_name, 100, 0); //Receiving old name from client
				recv(sock2, new_name, 100, 0); //Receiving new name from client
	
				if (stat(buf, &st) == 1) //Checking if directory is present or not 
				{
					printf("No Directory Found \n");
				}
				else 
				{
					rename(old_name, new_name); //Renaming the file name
				}
				send(sock2, buf, 100, 0); //Sending response to client
				printf("Given Directory name has been renamed Sucessfully \n");
			}

			else if(!strcmp(command, "PWD")) //It gives present working directory
			{
				system("pwd > PWD.txt");
				i = 0;

				FILE*f = fopen("PWD.txt", "r"); //Opening file in read only mode
				while(!feof(f)) //Here, we are looping until eof
				{
					buf[i++] = fgetc(f); //Fetching the present working dir and storing it in buffer 
				}
				buf[i-1] = '\0'; //Adding the NULL value at the end of file 
				fclose(f);
				send(sock2, buf, 100, 0); //Sending response to client
			}

			else if(!strcmp(command, "MKD")) //Making the Directory
			{
				recv(sock2, buf, 100, 0);
				if (stat(buf, &st) == -1) //Checking if Dir is not created
				{
					mkdir(buf, 0777); //Using System function to make dir
					printf("Directory Created %s \n", buf);
				}
				else 
				{
					printf("Unable to create Directory \n");
					exit(1);
				}
				send(sock2, buf, 100, 0); //Sending response to client
				printf("Directory Created Sucessfully \n");
			}

			else if(!strcmp(command, "RMD")) //Removing the Directory
			{
				recv(sock2, buf, 100, 0);
				if (stat(buf, &st) == 1) //Checking if Dir dosen't exist
				{
					printf("Please enter a valid Directory \n");
				}
				else //Removing Dir
				{
					remove(buf);
				}
				send(sock2, buf, 100, 0); //Sending response to client
				printf("Directory has been removed sucessfully \n");
			}

			else if(!strcmp(command, "DELE")) //Both the DELE and RMD works the same way to delete the Dir
			{
				recv(sock2, buf, 100, 0);
				if (stat(buf, &st) == 1)  //Checking if Dir dosen't exist
				{
					printf("Please enter a valid Directory\n");
				}
				else 
				{
					remove(buf);
				}
				send(sock2, buf, 100, 0); //Sending response to client
				printf("Directory has been Deleted sucessfully \n");
			}

			else if(!strcmp(command, "LIST")) //Listing all the dir working as LS
			{
				system("ls > LIST.txt");
				i = 0;
				FILE*f = fopen("LIST.txt","r"); //Opening the File in read only mode
				while(!feof(f)) //Looping until the end of file
				{
					buf[i++] = fgetc(f); //Fetching the data and storing it in array buffer
				}
				buf[i-1] = '\0'; //Adding NULL value at the end of the file 
				fclose(f);
				send(sock2, buf, 1024, 0); //Sending response to client
				printf("All the Directory has been listed \n");
			}

			else if(!strcmp(command, "CWD")) //This is to change the Directory
			{
				if (chdir(buf + 3) == 0) //Function to change to current dir
				{
					c = 1;
				}
				else
				{
					c = 0;
				}
				send(sock2, &c, sizeof(int), 0); //Sending response to client
				printf("The Directory has been changed Sucessfully \n");
			}
			
			else if(!strcmp(command, "QUIT")) //To exit 
			{
				printf("Closing Client\n");
				i = 1;
				send(sock2, &i, sizeof(int), 0); //Signal to close client connection
				break;
			}
			else //Command couldn't be executed
			{
				printf("Client's cannot exit \n");
			}
		}
	}
	return 0;
}