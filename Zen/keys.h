#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

typedef struct Row {
	char *text;
	int text_size;
	int is_connected;
	struct Row *next;
	struct Row *prev;
} Content;

char *shift_left(Content *line, int size, int pos,int range);

int shift_right(WINDOW *my_win, Content *head, Content *line, int size, int pos,int y, int ins);

Content *new_line(Content *line,int x, int size);

int shift_tab(Content *line, int size, int range, int pos, const bool reversed);

void display_again(WINDOW *my_win, Content *head, int start_line, int max_lines);	

Content *back_space(Content **head, Content *line, int size, int pos);

void remove_node(Content **head, Content *to_remove);

void insert_missing(Content *line, int pos, int range, char *ins, int size);