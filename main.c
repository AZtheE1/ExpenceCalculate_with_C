#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define N 50
#define M 30

/* ── ANSI escape helpers ──────────────────────────────────── */
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"

#define FG_WHITE    "\033[97m"
#define FG_GREEN    "\033[92m"
#define FG_RED      "\033[91m"
#define FG_YELLOW   "\033[93m"
#define FG_CYAN     "\033[96m"
#define FG_BLUE     "\033[94m"
#define FG_GRAY     "\033[90m"
#define FG_BLACK    "\033[30m"

#define BG_GREEN    "\033[42m"
#define BG_RED      "\033[41m"
#define BG_BLUE     "\033[44m"
#define BG_BLACK    "\033[40m"
#define BG_YELLOW   "\033[43m"

#define CLEAR       "\033[2J\033[H"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

/* Box-drawing */
#define TL "╔"
#define TR "╗"
#define BL "╚"
#define BR "╝"
#define HL "═"
#define VL "║"
#define ML "╠"
#define MR "╣"

#define BOX_W 58   /* inner width */

double currentincome  = 0;
double currentexpense = 0;

typedef struct node {
    char   date[M];
    double amount;
    char   category[N];
    struct node *next;
} Node;

typedef struct { double x, y; } Record;

Node *income  = NULL;
Node *expense = NULL;

/* ── drawing primitives ───────────────────────────────────── */

static void rep(const char *s, int n) { for (int i=0; i<n; i++) fputs(s, stdout); }

static void h_rule(const char *l, const char *r)
{
    printf(FG_CYAN "%s", l); rep(HL, BOX_W); printf("%s\n" RESET, r);
}

static void blank_row(void)
{
    printf(FG_CYAN VL RESET "%*s" FG_CYAN VL "\n" RESET, BOX_W, "");
}

static void centre_row(const char *colour, const char *text)
{
    int len = (int)strlen(text);
    /* Simple adjustment for UTF-8 symbols in the "BUDGET TRACKER" header */
    /* This calculation is slightly off but good enough for centering */
    int pad = (BOX_W - (len > BOX_W ? BOX_W : len)) / 2;
    printf(FG_CYAN VL RESET "%*s%s%s" RESET "%*s" FG_CYAN VL "\n" RESET,
           pad, "", colour, text, BOX_W - pad - len, "");
}

/* ── header + summary ─────────────────────────────────────── */

static void draw_header(void)
{
    printf(CLEAR);
    h_rule(TL, TR);
    blank_row();
    centre_row(BOLD FG_YELLOW, "৳ BUDGET TRACKER");
    centre_row(DIM FG_WHITE,   "Personal Finance Manager");
    blank_row();
    h_rule(ML, MR);
}

static void draw_summary(void)
{
    double bal = currentincome - currentexpense;
    char buf[64];

    blank_row();
    snprintf(buf, sizeof(buf), "INCOME    +  ৳ %.2f", currentincome);
    centre_row(FG_GREEN BOLD, buf);
    snprintf(buf, sizeof(buf), "EXPENSE   -  ৳ %.2f", currentexpense);
    centre_row(FG_RED BOLD, buf);

    printf(FG_CYAN VL FG_GRAY); rep("─", BOX_W); printf(FG_CYAN VL "\n" RESET);

    snprintf(buf, sizeof(buf), "BALANCE      ৳ %.2f", bal);
    centre_row(bal >= 0 ? FG_CYAN BOLD : FG_RED BOLD, buf);
    blank_row();
    h_rule(ML, MR);
}

/* ── menu ─────────────────────────────────────────────────── */

static void draw_menu(void)
{
    blank_row();

    typedef struct { const char *badge; const char *label; const char *lc; } Item;
    Item items[] = {
        {BG_BLUE  FG_WHITE BOLD " 1 " RESET, "  Insert Income",        FG_BLUE},
        {BG_RED   FG_WHITE BOLD " 2 " RESET, "  Insert Expense",       FG_RED},
        {BG_GREEN FG_BLACK BOLD " 3 " RESET, "  View Income Records",  FG_GREEN},
        {BG_YELLOW FG_BLACK BOLD" 4 " RESET, "  View Expense Records", FG_YELLOW},
        {BG_BLACK FG_WHITE BOLD " 5 " RESET, "  Exit",                 FG_GRAY},
    };
    for (int i = 0; i < 5; i++) {
        printf(FG_CYAN VL RESET "  %s %s%-30s" RESET "%*s" FG_CYAN VL "\n" RESET,
               items[i].badge, items[i].lc, items[i].label,
               BOX_W - 38, "");
    }

    blank_row();
    h_rule(ML, MR);
    blank_row();
    centre_row(DIM FG_WHITE, "Enter your choice and press ENTER");
    blank_row();
    h_rule(BL, BR);
    printf("\n  " BOLD FG_CYAN "»» " RESET);
}

