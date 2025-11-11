#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_FILES 1024

char *files[MAX_FILES];
int nfiles = 0;
int selected = 0;
int playing = -1;
pid_t mpg123_pid = 0;
int horiz_offset = 0;
int fg_color = COLOR_WHITE;
int bg_color = COLOR_BLACK;

/* Prototypes */
int cmp_files(const void *a, const void *b);
void list_mp3_files(void);
void stop_current(void);
void play_file(int idx);
void check_playback_ended(void);
void draw_ui(WINDOW *win, int start_row);

int cmp_files(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    while (*sa && *sb) {
        int ca = tolower((unsigned char)*sa);
        int cb = tolower((unsigned char)*sb);
        if (ca != cb) return ca - cb;
        sa++; sb++;
    }
    return tolower((unsigned char)*sa) - tolower((unsigned char)*sb);
}

void list_mp3_files(void) {
    DIR *dir = opendir(".");
    if (!dir) { perror("opendir"); exit(1); }
    struct dirent *ent;
    while ((ent = readdir(dir)) && nfiles < MAX_FILES) {
        if (ent->d_type != DT_REG) continue;
        char *ext = strrchr(ent->d_name, '.');
        if (ext && strcasecmp(ext, ".mp3") == 0) {
            files[nfiles++] = strdup(ent->d_name);
        }
    }
    closedir(dir);
    qsort(files, nfiles, sizeof(char *), cmp_files);
}

void stop_current(void) {
    if (mpg123_pid > 0) {
        kill(mpg123_pid, SIGTERM);
        waitpid(mpg123_pid, NULL, 0);
        mpg123_pid = 0;
    }
    playing = -1;
}

void play_file(int idx) {
    stop_current();
    if (idx < 0 || idx >= nfiles) return;
    pid_t pid = fork();
    if (pid == 0) {
        int devnull = open("/dev/null", O_RDWR);
        if (devnull != -1) {
            dup2(devnull, STDIN_FILENO);
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            if (devnull > 2) close(devnull);
        }
        execlp("mpg123", "mpg123", "-q", "--", files[idx], (char*)NULL);
        _exit(127);
    } else if (pid > 0) {
        mpg123_pid = pid;
        playing = idx;
    }
}

void check_playback_ended(void) {
    if (mpg123_pid <= 0) return;
    int status;
    pid_t r = waitpid(mpg123_pid, &status, WNOHANG);
    if (r == mpg123_pid) {
        mpg123_pid = 0;
        playing = -1;
    }
}

void draw_ui(WINDOW *win, int start_row) {
    werase(win);
    box(win, 0, 0);

    int max_y = getmaxy(win);
    int max_x = getmaxx(win);
    int content_height = max_y - 2;  // Exclude top/bottom border
    int content_width = max_x - 2;   // Exclude left/right border

    // Title
    mvwprintw(win, 0, 2, " %d file%s ", nfiles, nfiles == 1 ? "" : "s");

    const int prefix_len = 3;  // " > " or "   "
    const int name_max_len = content_width - prefix_len;  // Max chars for filename

    for (int i = 0; i < content_height && (i + start_row) < nfiles; i++) {
        int idx = i + start_row;
        const char *name = files[idx];
        int len = strlen(name);

        // Buffer for visible part of filename
        char visible[name_max_len + 1];
        const char *src = name;
        int show_len = name_max_len;

        // Handle horizontal scrolling for selected item
        if (idx == selected && len > name_max_len) {
            int remain = len - horiz_offset;
            show_len = (remain < name_max_len) ? remain : name_max_len;
            src += horiz_offset;
        } else {
            show_len = (len < name_max_len) ? len : name_max_len;
        }

        // Copy and null-terminate
        strncpy(visible, src, show_len);
        visible[show_len] = '\0';

        // Print with prefix
        if (idx == selected) wattron(win, A_REVERSE);
        mvwprintw(win, i + 1, 1, "%s%s", (idx == playing) ? "> " : "  ", visible);
        if (idx == selected) wattroff(win, A_REVERSE);
    }

    wrefresh(win);
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        int f = atoi(argv[1]);
        int b = atoi(argv[2]);
        if (f >= 0 && f <= 7) fg_color = f;
        if (b >= 0 && b <= 7) bg_color = b;
    }

    initscr();
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal doesn't support colors.\n");
        return 1;
    }
    start_color();
    use_default_colors();
    init_pair(1, fg_color, bg_color);
    wbkgd(stdscr, COLOR_PAIR(1) | A_BOLD);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    list_mp3_files();
    if (nfiles == 0) {
        endwin();
        printf("No .mp3 files found in current directory.\n");
        return 1;
    }

    int start_row = 0;
    int visible_lines = getmaxy(stdscr) - 2;

    draw_ui(stdscr, start_row);

    int ch;
    while ((ch = getch()) != 'q') {
        check_playback_ended();
        int redraw = 1;

        // Handle resize
        if (ch == KEY_RESIZE) {
            visible_lines = getmaxy(stdscr) - 2;
            if (selected >= start_row + visible_lines)
                start_row = selected - visible_lines + 1;
            if (start_row < 0) start_row = 0;
        }

        switch (ch) {
            case KEY_UP:
                if (selected > 0) {
                    selected--;
                    horiz_offset = 0;
                    if (selected < start_row) start_row = selected;
                }
                break;

            case KEY_DOWN:
                if (selected < nfiles - 1) {
                    selected++;
                    horiz_offset = 0;
                    if (selected >= start_row + visible_lines)
                        start_row = selected - visible_lines + 1;
                }
                break;

            case KEY_LEFT:
                if (horiz_offset > 0) horiz_offset--;
                else redraw = 0;
                break;

            case KEY_RIGHT:
                {
                    int content_width = getmaxx(stdscr) - 2;
                    int name_max_len = content_width - 3;
                    if (strlen(files[selected]) > name_max_len + horiz_offset)
                        horiz_offset++;
                    else
                        redraw = 0;
                }
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
#ifdef PADENTER
            case PADENTER:
#endif
                horiz_offset = 0;
                if (playing == selected && mpg123_pid > 0) {
                    kill(mpg123_pid, SIGINT);
                } else {
                    play_file(selected);
                }
                break;

            case ' ':
                if (playing == selected && mpg123_pid > 0) {
                    kill(mpg123_pid, SIGINT);
                }
                break;

            case 's':
            case 'S':
                stop_current();
                break;

            case KEY_RESIZE:
                break;

            default:
                redraw = 0;
                break;
        }

        if (redraw) {
            visible_lines = getmaxy(stdscr) - 2;
            draw_ui(stdscr, start_row);
        }
    }

    stop_current();
    for (int i = 0; i < nfiles; i++) free(files[i]);
    endwin();
    return 0;
}
