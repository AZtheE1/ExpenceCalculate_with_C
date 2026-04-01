#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 50
#define M 30

double currentincome  = 0;
double currentexpense = 0;

typedef struct node {
    char   date[M];
    double amount;
    char   category[N];
    struct node *next;
} Node;

typedef struct {
    double x, y;
} Record;

Node *income  = NULL;
Node *expense = NULL;

/* ── helpers ──────────────────────────────────────────────── */

static void print_divider(void)
{
    puts("________________________________________________"
         "________________________________________________\n");
}

/* Append a new record to a linked list in O(1) via tail pointer.
   We keep a static tail per list; pass the list's head pointer. */
static void list_append(Node **head, Node **tail,
                        const char date[], double amount,
                        const char category[])
{
    Node *n = malloc(sizeof(Node));
    if (!n) { fputs("Out of memory\n", stderr); exit(1); }
    strncpy(n->date,     date,     M - 1); n->date[M - 1]     = '\0';
    strncpy(n->category, category, N - 1); n->category[N - 1] = '\0';
    n->amount = amount;
    n->next   = NULL;

    if (*head == NULL) { *head = *tail = n; }
    else               { (*tail)->next = n; *tail = n; }
}

static void list_free(Node *head)
{
    while (head) { Node *tmp = head->next; free(head); head = tmp; }
}

/* ── file I/O ─────────────────────────────────────────────── */

/* Write an entire linked list to a binary file.
   The next pointer is zeroed before writing so the stored struct is clean. */
static int list_write(Node *head, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) { printf("Cannot save to %s\n", filename); return 0; }

    for (Node *p = head; p; p = p->next) {
        Node tmp = *p;
        tmp.next = NULL;
        fwrite(&tmp, sizeof(Node), 1, fp);
    }
    fclose(fp);
    return 1;
}

/* Read a binary file back into a freshly allocated linked list.
   Returns the head, or NULL on error / empty file. */
static Node *list_read(const char *filename, Node **tail_out)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size    = ftell(fp);
    rewind(fp);
    int  entries = (int)(size / sizeof(Node));

    Node *head = NULL, *tail = NULL;
    for (int i = 0; i < entries; i++) {
        Node *n = malloc(sizeof(Node));
        if (!n) { fputs("Out of memory\n", stderr); fclose(fp); exit(1); }
        fread(n, sizeof(Node), 1, fp);
        n->next = NULL;
        if (!head) { head = tail = n; }
        else       { tail->next = n; tail = n; }
    }
    fclose(fp);
    if (tail_out) *tail_out = tail;
    return head;
}

/* Persist totals between sessions */
static void record_write(double inc, double exp)
{
    FILE *fp = fopen("Record.bin", "wb");
    if (!fp) { fputs("Cannot save Record.bin\n", stderr); return; }
    Record r = { inc, exp };
    fwrite(&r, sizeof(Record), 1, fp);
    fclose(fp);
}

static int record_read(double *inc, double *exp)
{
    FILE *fp = fopen("Record.bin", "rb");
    if (!fp) return 0;
    Record r;
    int ok = (fread(&r, sizeof(Record), 1, fp) == 1);
    fclose(fp);
    if (ok) { *inc = r.x; *exp = r.y; }
    return ok;
}

/* ── display ──────────────────────────────────────────────── */

static void display_list(Node *head, const char *label)
{
    printf("*********  YOUR %s RECORD  *********\n\n", label);
    if (!head) { puts("NO RECORDS AVAILABLE\n"); print_divider(); return; }
    for (Node *p = head; p; p = p->next)
        printf("Date: %s\nAmount: %.2f Taka\nCategory: %s\n\n",
               p->date, p->amount, p->category);
    print_divider();
}

static void print_summary(void)
{
    puts("           _______________________________________________");
    printf("          |  YOUR INCOME   =  %.2f Taka\n", currentincome);
    printf("          |  YOUR EXPENSE  =  %.2f Taka\n", currentexpense);
    printf("          |  YOUR BALANCE  =  %.2f Taka\n", currentincome - currentexpense);
    puts("          |_______________________________________________\n");
}

/* ── main ─────────────────────────────────────────────────── */

int main(void)
{
    /* tail pointers so append is O(1) */
    Node *income_tail  = NULL;
    Node *expense_tail = NULL;

    record_read(&currentincome, &currentexpense);

    income  = list_read("myincome.bin",  &income_tail);
    expense = list_read("myexpense.bin", &expense_tail);

    int option;
    do {
        print_summary();
        puts("ENTER THE OPTION FROM THE BELOW\n");
        puts("1. INSERT INCOME");
        puts("2. INSERT EXPENSE");
        puts("3. VIEW INCOME RECORD");
        puts("4. VIEW EXPENSE RECORD");
        puts("5. EXIT");
        if (scanf("%d", &option) != 1) { option = 0; continue; }
        puts("");

        switch (option) {
        case 1:
        case 2: {
            char s1[15], s2[15], s3[15], date[M], category[N];
            double amount;
            int is_income = (option == 1);

            printf("***** ADD %s *****\n\n", is_income ? "INCOME" : "EXPENSE");
            printf("Enter date (day month year): ");
            if (scanf("%14s %14s %14s", s1, s2, s3) != 3) break;
            snprintf(date, M, "%s %s %s", s1, s2, s3);

            printf("Enter amount: ");
            if (scanf("%lf", &amount) != 1 || amount <= 0) {
                puts("Invalid amount.\n"); break;
            }
            printf("Enter category: ");
            if (scanf("%49s", category) != 1) break;

            if (is_income) {
                currentincome += amount;
                list_append(&income,  &income_tail,  date, amount, category);
                if (list_write(income, "myincome.bin"))
                    puts("\nINCOME SAVED SUCCESSFULLY\n");
            } else {
                currentexpense += amount;
                list_append(&expense, &expense_tail, date, amount, category);
                if (list_write(expense, "myexpense.bin"))
                    puts("\nEXPENSE SAVED SUCCESSFULLY\n");
            }
            print_divider();
            break;
        }
        case 3: display_list(income,  "INCOME");  break;
        case 4: display_list(expense, "EXPENSE"); break;
        case 5:
            record_write(currentincome, currentexpense);
            puts("Data saved. Goodbye!");
            break;
        default:
            puts("Invalid option — please choose 1-5.\n");
        }
    } while (option != 5);

    list_free(income);
    list_free(expense);
    return 0;
}