#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Asa arata structura definita pentru lista dublu inlantuita

typedef struct node {
    char data;
    struct node *next;
    struct node *prev;
} Node;

// Asa arata structura definita pentru banda magica
typedef struct banda_magica {
    Node *finger;
    Node *head;
} Band;

// Structura pentru coada (efectuata ca o lista dublu inlantuita circulara)
typedef struct queue {
    char *command;
    struct queue *next;
    struct queue *prev;
} Queue;

typedef struct stack {
    Node *address;
    struct stack *next;
} Stack;

/* Functia ce creaza prima celula si santinela pentru banda, ce returneaza banda
initiala */

Band *initBand() {
    Band *mband = calloc(1, sizeof(Band));
    mband->head = calloc(1, sizeof(Node));
    mband->head->prev = NULL;
    mband->head->data = 0;
    mband->head->next = calloc(1, sizeof(Node));
    mband->head->next->prev = mband->head;
    mband->head->next->data = '#';
    mband->head->next->next = NULL;
    // Actualizez pozitia degetului pe prima celula din banda
    mband->finger = mband->head->next;

    return mband;
}

/* Se da free recursiv la celula anterioara din banda incepand de la a doua
celula */

void freeBand(Band *mband) {
    Node *copy = mband->head->next;
    for (;; copy = copy->next) {
        free(copy->prev);
        // pentru ultima celula un if aparte pentru a-i da free
        if (copy->next == NULL) {
            free(copy);
            break;
        }
    }
    free(mband);
}

void freeStack(Stack **DO) {
    if (*DO != NULL) {
        Stack *copy = *DO;
        for (*DO = (*DO)->next;; copy = *DO, *DO = (*DO)->next) {
            free(copy);
            if (*DO == NULL) {
                break;
            }
        }
    }
}

// Functia ce adauga comenzile in coada

void enQueue(Queue **queue, char *commands) {
    // Daca coada este goala se aloca o celula noua
    if (*queue == NULL) {
        *queue = calloc(1, sizeof(Queue));
        (*queue)->command = calloc(20, sizeof(char));
        // Creez legaturile ca o lista dublu inlantuita circulara
        (*queue)->next = (*queue);
        (*queue)->prev = (*queue);
        // Adaug comanda in continutul celulei
        strcpy((*queue)->command, commands);
    } else {
        /* Am generalizat acelasi proces de mai sus insa pentru cazul
        cand coada are cel putin o celula */
        Queue *new_queue = calloc(1, sizeof(Queue));
        new_queue->command = calloc(20, sizeof(char));
        strcpy(new_queue->command, commands);
        (*queue)->prev->next = new_queue;
        new_queue->prev = (*queue)->prev;
        new_queue->next = *queue;
        (*queue)->prev = new_queue;
    }
}

void addinStack(Stack **DO, Node *cell) {
    Stack *stiva_cell = calloc(1, sizeof(Stack));
    stiva_cell->address = cell;
    stiva_cell->next = (*DO);
    (*DO) = stiva_cell;
}

Node *extractfromStack(Stack **DO) {
    if ((*DO) != NULL) {
        Node *aux = (*DO)->address;
        Stack *copy = (*DO);
        (*DO) = (*DO)->next;
        free(copy);
        return aux;
    } else {
        return NULL;
    }
}

