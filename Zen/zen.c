#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <menu.h>
#include "keys.h"
#define ESC     27
#define ENTER  '\n'
#define CTRL_Q  17
#define TAB 9
#define TRUE 1
#define False 0

WINDOW *create_newwin(int height, int width, int starty, int startx,int ls, int rs, int ts, int bs, int tl, int tr,int bl, int br);

char *tell_extension(char *file_name);

void init_all_colors(void);

void process_input(WINDOW *my_win, WINDOW *top, WINDOW *bottom, char *filename, Content *head);

void display_file_content(WINDOW *my_win, Content *head);

void load_file(FILE *fp, char *extension, WINDOW *text_win, WINDOW *top, WINDOW *bottom, char *filename);

Content *new_line(Content *line,int x, int size);

void free_content(Content *head);

int main(int argc, char* argv[]) { 
    WINDOW *insert_win;
    WINDOW *menu_win;  
    WINDOW *default_win;
    WINDOW *lines_win;
    FILE *fp;
    int startx, starty, width, height, flag, ch;
    char* filename = "None";
    initscr();
    init_all_colors();
    raw();
    noecho();
    menu_win = create_newwin(3, COLS, 0, 0, ' ',' ',' ',' ',' ',' ',' ',' ');
    insert_win = create_newwin(LINES-5,COLS, 3, 0, ' ',' ',' ',' ',' ',' ',' ',' ');
    default_win = create_newwin(2,COLS,LINES-2,0,' ',' ',' ',' ',' ',' ',' ',' ');
    wbkgd(menu_win,COLOR_PAIR(3));
    wbkgd(insert_win,COLOR_PAIR(1)); 
    wbkgd(default_win,COLOR_PAIR(4));
    wrefresh(menu_win);
    wrefresh(insert_win);
    wrefresh(default_win);
    
    if (argc == 2) {
        filename = argv[1];
        fp = fopen(filename,"r+");
        if (fp == NULL) {
            fp = fopen(filename, "w+");
            if (fp == NULL) {
                perror("Error creating file");
                return 1;
            }
        }
        
    }
    else {
        fp = fopen(filename, "w+");
        if (fp == NULL) {
                perror("Error creating file");
                return 1;
        }
    }
    char *extension = tell_extension(filename);
    load_file(fp,extension,insert_win,menu_win, default_win, filename);    
    endwin();
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx, int ls, int rs, int ts, int bs, int tl, int tr,int bl, int br) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    refresh();
    wborder(local_win,ls,rs,ts,bs,tl,tr,bl,br);
    wrefresh(local_win);
    return local_win;
}

