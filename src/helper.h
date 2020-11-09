typedef enum BG_COLOR { DEF_BG = 40, RED_BG, GREEN_BG, YELLOW_BG, BLUE_BG, PINK_BG, CYAN_BG, WHITE_BG } BG_COLOR;
typedef enum FG_COLOR { BLACK_FG = 30, RED_FG, GREEN_FG, YELLOW_FG, BLUE_FG, PINK_FG, CYAN_FG, DEF_FG } FG_COLOR;

void cprintf(BG_COLOR, FG_COLOR, const char*, ...);
void nprintf(const char*, ...);
const char *_color(int, int, const char*);