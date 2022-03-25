#include "header.h"

#define D(...) fprintf(new_stream, __VA_ARGS__)

#define MAXBUF 1024

int main() {
	int sock;
	struct sockaddr_in name;
	char buf[MAX_MSG_LENGTH] = {0};

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) perro("opening socket");

	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(PORT);
	if(bind(sock, (void*) &name, sizeof(name))) perro("binding tcp socket");
	if(listen(sock, 1) == -1) perro("listen");
	
	struct sockaddr cli_addr;
	int cli_len = sizeof(cli_addr);
	int new_socket, new_fd, pid;
	FILE* new_stream;
	
	if(new_fd = dup(STDERR_FILENO) == -1) perro("dup");
	new_stream = fdopen(new_fd, "w");
	setbuf(new_stream, NULL); // sin buffering
	
	D("Initializing server...\n");
	while(new_socket = accept(sock, &cli_addr, &cli_len)) {
		D("Client connected.\nForking... ");
		if(pid = fork()) D("child pid = %d.\n", pid);
		else {
			pid = getpid();
			if(new_socket < 0) perro("accept");
			if(dup2(new_socket, STDOUT_FILENO) == -1) perro("dup2");
			if(dup2(new_socket, STDERR_FILENO) == -1) perro("dup2");

			char username[MAXBUF] = { 0 };
			char passwd[MAXBUF] = { 0 };
			int login_error_cnt = 0;
			char enter_username_msg[MAXBUF] = "Enter username:\n";
			char enter_passwd_msg[MAXBUF] = "Enter password:\n";
			char *correct_username = "admin";
			char *correct_passwd = "123123";
			char *login_error = "Username or password error\n";
			char *login_success = "Login success!\n";

		login:
			send(new_socket, enter_username_msg, strlen(enter_username_msg), 0);
			recv(new_socket, username, MAXBUF, 0);
			send(new_socket, enter_passwd_msg, strlen(enter_passwd_msg), 0);
			recv(new_socket, passwd, MAXBUF, 0);

			if (login_error_cnt > 3) {
				printf("Too many try\n");
				printf("Bye\n");

				// close socket
				close(new_socket);
			}

			// kill(0, SIGSTOP);

			if ((strncmp(username, correct_username, strlen(correct_username)) != 0) || (strncmp(passwd, correct_passwd, strlen(correct_passwd)) != 0)) {
				send(new_socket, login_error, strlen(login_error), 0);
				login_error_cnt++;
				goto login;
			}

			while(1) {
				int readc = 0, filled = 0;
				while(1) {
					readc = recv(new_socket, buf+filled, MAX_MSG_LENGTH-filled-1, 0);
					if(!readc) break;
					filled += readc;
					if(buf[filled-1] == '\0') break;
				}
				if(!readc) {
					D("\t[%d] Client disconnected.\n", pid);
					break;
				}
				D("\t[%d] Command received: %s", pid, buf);
				system(buf);
				D("\t[%d] Finished executing command.\n", pid);
				send(new_socket, "> ", 3, MSG_NOSIGNAL);
			}
			close(new_socket);
			D("\t[%d] Dying.", pid);
			exit(0);
		}
	}
	fclose(new_stream);
	close(sock);
	return 0;
}