void deQueue(Queue **queue, Band *mband, FILE *fout, Stack **UNDO,
             Stack **REDO) {
    /* Memorez prima celula din coada si ma mut pe urmatoarea,
    mai apoi ma folosesc de copia ei ca apoi sa ii dau free */
    Queue *mem_queue = *queue;
    (*queue)->next->prev = (*queue)->prev;
    (*queue)->prev->next = (*queue)->next;
    *queue = (*queue)->next;
    if (strcmp(mem_queue->command, "MOVE_LEFT") == 0) {
        // Efectuez doar daca pozitia degetului nu e pe prima celula
        if (mband->finger != mband->head->next) {
            addinStack(UNDO, mband->finger);
            mband->finger = mband->finger->prev;
        }
    }
    if (strcmp(mem_queue->command, "MOVE_RIGHT") == 0) {
        addinStack(UNDO, mband->finger);
        if (mband->finger->next == NULL) {
            /* Daca pozitia degetului se afla pe ultima celula aloc memorie
            pentru o noua celula */
            Node *new_cell = calloc(1, sizeof(Node));
            // Ii adaug caracterul '#'
            new_cell->data = '#';
            // Creez legaturile
            new_cell->prev = mband->finger;
            mband->finger->next = new_cell;
            new_cell->next = NULL;
            // Actualizez pozitia degetului la noua celula
            mband->finger = new_cell;
        } else {
            mband->finger = mband->finger->next;
        }
    }
    if (strncmp(mem_queue->command, "MOVE_LEFT_CHAR", 14) == 0) {
        /* Creez o copie pentru pozitia degetului pentru a nu-i schimba pozitia
        initiala */
        for (Node *aux = mband->finger; aux != mband->head; aux = aux->prev) {
            if (mem_queue->command[strlen(mem_queue->command) - 1] ==
                aux->data) {
                // Verific daca parametrul din comanda coincide cu valoarea din
                // celula Apoi actualizez pozitia degetului
                mband->finger = aux;
                break;
            }
        }
        // In caz ca am ajuns la periferia stanga se printeaza "ERROR"
        if (mband->finger->data !=
            mem_queue->command[strlen(mem_queue->command) - 1]) {
            fprintf(fout, "ERROR\n");
        }
    }
    if (strncmp(mem_queue->command, "MOVE_RIGHT_CHAR", 15) == 0) {
        for (; mband->finger->next != NULL;
             mband->finger = mband->finger->next) {
            if (mem_queue->command[strlen(mem_queue->command) - 1] ==
                mband->finger->data) {
                break;
                /* Daca parametrul coincide cu valoarea celulei
                iese din for si pozita degetului ramane ultima din for */
            }
        }
        // Daca s-a ajuns la periferia din dreapta se creeaza o noua celula
        if (mband->finger->data !=
            mem_queue->command[strlen(mem_queue->command) - 1]) {
            Node *new_cell = calloc(1, sizeof(Node));
            // Inserez valoarea '#' in noua celula
            new_cell->data = '#';
            // Fac legaturile
            new_cell->prev = mband->finger;
            mband->finger->next = new_cell;
            new_cell->next = NULL;
            mband->finger = new_cell;
        }
    }
    if (strncmp(mem_queue->command, "WRITE", 5) == 0) {
        /* Atribui parametrul din comanda pentru valoarea la care
        se afla degetul */
        mband->finger->data =
            mem_queue->command[strlen(mem_queue->command) - 1];
        freeStack(UNDO);
        freeStack(REDO);
    }
    if (strncmp(mem_queue->command, "INSERT_LEFT", 11) == 0) {
        if (mband->finger != mband->head->next) {
            Node *new_cell = calloc(1, sizeof(Node));
            new_cell->data = mem_queue->command[strlen(mem_queue->command) - 1];
            // Creez legaturile pentru noua celula din stanga
            new_cell->prev = mband->finger->prev;
            mband->finger->prev->next = new_cell;
            mband->finger->prev = new_cell;
            new_cell->next = mband->finger;
            // Actualizez pozitia degetului pe noua celula inserata la stanga
            mband->finger = new_cell;
        } else {
            fprintf(fout, "ERROR\n");
        }
    }
    if (strncmp(mem_queue->command, "INSERT_RIGHT", 12) == 0) {
        Node *new_cell = calloc(1, sizeof(Node));
        new_cell->data = mem_queue->command[strlen(mem_queue->command) - 1];
        if (mband->finger->next != NULL) {
            mband->finger->next->prev = new_cell;
        }
        /* Creez legaturile pentru noua celula din dreapta, caz special mai sus
        cand afla la periferia dreapta */
        new_cell->next = mband->finger->next;
        mband->finger->next = new_cell;
        new_cell->prev = mband->finger;

        mband->finger = new_cell;
    }
    // Dau free la comanda, mai apoi la celula
    free(mem_queue->command);
    // Daca asta a fost ultima celula o egalam cu NULL
    if (mem_queue == *queue) {
        *queue = NULL;
    }
    free(mem_queue);
}

void show_list(Band *mband, FILE *fout) {
    for (Node *i = mband->head->next; i != NULL; i = i->next) {
        if (i == mband->finger) {
            // Daca pozitia degetului se afla pe celula data o afisam intre '|'
            fprintf(fout, "|%c|", i->data);
        } else {
            fprintf(fout, "%c", i->data);
        }
    }
    fprintf(fout, "\n");
}

void show_current(Band *mband, FILE *fout) {
    fprintf(fout, "%c\n", mband->finger->data);
    // Afiseaza valoarea unde se afla degetul
}

int main(void) {
    // Deschid fisierele
    FILE *fin = fopen("tema1.in", "r");
    FILE *fout = fopen("tema1.out", "w");

    Band *mband = initBand();
    Stack *UNDO = NULL, *REDO = NULL;
    Queue *queue = NULL;

    int number, i, j = 0;  // number e numarul de comenzi din fisier
    fscanf(fin, "%d ", &number);
    /* Aloc memorie pentru commands, pointerul unde voi stoca rand pe rand
    cate o comanda citita din fisier */
    char *commands = calloc(20, sizeof(char));
    for (i = 0; i < number; i++) {
        j = 0;
        do {
            fscanf(fin, "%c", &commands[j]);
            j++;
        } while (commands[j - 1] != 10);
        commands[j - 1] = 0;
        if (strcmp(commands, "MOVE_LEFT") == 0) {
            enQueue(&queue, commands);
        }
        if (strcmp(commands, "MOVE_RIGHT") == 0) {
            enQueue(&queue, commands);
        }
        if (strncmp(commands, "MOVE_LEFT_CHAR", 14) == 0) {
            enQueue(&queue, commands);
        }
        if (strncmp(commands, "MOVE_RIGHT_CHAR", 15) == 0) {
            enQueue(&queue, commands);
        }
        if (strncmp(commands, "WRITE", 5) == 0) {
            enQueue(&queue, commands);
        }
        if (strncmp(commands, "INSERT_LEFT", 11) == 0) {
            enQueue(&queue, commands);
        }
        if (strncmp(commands, "INSERT_RIGHT", 12) == 0) {
            enQueue(&queue, commands);
        }
        if (strcmp(commands, "SHOW") == 0) {
            show_list(mband, fout);
        }
        if (strcmp(commands, "SHOW_CURRENT") == 0) {
            show_current(mband, fout);
        }
        if (strcmp(commands, "EXECUTE") == 0) {
            deQueue(&queue, mband, fout, &UNDO, &REDO);
        }
        if (strcmp(commands, "UNDO") == 0) {
            addinStack(&REDO, mband->finger);
            mband->finger = extractfromStack(&UNDO);
        }
        if (strcmp(commands, "REDO") == 0) {
            addinStack(&UNDO, mband->finger);
            mband->finger = extractfromStack(&REDO);
        }
    }
    free(commands);
    freeBand(mband);
    freeStack(&UNDO);
    freeStack(&REDO);
    // Inchid fisierele
    fclose(fin);
    fclose(fout);
    return 0;
}