#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "keys.h"

int shift_tab(Content *line, int size, int range, int pos, const bool reversed) {
	char *aux = malloc(size+2*sizeof(char));
	memcpy(aux,line->text,size+2);
	if (range > 4) {
			range = 4;
	}
	if (range < 0) {
		return 0;
	}
	
	if (!reversed) {
		int x = pos;
		for (x; x < line->text_size+range; x++) {
			aux[x+range] = line->text[x];
		}
		aux[x+range] = '\0';
		for (int i = 0; i < range; i++) {
			aux[pos++] = ' ';
		}	
	}
	free(line->text);
	line->text = aux;
	return range;
}

void display_again(WINDOW *my_win, Content *head, int start_line, int max_lines) {
	werase(my_win);
	int y = 0, x = 0;
    Content *current = head;
    for (int i = 0;current != NULL && i < start_line; i++) {
        current = current->next;
    }
    while (current != NULL && y < max_lines) {
        mvwprintw(my_win, y++, x, "%s", current->text);
        current = current->next;
    }
    wrefresh(my_win);
}

int shift_right(WINDOW *my_win, Content *head, Content *line, int size, int pos, int y, int ins) {
	if (line != head && line->prev->text_size == size) {
		line->prev->is_connected = 1;
	}
	if (line->text_size < size - 1) {
		char *aux = calloc(size+2,sizeof(char));
		memcpy(aux,line->text,size+2);
		for (int i = pos; i <= line->text_size; i++) {
			aux[i+1] = line->text[i]; 
		}
		aux[pos] = ins;
		line->text = aux;
		line->text_size++;
		mvwprintw(my_win, y, 0, "%s", line->text);

		
	}  
	else {
		int flag = 1;
		int flag1 = 0;
		if (line->next == NULL) {
			Content *keep = (Content*)malloc(sizeof(Content));
			keep->text = calloc(size+2,sizeof(char));
			keep->text[0] = '\0';
			keep->text_size = 0;
			keep->next = NULL;
			keep->prev = line;
			line->next = keep;
			flag = 0;
		}
		if (line->text_size == size) {
			flag1 = 1;
		}
		char *aux = calloc(size+2,sizeof(char));
		aux = strdup(line->text);
		int ch = line->text[size-1];
		for (int i = pos; i < line->text_size; i++) {
			aux[i+1] = line->text[i]; 
		}
		aux[pos] = ins;
		aux[size] = '\0';
		line->text = aux;
		line->text_size = size;
		mvwprintw(my_win, y++, 0, "%s", line->text);
		if (flag && flag1) {
			return shift_right(my_win, head,line->next, size, 0, y, ch);	
		}
	}	
	return -1;
}

void insert_missing(Content *line, int pos, int range, char *ins, int size) {
	if (pos == size) {
		line->text[pos-1] = ins[0];
	}
	else { 
		for (int i = 0; i < range; i++) {
			line->text[pos+i] = ins[i];
		}
	}
	if (ins[0] != '\0') {
		if (line->text_size != size) {
			line->text_size += range;
		}
		line->text[line->text_size] = '\0';
	}
	else {
		line->text_size--;
	}
	
}

char *shift_left(Content *line, int size, int pos, int range) {
	char *arr = calloc(range+1,sizeof(char));
	int i = 0;
	if (line->text_size == 0) {
		arr[0] = '\0';
		return arr;
	}
	
	for (i = 0; i < line->text_size; i++) {
		if (i < range) {
			arr[i] = line->text[i];
		}
		line->text[i] = line->text[i+range]; 
	}
	line->text_size -= range;
	line->text[line->text_size] = '\0';
	arr[range] = '\0';
	return arr;
}
void remove_node(Content **head, Content *to_remove) {
    if (*head == NULL || to_remove == NULL) {
        return;
    }
    if (*head == to_remove) {
        *head = to_remove->next;
    }
    if (to_remove->prev != NULL) {
        to_remove->prev->next = to_remove->next;
    }
    if (to_remove->next != NULL) {
        to_remove->next->prev = to_remove->prev;
    }
    free(to_remove);
}



Content *back_space(Content **head, Content *line, int size, int pos) {
	Content *keep = line;
	if (!line->is_connected) {
		if (pos > 0) {
			for (pos; pos < line->text_size; pos++) {
				line->text[pos-1] = line->text[pos]; 
			}
			line->text[--line->text_size] = '\0';
		}	
		else {
			if (line->prev != NULL) {
				keep = keep->prev;
				int range = 0;
				if (line->prev->text_size == size) {
					range = 1;
				}
				else {
					range = size + 1 - line->prev->text_size;
					if (range > line->text_size) {
						range = line->text_size;
					}
				}
				char *to_insert =  shift_left(line,size,0,range);
				insert_missing(line->prev,line->prev->text_size,range,to_insert, size);
				free(to_insert);
				if (line->text_size == 0) {
					if (line->prev != NULL && line->prev->is_connected) {
						line->prev->is_connected = 0;
					}
					if (line->prev != NULL && line->next != NULL) {
							remove_node(head,line);
					}
				}
			}
		}
	}
	else {
		if (pos > 0) {
			for (pos; pos < line->text_size; pos++) {
				line->text[pos-1] = line->text[pos]; 
			}
			line->text[line->text_size] = '\0';
			line = line->next;
			while (line->is_connected) {
				char *to_insert =  shift_left(line,size,0,1);
				insert_missing(line->prev,line->prev->text_size,1,to_insert, size);
				free(to_insert);	
				line = line->next;
			}
			char *to_insert =  shift_left(line,size,0,1);
			insert_missing(line->prev,line->prev->text_size,1,to_insert, size);
			free(to_insert);
			if (line->text_size == 0) {
				if (line->prev != NULL && line->prev->is_connected) {
					line->prev->is_connected = 0;
				}
				if (line->prev != NULL && line->next != NULL) {
					remove_node(head,line);
				}
			}
		}
		else if (line->prev != NULL && pos == 0) {
			keep = keep->prev;
			while (line->is_connected) {
				int range = 0;
				if (line->prev->text_size == size) {
					range = 1;
				}
				else {
					range = size + 1 - line->prev->text_size;
					if (range > line->text_size) {
						range = line->text_size;
					}
				}
				char *to_insert =  shift_left(line,size,0,range);
				insert_missing(line->prev,line->prev->text_size,range,to_insert, size);
				free(to_insert);
				line = line->next;
			}
			int range = 0;
			if (line->prev->text_size == size) {
				range = 1;
			}
			else {
				range = size + 1 - line->prev->text_size;
				if (range > line->text_size) {
					range = line->text_size;
				}
			}
			char *to_insert =  shift_left(line,size,0,range);
			insert_missing(line->prev,line->prev->text_size,range,to_insert, size);
			free(to_insert);
			if (line->prev != NULL && line->text_size == 0) {
				line->prev->is_connected = 0;
				if (line->prev != NULL && line->next != NULL) {
					remove_node(head,line);
				}
			}
		}
	}	
	return keep;
} 

