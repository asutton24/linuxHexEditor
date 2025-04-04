#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char byte;

typedef struct mem{
	byte* main_mem;
	byte* overflow_mem;
	int main_size;
	int overflow_size;
	int mem_len;
} Memory;

typedef struct termset{
	int low_byte;
	int cur_byte;
	char cur_nibble;
	int bytes_per_row;
} Settings;

int edit_mem(Memory* m, int pos, byte val){
	if (pos < 0 || pos >= m->mem_len) return -1;
	if (pos < m->main_size){
		m->main_mem[pos] = val;
		return 0;
	}
	if (pos - m->main_size < m->overflow_size){
		m->overflow_mem[pos - m->main_size] = val;
		return 0;
	}
	return -1;
}

int fetch_mem(Memory* m, int pos){
	if (pos < 0 || pos >= m->mem_len) return -1;
	if (pos < m->main_size){
		return m->main_mem[pos];
	}
	if (pos - m->main_size < m->overflow_size){
		return m->overflow_mem[pos - m->main_size];
	}
	return -1;
}

int insert_in_mem(Memory* m, int pos, byte val){
	if (pos < 0 || pos > m->mem_len) return -1;
	m->mem_len++;
	if (m->mem_len > m->main_size + m->overflow_size){
		m->overflow_mem = realloc(m->overflow_mem, m->overflow_size + 1024);
		if (m->overflow_mem == NULL) return -1;
		m->overflow_size += 1024;
	}
	for (int i = m->mem_len - 1; i > pos; i--){
		if (edit_mem(m, i, fetch_mem(m, i - 1)) == -1) return -1; 
	}
	if (edit_mem(m, pos, val) == -1) return -1;
	return 0;
}

int delete_from_mem(Memory* m, int pos){
	if (pos < 0 || pos >= m->mem_len) return -1;
	m->mem_len--;
	for (int i = pos; i < m->mem_len; i++){
		if (edit_mem(m, i, fetch_mem(m, i + 1)) == -1) return -1;
	}
	return 0;
}

int printFullScreen(char* header, Settings* s, Memory* m){
	int rows, cols;
	clear();
	getmaxyx(stdscr, rows, cols);
	attron(A_REVERSE);
	printw("%s", header);
	for (int i = 0; i < cols - strlen(header); i++){
		printw(" ");
	}
	attroff(A_REVERSE);
	for (int i = s->low_byte; i < s->bytes_per_row * (cols - 1) || i < m->mem_len; i++){
		if ((i - s->low_byte) % s->bytes_per_row == 0){
			printw("%.08X: ", i);
		}
		if (i == s->cur_byte){
			if (s->cur_nibble == 'h') attron(A_REVERSE);
			printw("%X", fetch_mem(m, pos) / 16);
			attroff(A_REVERSE);
			if (s->cur_nibble == 'l') attron(A_REVERSE);
			printw("%X", fetch_mem(m, pos) % 16);
			attroff(A_REVERSE);
			printw(" ");
		}
	}
	return 0;
}

int run_editor(byte* data, int org_len, char* file_name){
	Memory* file = (Memory*)malloc(sizeof(Memory));
	file->main_mem = data;
	file->main_size = org_len;
	file->overflow_mem = (byte*)malloc(1024);
	file->overflow_size = 1024;
	file->mem_len = org_len;
	file->overflow_mem[0] = 0;
	initscr();
	raw();
	noecho();
}

int main(){
	initscr();
	cbreak();
	noecho();
	scrollok(stdscr, TRUE);
	curs_set(0);
	printFullScreen("test", NULL);
	getch();
	endwin();
	return 0;
}