void destroy_win(WINDOW *local_win) {
    wborder(local_win,' ',' ',' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}

// find extension so i can make my syntax highlight
char *tell_extension(char *file_name) {
    int fst = 0, i = 0, buffer = 100;
    char *extension = (char*)malloc(buffer * sizeof(char));
    while (fst < strlen(file_name)) {
        if (file_name[fst] == '.') {
            fst++;
            break;
        }
        fst++;
    }
    while (fst < strlen(file_name)) {
        extension[i++] = file_name[fst++];
        if (i == buffer-1) {
            buffer *= 2;
            char *aux = (char*)realloc(extension, buffer * sizeof(char));
            if (aux == NULL) {
                free(extension);
                return NULL;
            }
            extension = aux;
        }
    }
    extension[i] = '\0'; 
    return extension;
}

void init_all_colors(void) {
    start_color();
    init_color(1,147,144,201);    // 800,800,900 roxo bebe
    init_color(2,800,850,800);
    init_pair(1,COLOR_WHITE,1); // background
    init_pair(2,COLOR_CYAN,1); // comments
    init_pair(3,COLOR_BLACK,2);
    init_pair(4,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(5,COLOR_MAGENTA,1);
    
}

void load_file(FILE *fp, char *extension, WINDOW *text_win, WINDOW *top, WINDOW *bottom, char *filename) { 
    int maxY, maxX;
    getmaxyx(text_win,maxY,maxX);
    Content *head = NULL, *tail = NULL;
    int flag = 1;
    int tab = 0;
    int connected = 0;
    char *buffer;
    while (flag) {
        buffer = calloc(maxX+2,sizeof(char));
        int i = 0, ch;
        if (connected) {
            buffer[0] = ch;
            i = 1;
        }
        connected = 0;
        if (tab) {
            buffer[0] = ' ';
            buffer[1] = ' '; 
            buffer[2] = ' ';
            buffer[3] = ' ';
            i = 4;
        }
        tab = 0;
        while (ch = fgetc(fp)) {
            if (ch == EOF) {
                flag = 0;
                break;
            }
            if (ch == '\n' || ch == '\0' || i == maxX) {
                break;
            }
            if (ch == '\t') {
                if (i <= maxX-4) {
                    buffer[i++] = ' ';
                    buffer[i++] = ' '; 
                    buffer[i++] = ' ';
                    buffer[i++] = ' ';
                }
                else {
                    tab = 1;
                    break;
                }
            }
            else {
                buffer[i++] = ch;
            }
        }
        Content *new_row = (Content*)(malloc(sizeof(Content)));
        if (i == maxX && ch != '\n' && ch != '\0') {
            connected = 1;
            new_row->is_connected = 1;
        }
        buffer[i] = '\0';
        new_row->text = strdup(buffer);
        int range = 0;
        new_row->text_size = i;
        new_row->text_size += range;
        new_row->next = NULL;
        new_row->prev = tail;
        if (!head) {
            head = new_row;
        } 
        else {
            tail->next = new_row;
        }
        tail = new_row; 
    }
    free(buffer);
    fclose(fp);
    display_file_content(text_win,head);
    process_input(text_win, top, bottom, filename, head);    
}

void display_file_content(WINDOW *my_win, Content *head) {
    int y = 0, x = 0, maxY, maxX;
    getmaxyx(my_win,maxY,maxX);
    Content *current = head;
    while (current != NULL && y < maxY) {
        mvwprintw(my_win, y++, x, "%s", current->text);
        current = current->next;
    }
    wmove(my_win, 0, 0);
    wrefresh(my_win);
} 

Content *new_line(Content *line, int x, int size) {
    Content *keep = (Content*)malloc(sizeof(Content));
    keep->text = calloc(size+1,sizeof(char));
    int fst = 0;
    int i = 0;
    if (line != NULL) {
        for (i; i < size+1; i++) {
            if (line->text[i] == '\0') {
                keep->text[fst]  = '\0';
                break;
            }
            if (i == x) {
                keep->text_size =  line->text_size - i;
                line->text_size = i;
            }
            if (i >= x) {
                keep->text[fst++] = line->text[i];
            }
        }
    }
    else {
        keep->text[0] = '\0';
        keep->text_size = 0;
    }

    keep->next = NULL;
    keep->prev = line;
    return keep;
}    

void free_content(Content *head) {
    Content *current = head;
    while (current != NULL) {
        Content *next = current->next;
        free(current->text);
        free(current);
        current = next;
    }
}


// enable user to write into his file
typedef enum {
    EDIT,
    DEFAULT
} Mode;


void process_input(WINDOW *my_win, WINDOW *top, WINDOW *bottom, char *filename, Content *head) {
    int y = 0, x = 0, maxY, maxX, ch, start_line = 0, end_line;    
    getmaxyx(my_win,maxY,maxX);
    end_line = maxY;
    Content *line = head;
    Mode mode = EDIT;
    keypad(my_win, TRUE); 
    scrollok(my_win, TRUE);
    if (!line) {
        line = new_line(NULL,x, maxX);
        head = line;
    }
    while (1) {
        if (line->text_size < x) {
            line->text_size = x;
        }
        wmove(my_win,y,x);
        ch = wgetch(my_win);
        switch (ch) {
            case KEY_RESIZE: {
                //supdate_content();
                getmaxyx(my_win,maxY,maxX);
                end_line = maxY-1;
            }
            case ESC: {
                //mode = DEFAULT;
                break;
	    }
            case KEY_RIGHT: {
                if (x < line->text_size && x != maxX - 1) {
                    x++;
                }
                break;
            }
            case KEY_LEFT: { 
                if (x > 0) {
                    x--;
                }
                break;
            }
            case KEY_UP: {
                if (line->prev != NULL) {
                    line = line->prev;
                    if (y == 0) { 
                        if (start_line > 0) {
                            --start_line;
                            --end_line;
                            wscrl(my_win, -1);
                            wrefresh(my_win);
                            mvwprintw(my_win, 0, 0, "%s", line->text); 
                        }
                    } else {
                        --y; 
                    }
                    if (x > line->text_size) {
                        x = line->text_size;
                    }
                }
                break;
            }

            case KEY_DOWN: {
                if (line->next == NULL) {
                    if (line->text_size) {
                        Content *keep = (Content*)malloc(sizeof(Content));
                        keep->text = calloc(maxX+2,sizeof(char));
                        keep->text[0] = '\0';
                        keep->text_size = 0;
                        keep->next = NULL;
                        keep->prev = line;
                        line->next = keep;
                    }
                    else {
                        continue;
                    }
                }

                if (y == maxY-1) {    
                    ++end_line;
                    ++start_line;
                    wscrl(my_win, 1);
                    wrefresh(my_win);
                    mvwprintw(my_win, y, 0, "%s", line->next->text);
                }
                if (x > line->next->text_size) {
                    x = line->next->text_size;
                }
                if (y < maxY-1) {
                    y++;
                }
                line = line->next;
                break;
            }
            case KEY_BACKSPACE: { // o problema esta quando faço backspace e a linha esta toda cheiaa
                int keep_pos = 0, flag = 0;
                if (x == 0 && line == head) {
                    break;
                }
                if (line != head) {
                    keep_pos = line->prev->text_size;
                    flag = 1;
                }
                line = back_space(&head , line, maxX, x);
                if (x == 0) {    
                    if (flag) {
                        x = keep_pos;
                    }
                    if (start_line > 0 && y == 0) {
                        --start_line;
                        --end_line;
                    }
                    if (x == maxX) {
                        x--;
                    }
                    if (y > 0) {
                        y--;
                    }
                    
                }
                else {
                    x--;    
                }
                display_again(my_win,head,start_line,end_line);
                break;
            }
            case ENTER: {
                Content *new_row = new_line(line, x, maxX);
                line->is_connected = 0;
                if (x == 0 && line->prev) {
                    line->prev->is_connected = 0;
                }
                if (line->next != NULL) {
                    new_row->next = line->next;
                    line->next->prev = new_row;
                }
                int i = x;
                line->text[i] = '\0';
                if (y == maxY-1) {
                    line->next = new_row;
                    line = new_row;
                    x = 0;
                    ++end_line;
                    ++start_line;        
                    display_again(my_win,head,start_line,end_line);    
                }
                else {
                    y++;
                    line->next = new_row;
                    line = new_row;
                    x = 0;
                    display_again(my_win,head,start_line,end_line);    
                }
                break;
            }
            case CTRL_Q: {
                scrollok(my_win, FALSE);
                FILE *fp = fopen(filename,"w+");
                Content *curr = head;
                while (curr != NULL) {
                    fprintf(fp, "%s", curr->text);
                    curr = curr->next;
                    if (curr && !curr->next && !curr->text_size) {
                        break;
                    }
                    if (curr != NULL && !curr->prev->is_connected) {
                        fputc('\n',fp);
                    }
                }                
                fclose(fp);
                   
                return;
            }
            case TAB: {  
                int range = maxX - 1 - line->text_size;
                int shift_range = shift_tab(line, maxX, range, x, FALSE);    
                x += shift_range;
                mvwprintw(my_win, y, 0, "%s", line->text);
                line->text_size += shift_range;
                break;
            }
            case KEY_BTAB: {
                int range = maxX-1- line->text_size;
                int shift_range = shift_tab(line, maxX, range, x, TRUE);
                x -= shift_range;
                mvwprintw(my_win, y, 0, "%s", line->text);
                line->text_size -= shift_range;
                break;
            }
            default: {
                if (mode == DEFAULT) {
                    if (ch == 'i') {
                        mode = EDIT;
                        break;
                    }
                    else {
                        //default_mode();
                        break;
                    }
                }
                if (mode == EDIT) {
                    shift_right(my_win, head, line, maxX, x, y, ch);
                    if (x == maxX-1) {
                        y++;
                        line = line->next;
                        x = 0;
                    }    
                    else {
                        x++;
                    }
                                
                }
            }
        }
        wrefresh(my_win);
    } 
}


// keep track of all moves to be able to make an 'undo' action later
typedef struct undo {
    char op; // i -> insert | d -> delete | c -> change Attribute
    char *data; // char
    char data_size;
    int starty, startx, endy, endx; // position
    struct undo *next; // next move
} UndoNode;

// F1 or ctrl q -> quit | ctrl c -> copy | ctrl v -> copy | ctrl m -> move to line | ctrl
