#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BLK		4096
#define PORT	1234
#define SA struct sockaddr // DISGUSTING; FIX

int sock_fd;
int connfd;
int has_client = 0;


void *ip_server_setup(void *dump){
	// int connfd;
	struct sockaddr_in servaddr, cli;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	// printf("%d\n",sock_fd );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	if ((bind(sock_fd, (SA * ) & servaddr, sizeof(servaddr))) != 0) {
		perror("ERROR: BIND_FAILURE\n");
		exit(0);
	}	
	if ((listen(sock_fd, 5)) != 0) {
		perror("ERROR: LISTEN_FAILURE\n");
		exit(0);
	}
	int len = sizeof(cli);
	connfd = accept(sock_fd, (SA * ) &cli, &len);
	if (connfd < 0) {
		perror("ERROR: CONNECTION_FAILURE\n");
		exit(0);
	}
	has_client = 1;
	// printf("WOWOWOWWWWWWWWWWWWWWW%d\n", connfd);
	write(connfd,"Connection esatblished\n", 15);
	// send(connfd, "FUNKY\n", 6, 0);
	// write(connfd,"WOWOWOWOWOWOW\n\n", 15);
	// write(connfd,"WOWOWOWOWOWOW\n\n", 15);
	// write(connfd,"WOWOWOWOWOWOW\n\n", 15);
}


typedef struct row_dll{
	char *text;

	int used_len;
	int avlbl_len;
	
	struct row_dll *next;
	struct row_dll *prev;

} row;

int screen_rows = 30;

int num_rows = 0;
row *file_buffer = NULL;


row *top_line = NULL;
row *bot_line = NULL;

row *cur_row;
int cur_y;
int cur_x;



// User file population
row *row_from_line(char *line, int sz);
void populate_file_buffer(char const *filename);

void save_to_disk(char const *filename);
int insert_at(row *r, int index, char c);
void backspace_subroutine(row *r, int index);

void display();

void get_xy();


struct termios orig_termios;



void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
	close(connfd);
}

void enableRawMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disableRawMode);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void cur_up(){
	if((cur_y > 1) && (cur_row != top_line)){
		if((cur_row->prev->used_len-2) < cur_x){
			cur_x = cur_row->prev->used_len-2;
		}
		cur_row = cur_row->prev;
		printf("\x1b[%d;%dH", --cur_y, cur_x); fflush(stdout);
	}
}
void cur_down(){
	if(cur_row->next != NULL){
		if((cur_row->next->used_len-2) < cur_x){
			cur_x = cur_row->next->used_len-2;
		}
		cur_row = cur_row->next;
		printf("\x1b[%d;%dH", ++cur_y, cur_x); fflush(stdout);
	}
}
void cur_left(){
	if(cur_x > 1){
		printf("\x1b[%d;%dH", cur_y, --cur_x); fflush(stdout);
	}
}
void cur_right(){
	if (cur_x < (cur_row->used_len-2)){
		printf("\x1b[%d;%dH", cur_y, ++cur_x); fflush(stdout);
	}
}

int main(int argc, char const *argv[]) {
	
	enableRawMode();
	
	if(argc == 2){
		populate_file_buffer(argv[1]);
	}
	else{
		printf("ERROR:	please specify file name \nusage : $ ./ed filename.txt\n");
		exit(0);
	}

	pthread_t server_thread;

	if(pthread_create(&server_thread, NULL, ip_server_setup, NULL)){
		perror("ERROR: THREAD_LAUNCH_FAILURE\n");
	}

	char c;
	
	display();
	printf("\x1b[1;1H");fflush(stdout);

	// int state = 0;

	while (read(STDIN_FILENO, &c, 1) == 1) {
		if( ((int)c) == 19 ){
			printf("SAVED");fflush(stdout);
			save_to_disk(argv[1]);
		}
		else if(c == 127) {
			backspace_subroutine(cur_row, cur_x-1);
			display();
		}
		else if(c == '\033') {
			char arrow;

			read(STDIN_FILENO, &c, 1);
			read(STDIN_FILENO, &arrow, 1);
// ^[[A
			if(c=='['){
				switch(arrow){
					case 'A':cur_up();break;
					case 'B':cur_down();break;
					case 'C':cur_right();break;
					case 'D':cur_left();
				}
			}
		}
		else {
			insert_at(cur_row, cur_x-1, c);
			// cur_row->text[cur_x-1] = c;
			cur_x++;
			display();
		}
	}
	return 0;
}

