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
	move(0,0);
	getmaxyx(stdscr, rows, cols);
	attron(A_REVERSE);
	printw("%s", header);
	for (int i = 0; i < cols - strlen(header); i++){
		printw(" ");
	}
	attroff(A_REVERSE);
	for (int i = s->low_byte; (i < s->bytes_per_row * (cols - 2)) && (i < m->mem_len); i++){
		if ((i - s->low_byte) % s->bytes_per_row == 0){
			printw("%.08X: ", i);
		}
		if (i == s->cur_byte){
			if (s->cur_nibble == 'h') attron(A_REVERSE);
			printw("%X", fetch_mem(m, i) / 16);
			attroff(A_REVERSE);
			if (s->cur_nibble == 'l') attron(A_REVERSE);
			printw("%X", fetch_mem(m, i) % 16);
			attroff(A_REVERSE);
			printw(" ");
		} else {
			printw("%.02X ", fetch_mem(m, i));
		}
		if ((i - s->low_byte) % s->bytes_per_row == s->bytes_per_row - 1){
			printw("\n");
		}
	}
	return 0;
}

int run_editor(byte* data, int org_len, char* file_name){
	int running = 1;
	int input;
	int insert_val;
	int cur_val;
	int rows, cols;
	Memory* file = (Memory*)malloc(sizeof(Memory));
	file->main_mem = data;
	file->main_size = org_len;
	file->overflow_mem = (byte*)malloc(1024);
	file->overflow_size = 1024;
	file->mem_len = org_len;
	file->overflow_mem[0] = 0;
	Settings* set = (Settings*)malloc(sizeof(Settings));
	set->low_byte = 0;
	set->cur_byte = 0;
	set->cur_nibble = 'h';
	set->bytes_per_row = 16;
	initscr();
	raw();
	noecho();
	scrollok(stdscr, FALSE);
	curs_set(0);
	printFullScreen(file_name, set, file);
	while (running){
		input = getch();
		if (('9' >= input && '0' <= input) || ('F' >= input && 'A' <= input) || ('f' >= input && 'a' <= input)){
			if ('9' >= input && '0' <= input){
				insert_val = input - '0';
			} else if ('F' >= input && 'A' <= input){
				insert_val = input - 'A' + 10;
			} else {
				insert_val = input - 'a' + 10;
			}
			cur_val = fetch_mem(file, set->cur_byte);
			if (set->cur_nibble == 'h'){
				cur_val = cur_val % 16;
				cur_val += insert_val << 4;
				edit_mem(file, set->cur_byte, cur_val);
				set->cur_nibble = 'l';
			} else if (set->cur_nibble == 'l'){
				cur_val = cur_val & 0xF0;
				cur_val += insert_val;
				edit_mem(file, set->cur_byte, cur_val);
				set->cur_nibble = 'h';
				set->cur_byte += 1;
				if (set->cur_byte == file->mem_len){
					insert_in_mem(file, set->cur_byte, 0);
				}
				getmaxyx(stdscr, rows, cols);
				if (set->cur_byte / set->bytes_per_row - set->low_byte / set->bytes_per_row >= cols){
					set->low_byte += set->bytes_per_row;
				}
			}
		} else if (input == 'q'){
			running = 0;
		}
		printFullScreen(file_name, set, file);
	}
	endwin();
	free(file->main_mem);
	free(file->overflow_mem);
	free(file);
	free(set);
	return 0;
}