/* ── section box ──────────────────────────────────────────── */

static void sec_open(const char *title, const char *col)
{
    printf("\n" FG_CYAN "  " TL); rep(HL, BOX_W-4); printf(TR "\n" RESET);
    printf(FG_CYAN "  " VL RESET "  %s%s%s" RESET, BOLD, col, title);
    printf("%*s" FG_CYAN VL "\n" RESET, (int)(BOX_W-5-(int)strlen(title)), "");
    printf(FG_CYAN "  " ML); rep(HL, BOX_W-4); printf(MR "\n" RESET);
}

static void sec_close(void)
{
    printf(FG_CYAN "  " BL); rep(HL, BOX_W-4); printf(BR "\n\n" RESET);
}

/* ── status pills ─────────────────────────────────────────── */

static void msg_ok (const char *t){printf("\n  "BG_GREEN FG_BLACK BOLD"  OK  "RESET"  "FG_GREEN"%s"RESET"\n\n",t);}
static void msg_err(const char *t){printf("\n  "BG_RED   FG_WHITE BOLD" ERR  "RESET"  "FG_RED  "%s"RESET"\n\n",t);}
static void msg_inf(const char *t){printf("\n  "BG_BLUE  FG_WHITE BOLD" INFO "RESET"  "FG_CYAN "%s"RESET"\n\n",t);}

static void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void wait_enter(void)
{
    printf("  " DIM FG_GRAY "Press ENTER to return to menu" RESET "  ");
    clear_input();
    getchar();
}

/* ── linked-list & file helpers ─────────────────────────── */

static void list_append(Node **h, Node **t,
                         const char *date, double amount, const char *cat)
{
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n){ fputs("Memory Error\n", stderr); exit(1); }
    strncpy(n->date,     date, M-1); n->date[M-1]     = '\0';
    strncpy(n->category, cat,  N-1); n->category[N-1] = '\0';
    n->amount = amount; n->next = NULL;
    if (!*h){ *h=*t=n; } else { (*t)->next=n; *t=n; }
}

static void list_free(Node *h){ while(h){ Node *x=h->next; free(h); h=x; } }

static int list_write(Node *h, const char *fn)
{
    FILE *fp = fopen(fn,"wb"); if(!fp) return 0;
    for(Node *p=h; p; p=p->next){ Node tmp=*p; tmp.next=NULL; fwrite(&tmp,sizeof(Node),1,fp); }
    fclose(fp); return 1;
}

static Node *list_read(const char *fn, Node **tout)
{
    FILE *fp = fopen(fn,"rb"); if(!fp) return NULL;
    fseek(fp,0,SEEK_END);
    long size = ftell(fp);
    int entries=(int)(size/sizeof(Node)); rewind(fp);
    Node *h=NULL,*t=NULL;
    for(int i=0;i<entries;i++){
        Node *n=(Node *)malloc(sizeof(Node)); if(!n){ fclose(fp); exit(1); }
        fread(n,sizeof(Node),1,fp); n->next=NULL;
        if(!h){h=t=n;} else{t->next=n;t=n;}
    }
    fclose(fp); if(tout)*tout=t; return h;
}

static void record_write(double inc, double exp)
{ FILE*fp=fopen("Record.bin","wb"); if(!fp)return; Record r={inc,exp}; fwrite(&r,sizeof(r),1,fp); fclose(fp); }

static int record_read(double *inc, double *exp)
{ FILE*fp=fopen("Record.bin","rb"); if(!fp)return 0; Record r; int ok=(fread(&r,sizeof(r),1,fp)==1); fclose(fp); if(ok){*inc=r.x;*exp=r.y;} return ok; }

/* ── screens ──────────────────────────────────────────────── */