void save_to_disk(char const *filename){
	int fd = open(filename, O_WRONLY);
	row *tmp = file_buffer;
	while(tmp){
		dprintf(fd, "%s", tmp->text);
		tmp = tmp->next;
	}
	close(fd);
}

void backspace_subroutine(row *r, int index){
	if(r==NULL){return;}
	if(index==r->avlbl_len-2){
		return;
		// TODO
	}
	int i;
	for(i=index;i<r->used_len-1;i++){
		r->text[i] = r->text[i+1];
	}
	return;
}

int insert_at(row *r, int index, char c){
	if(r==NULL){return -1;}
	if(index > r->used_len){return -1;}
	if( c=='\n' ){
		row *new = malloc(sizeof(row));
		new->avlbl_len = r->avlbl_len;
		
		// DLL pointer jaadu
		new->prev = r;
		new->next = r->next;
		r->next = new;
		if(new->next){new->next->prev = new;}

		new->text = malloc(new->avlbl_len * sizeof(char));
		int copy_len = strlen(r->text+index);
		strcpy(new->text, r->text+index);
		new->text[copy_len] = '\0';
		new->used_len = copy_len+1;
		
		r->text[index] = '\n';
		r->text[index+1] = '\0';
		r->used_len = index+2;

		printf("\x1b[%d;%dH", cur_y, --cur_x); fflush(stdout);

	}
	else{
		if(r->used_len == r->avlbl_len){
			char *tmp = realloc(r->text, r->avlbl_len*2);
			if(tmp == NULL){return -1;}
			r->text = tmp;
			r->avlbl_len *= 2;
		}
		int i;
		for(i=r->used_len;i>index;i--){
			r->text[i] = r->text[i-1];
		}
		r->text[index] = c;
		r->used_len += 1;
	}
	return 0;
}


void get_xy(){
	
	char buf[32];
	unsigned int i = 0;
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return ;
	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';
	
	fflush(stdout);

}

row *row_from_line(char *line, int sz){
	row *new_row = malloc(sizeof(row));

	int aloc_size = BLK * ((sz / BLK) + 1);

	new_row->text = malloc( aloc_size * sizeof(char) );
	new_row->avlbl_len = aloc_size;

	memcpy(new_row->text, line, sz+1);
	new_row->used_len = sz + 1;

	new_row->next = new_row->prev = NULL;

	return new_row;
}

void populate_file_buffer(char const* filename){
	
	file_buffer = NULL;
	row *tmp = NULL;
	
	FILE *user_file_stream = fopen(filename, "r");
	if(user_file_stream==NULL){
		printf("ERROR:	invalid filename\n");
		exit(0);
	}

	char *line_buffer = NULL;
	long unsigned int bytes_to_read = 0; // This value does not mean 0 bytes will be read. Read the getline() man page

	int bytes_read = 0;

	while( (bytes_read = getline(&line_buffer, &bytes_to_read, user_file_stream)) != -1){
		if( file_buffer ){
			tmp->next = row_from_line(line_buffer, bytes_read);
			tmp->next->prev = tmp;
			tmp = tmp->next;
		}
		else{
			file_buffer = row_from_line(line_buffer, bytes_read);
			tmp = file_buffer;
		}
		
		free(line_buffer);
		line_buffer = NULL;
		num_rows++;
	}

	fclose(user_file_stream);
	printf("Populated file_buffer\n");

	top_line = file_buffer;
	cur_row = top_line;
	cur_x = 1;
	cur_y = 1;
}


void display(){
	printf("\e[1;1H\e[2J");
	if(has_client){
		write(connfd, "\e[1;1H\e[2J", 10);
	}
	row *curr = top_line;
	int tbd = screen_rows;
	while(curr && tbd){
		printf("%s",curr->text);
		if(has_client){
			// printf("socket number:%d\n",sock_fd);
			write(connfd, curr->text, strlen(curr->text));
		}
		tbd--;
		curr = curr->next;
		bot_line = curr;
	}
	printf("\x1b[%d;%dH", cur_y, cur_x);
	// if(has_client){
	// 	dprintf(sock_fd"\x1b[%d;%dH", cur_y, cur_x);
	// }
	fflush(stdout);
}