static void screen_input(int is_income,
                          Node **head, Node **tail,
                          double *total, const char *fn)
{
    draw_header();
    sec_open(is_income ? "ADD INCOME" : "ADD EXPENSE",
             is_income ? FG_GREEN : FG_RED);

    char s1[9],s2[9],s3[9],date[M],cat[N]; double amt;

    printf(FG_CYAN "  " VL RESET "\n");
    printf(FG_CYAN "  " VL RESET "  " BOLD "Date " DIM "(day  month  year)" RESET ":  ");
    if(scanf("%8s %8s %8s",s1,s2,s3)!=3){
        msg_err("Invalid date."); sec_close(); wait_enter(); return; }
    snprintf(date,M-1,"%s %s %s",s1,s2,s3); date[M-1]='\0';

    printf(FG_CYAN "  " VL RESET "\n");
    printf(FG_CYAN "  " VL RESET "  " BOLD "Amount " DIM "(৳)" RESET ":  ");
    if(scanf("%lf",&amt)!=1||amt<=0){
        msg_err("Amount must be a positive number."); sec_close(); wait_enter(); return; }

    printf(FG_CYAN "  " VL RESET "\n");
    printf(FG_CYAN "  " VL RESET "  " BOLD "Category" RESET ":  ");
    if(scanf("%49s",cat)!=1){
        msg_err("Invalid category."); sec_close(); wait_enter(); return; }

    printf(FG_CYAN "  " VL RESET "\n");
    sec_close();

    *total += amt;
    list_append(head, tail, date, amt, cat);

    if(list_write(*head, fn)) msg_ok(is_income?"Income saved!":"Expense saved!");
    else                      msg_err("Could not write to file.");
    wait_enter();
}

static void screen_records(Node *head, int is_income)
{
    draw_header();
    sec_open(is_income ? "INCOME RECORDS" : "EXPENSE RECORDS",
             is_income ? FG_GREEN : FG_RED);

    if(!head){
        printf(FG_CYAN "  " VL RESET "\n");
        msg_inf("No records found.");
    } else {
        printf(FG_CYAN "  " VL RESET "\n");
        printf(FG_CYAN "  " VL RESET "  " BOLD FG_WHITE "%-18s  %-14s  %-12s" RESET "  " FG_CYAN VL "\n" RESET,
               "DATE","AMOUNT","CATEGORY");
        printf(FG_CYAN "  " VL FG_GRAY); rep("─", BOX_W-4); printf(FG_CYAN VL "\n" RESET);

        double total=0; int count=0;
        for(Node *p=head; p; p=p->next){
            const char *ac = is_income ? FG_GREEN : FG_RED;
            const char *sg = is_income ? "+" : "-";
            printf(FG_CYAN "  " VL RESET
                   "  " FG_GRAY "%-18s" RESET "  %s%s৳ %-10.2f" RESET "  " FG_YELLOW "%-12s" RESET "  " FG_CYAN VL "\n" RESET,
                   p->date, ac, sg, p->amount, p->category);
            total+=p->amount; count++;
        }
        printf(FG_CYAN "  " VL FG_GRAY); rep("─", BOX_W-4); printf(FG_CYAN VL "\n" RESET);

        char buf[48];
        snprintf(buf,sizeof(buf),"৳ %.2f  (%d entries)",total,count);
        printf(FG_CYAN "  " VL RESET
               "  " BOLD FG_WHITE "%-18s  %s%-14s" RESET "  %-12s  " FG_CYAN VL "\n" RESET,
               "TOTAL", is_income?FG_GREEN:FG_RED, buf, "");
        printf(FG_CYAN "  " VL RESET "\n");
    }
    sec_close();
    wait_enter();
}

static void screen_exit(void)
{
    draw_header();
    blank_row();
    centre_row(FG_GREEN BOLD, "Data saved successfully.");
    centre_row(DIM FG_WHITE,  "Goodbye! Stay on budget.");
    blank_row();
    h_rule(BL, BR);
    printf("\n" SHOW_CURSOR);
}

/* ── main ─────────────────────────────────────────────────── */

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    Node *income_tail=NULL, *expense_tail=NULL;
    printf(HIDE_CURSOR);
    
    if (record_read(&currentincome, &currentexpense)) {
        income  = list_read("myincome.bin",  &income_tail);
        expense = list_read("myexpense.bin", &expense_tail);
    }

    int opt;
    do {
        draw_header();
        draw_summary();
        draw_menu();
        if(scanf("%d",&opt)!=1){ 
            clear_input();
            opt=0; 
            continue; 
        }
        switch(opt){
        case 1: screen_input(1,&income, &income_tail, &currentincome, "myincome.bin");  break;
        case 2: screen_input(0,&expense,&expense_tail,&currentexpense,"myexpense.bin"); break;
        case 3: screen_records(income, 1);  break;
        case 4: screen_records(expense,0);  break;
        case 5: record_write(currentincome,currentexpense); screen_exit(); break;
        default: msg_err("Invalid option — choose 1 to 5."); wait_enter(); break;
        }
    } while(opt!=5);

    list_free(income);
    list_free(expense);
    return 0;
}