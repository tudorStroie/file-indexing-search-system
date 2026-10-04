#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct KeywordNode {
    char *keyword;
    struct KeywordNode *next;
} KeywordNode;

typedef struct FileNode {
    char *id;
    int score;
    KeywordNode *keywords;
    struct FileNode *next;
    struct FileNode *prev;
} FileNode, *TListFile;

typedef struct RefNode {
    FileNode *file;
    struct RefNode *next;
} RefNode;

typedef struct trieNode {
    struct trieNode *children[27];
    RefNode *referral;
    int terminal;
} TrieNode, *TTrie;

TTrie initTrieNode() {
    TTrie t = malloc(sizeof(TrieNode));
    if(t == NULL) {
        printf("Eroare alocare trie\n");
        exit(1);
    }
    for(int i = 0; i < 27; i++)
        t->children[i] = NULL;
    t->referral = NULL;
    t->terminal = 0;
    return t;
}

typedef struct heap {
    long int maxHeapSize;
    long int size;
    FileNode **elem;
} PriQueue, *APriQueue;

void PQInit(APriQueue q, int maxSize) {
    q->size = 0;
    q->maxHeapSize = maxSize + 1;
    q->elem = (FileNode**) malloc(q->maxHeapSize * sizeof(FileNode*));
    if(q->elem == NULL) {
        printf("Eroare alocare heap\n");
        exit(1);
    }
}

typedef struct {
    TListFile sentinel;
    TTrie trie_root;
} System;

TListFile initListFile(System *sys) {
    TListFile sentinel = malloc(sizeof(FileNode));
    if(sentinel == NULL) {
        printf("Eroare alocare sentinel\n");
        exit(1);
    }
    sentinel->id = NULL;
    sentinel->score = 0;
    sentinel->keywords = NULL;
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    sys->sentinel = sentinel;
    return sentinel;
}

void trieInsert(TTrie root, char *keyword, FileNode *file) {
    TTrie cur = root;
    for(int i = 0; keyword[i] != '\0'; i++) {
        int index = keyword[i] - 'a';
        if(cur->children[index] == NULL)
            cur->children[index] = initTrieNode();
        cur = cur->children[index];
    }
    cur->terminal = 1;
    RefNode *referral = malloc(sizeof(RefNode));
    if(referral == NULL) {
        printf("Eroare alocare referral\n");
        exit(1);
    }
    referral->file = file;
    referral->next = cur->referral;
    cur->referral = referral;
}

TTrie trieSearch(TTrie root, char *keyword) {
    TTrie cur = root;
    for(int i = 0; keyword[i] != '\0'; i++) {
        int index = keyword[i] - 'a';
        if(cur->children[index] == NULL)
            return NULL;
        cur = cur->children[index];
    }
    if(cur->terminal == 0)
        return NULL;
    return cur;
}

TListFile findFile(TListFile sentinel, char *id) {
    TListFile cur = sentinel->next;
    while(cur != sentinel) {
        if(strcmp(cur->id, id) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void addKeyword(TListFile file, char *keyword) {
    KeywordNode *word = malloc(sizeof(KeywordNode));
    if(word == NULL) {
        printf("Eroare alocare keyword\n");
        exit(1);
    }
    word->keyword = malloc(strlen(keyword) + 1);
    strcpy(word->keyword, keyword);
    word->next = file->keywords;
    file->keywords = word;
}

void removeKeyword(TListFile file, char *keyword) {
    KeywordNode *cur = file->keywords;
    KeywordNode *prev = NULL;
    while(cur != NULL) {
        if(strcmp(cur->keyword, keyword) == 0) {
            if(prev == NULL)
                file->keywords = cur->next;
            else
                prev->next = cur->next;
            free(cur->keyword);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

int fileHasKeyword(TListFile file, char *keyword) {
    KeywordNode *cur = file->keywords;
    while(cur != NULL) {
        if(strcmp(cur->keyword, keyword) == 0)
            return 1;
        cur = cur->next;
    }
    return 0;
}

void add_file(System *sys, char *id, int score, char keywords[][100], int n_keywords, FILE *fout) {
    if(findFile(sys->sentinel, id) != NULL) {
        fprintf(fout, "EXISTS\n");
        return ;
    }
    TListFile new = malloc(sizeof(FileNode));
    if(new == NULL) {
        printf("Eroare alocare file\n");
        exit(1);
    }
    new->id = malloc(strlen(id) + 1);
    strcpy(new->id, id);
    new->score = score;
    new->keywords = NULL;

    TListFile tail = sys->sentinel->prev;
    new->next = sys->sentinel;
    new->prev = tail;
    tail->next = new;
    sys->sentinel->prev = new;

    for(int i = 0; i < n_keywords; i++) {
        if(!fileHasKeyword(new, keywords[i])){
            addKeyword(new, keywords[i]);
            trieInsert(sys->trie_root, keywords[i], new);
        }
    }
    fprintf(fout, "OK\n");
}

int trieHasChildren(TTrie node) {
    for(int i = 0; i < 26; i++)
        if(node->children[i] != NULL)
            return 1;
    return 0;
}

void removeRef(TTrie node, TListFile file) {
    RefNode *cur = node->referral;
    RefNode *prev = NULL;
    while(cur != NULL) {
        if(cur->file == file) {
            if(prev == NULL)
                node->referral = cur->next;
            else
                prev->next = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

int trieDeleteWord(TTrie node, char *keyword, int depth, TListFile file) {
    if(node == NULL)
        return 0;
    if(keyword[depth] == '\0') {
        removeRef(node, file);
        if(node->referral == NULL)
            node->terminal = 0;
        return (!trieHasChildren(node) && node->terminal == 0);
    }
    int index = keyword[depth] - 'a';
    if(trieDeleteWord(node->children[index], keyword, depth + 1, file)) {
        free(node->children[index]);
        node->children[index] = NULL;
        return (!trieHasChildren(node) && node->terminal == 0);
    }
    return 0;
}

void deleteFile(System *sys, char *id, FILE *fout) {
    TListFile file = findFile(sys->sentinel, id);
    if(file == NULL) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }
    KeywordNode *word = file->keywords;
    while(word != NULL) {
        trieDeleteWord(sys->trie_root, word->keyword, 0, file);
        word = word->next;
    }
    file->prev->next = file->next;
    file->next->prev = file->prev;
    KeywordNode *cur = file->keywords;
    while(cur != NULL) {
        KeywordNode *temp = cur->next;
        free(cur->keyword);
        free(cur);
        cur = temp;
    }
    free(file->id);
    free(file);
    fprintf(fout, "OK\n");
}

void addkw(System *sys, char *id, char *keyword, FILE *fout) {
    TListFile file = findFile(sys->sentinel, id);
    if(file == NULL) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }
    if(fileHasKeyword(file, keyword)) {
        fprintf(fout, "OK\n");
        return;
    }
    addKeyword(file, keyword);
    trieInsert(sys->trie_root, keyword, file);
    fprintf(fout, "OK\n");
}

void delkw(System *sys, char *id, char *keyword, FILE *fout) {
    TListFile file = findFile(sys->sentinel, id);
    if(file == NULL) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }
    if(!fileHasKeyword(file, keyword)) {
        fprintf(fout, "OK\n");
        return;
    }
    trieDeleteWord(sys->trie_root, keyword, 0, file);
    removeKeyword(file, keyword);
    if(file->keywords == NULL) {
        file->prev->next = file->next;
        file->next->prev = file->prev;
        free(file->id);
        free(file);
    }
    fprintf(fout, "OK\n");
}

int compareID(const void *a, const void *b) {
    return strcmp(*(char**)a, *(char**)b);
}

void find(System *sys, char *keyword, FILE *fout) {
    TTrie node = trieSearch(sys->trie_root, keyword);
    if(node == NULL || node->referral == NULL) {
        fprintf(fout, "EMPTY\n");
        return;
    }
    int c = 0;
    RefNode *referral = node->referral;
    while(referral != NULL) {
        c++;
        referral = referral->next;
    }
    char **id = malloc(c * sizeof(char*));
    if(id == NULL) {
        printf("Eroare alocare id\n");
        exit(1);
    }
    referral = node->referral;
    for(int i = 0; i < c; i++) {
        id[i] = referral->file->id;
        referral = referral->next;
    }
    qsort(id, c, sizeof(char*), compareID);
    fprintf(fout, "%d", c);
    for(int i = 0; i < c; i++)
        fprintf(fout, " %s", id[i]);
    fprintf(fout, "\n");
    free(id);
}

void SiftUp(APriQueue q, int i) {
    int parent;
    FileNode *temp;
    while(i > 1) {
        parent = i / 2;
        if(q->elem[parent]->score > q->elem[i]->score)
            break;
        if(q->elem[parent]->score == q->elem[i]->score && strcmp(q->elem[parent]->id, q->elem[i]->id) < 0)
            break;
        temp = q->elem[parent];
        q->elem[parent] = q->elem[i];
        q->elem[i] = temp;
        i = parent;
    }
}

void SiftDown(APriQueue q, int i) {
    int j;
    FileNode *temp;
    while(2 * i <= q->size) {
        j = 2 * i;
        if(j < q->size) {
            if(q->elem[j]->score < q->elem[j + 1]->score)
                j++;
            else if(q->elem[j]->score == q->elem[j + 1]->score && strcmp(q->elem[j]->id, q->elem[j + 1]->id) > 0)
                j++;
        }
        if(q->elem[i]->score > q->elem[j]->score)
            break;
        if(q->elem[i]->score == q->elem[j]->score && strcmp(q->elem[i]->id, q->elem[j]->id) < 0)
            break;
        temp = q->elem[i];
        q->elem[i] = q->elem[j];
        q->elem[j] = temp;
        i = j;
    }
}

void Insert(APriQueue q, FileNode *file) {
    if(q->size == q->maxHeapSize - 1)
        return;
    q->size++;
    q->elem[q->size] = file;
    SiftUp(q, q->size);
}

FileNode *ExtractMax(APriQueue q) {
    FileNode *res;
    if(q->size == 0)
        return NULL;
    res = q->elem[1];
    q->elem[1] = q->elem[q->size];
    q->size--;
    SiftDown(q, 1);
    return res;
}

void topk(System *sys, char *keyword, int k, FILE *fout) {
    TTrie node = trieSearch(sys->trie_root, keyword);
    if(node == NULL || node->referral == NULL) {
        fprintf(fout, "EMPTY\n");
        return;
    }
    int c = 0;
    RefNode *referral = node->referral;
    while(referral != NULL) {
        c++;
        referral = referral->next;
    }
    APriQueue q = malloc(sizeof(PriQueue));
    if(q == NULL) {
        printf("Eroare alocare heap\n");
        exit(1);
    }
    PQInit(q, c);
    referral = node->referral;
    while(referral != NULL) {
        Insert(q, referral->file);
        referral = referral->next;
    }
    int extr_done = 0;
    int extr;
    if(k < c)
        extr = k;
    else 
        extr = c;
    fprintf(fout, "%d", extr);
    while(extr_done < extr) {
        FileNode *temp = ExtractMax(q);
        fprintf(fout, " %s", temp->id);
        extr_done++;
    }
    fprintf(fout, "\n");
    free(q->elem);
    free(q);
}

void printTrie(TTrie node, char *buffer, int depth, FILE *fout) {
    if(node == NULL)
        return;
    if(node->terminal == 1) {
        buffer[depth] = '\0';
        int c = 0;
        RefNode *referral = node->referral;
        while(referral != NULL) {
            c++;
            referral = referral->next;
        }
        char **id = malloc(c * sizeof(char*));
        if(id == NULL){
            printf("Eroare alocare id\n");
            exit(1);
        }
        referral = node->referral;
        for(int i = 0; i < c; i++) {
            id[i] = referral->file->id;
            referral = referral->next;
        }
        qsort(id, c, sizeof(char*), compareID);
        fprintf(fout, "%s %d", buffer, c);
        for(int i = 0; i < c; i++)
            fprintf(fout, " %s", id[i]);
        fprintf(fout, "\n");
        free(id);
    }
    for(int i = 0; i < 26; i++) {
        if(node->children[i] != NULL) {
            buffer[depth] = 'a' + i;
            printTrie(node->children[i], buffer, depth + 1, fout);
        }
    }
}

void print(System *sys, FILE *fout) {
    TTrie root = sys->trie_root;
    int empty = 1;
    for(int i = 0; i < 26; i++) {
        if(root->children[i] != NULL) {
            empty = 0;
            break;
        }
    }
    if(empty) {
        fprintf(fout, "EMPTY\n");
        return;
    }
    char buffer[101];
    printTrie(root, buffer, 0, fout);
}

void freeTrie(TTrie node) {
    if(node == NULL)
        return;
    for(int i = 0; i< 26; i++)
        freeTrie(node->children[i]);
    RefNode *cur = node->referral;
    while(cur != NULL) {
        RefNode *temp = cur->next;
        free(cur);
        cur = temp;
    }
    free(node);
}

void freeSystem(System *sys) {
    TListFile cur = sys->sentinel->next;
    while(cur != sys->sentinel) {
        TListFile temp = cur->next;
        KeywordNode *keyword = cur->keywords;
        while(keyword != NULL) {
            KeywordNode *kwtemp = keyword->next;
            free(keyword->keyword);
            free(keyword);
            keyword = kwtemp;
        }
        free(cur->id);
        free(cur);
        cur = temp;
    }
    free(sys->sentinel);
    freeTrie(sys->trie_root);
}

void DFS(TTrie node, FileNode **res, int *count) {
    if(node == NULL)
        return;
    if(node->terminal == 1) {
        RefNode *referral = node->referral;
        while(referral != NULL) {
            int gasit = 0;
            for(int i = 0; i < *count; i++) {
                if(res[i] == referral->file) {
                    gasit = 1;
                    break;
                }
            }
            if(!gasit) {
                res[*count] = referral->file;
                (*count)++;
            }
            referral = referral->next;
        }
    }
    for(int i = 0; i < 26; i++)
        DFS(node->children[i], res, count);
}

void prefix(System *sys, char *pref, FILE *fout) {
    TTrie cur = sys->trie_root;
    for(int i = 0; pref[i] != '\0'; i++) {
        int index = pref[i] - 'a';
        if(cur->children[index] == NULL) {
            fprintf(fout, "EMPTY\n");
            return;
        }
        cur = cur->children[index];
    }
    FileNode **res = malloc(10000 * sizeof(FileNode*));
    if(res == NULL) {
        printf("Eroare alocare rezultate\n");
        exit(1);
    }
    int count = 0;
    DFS(cur, res, &count);
    if(count == 0) {
        fprintf(fout, "EMPTY\n");
        free(res);
        return;
    }
    char **id = malloc(count * sizeof(char*));
    if(id == NULL) {
        printf("Eroare alocare id\n");
        exit(1);
    }
    for(int i = 0; i < count; i++)
        id[i] = res[i]->id;
    qsort(id, count, sizeof(char*), compareID);

    fprintf(fout, "%d", count);
    for(int i = 0; i < count; i++)
        fprintf(fout, " %s", id[i]);
    fprintf(fout, "\n");

    free(id);
    free(res);
}

int main() {
    FILE *fin = fopen("indexare.in", "r");
    FILE *fout = fopen("indexare.out", "w");
    System sys;
    sys.sentinel = initListFile(&sys);
    sys.trie_root = initTrieNode();

    int Q;
    fscanf(fin, "%d\n", &Q);
    for(int i = 0; i < Q; i++) {
        char cmd[20];
        fscanf(fin, "%s", cmd);

        if(strcmp(cmd, "ADD") == 0) {
            char id[300];
            int score, t;
            fscanf(fin, "%s %d %d", id, &score, &t);
            char keywords[200][100];
            for(int j = 0; j < t; j++)
                fscanf(fin, "%s", keywords[j]);
            add_file(&sys, id, score, keywords, t, fout);

        } else if(strcmp(cmd, "DEL") == 0) {
            char id[100];
            fscanf(fin, "%s", id);
            deleteFile(&sys, id, fout);

        } else if(strcmp(cmd, "ADDKW") == 0) {
            char id[100], keyword[100];
            fscanf(fin, "%s %s", id, keyword);
            addkw(&sys, id, keyword, fout);

        } else if(strcmp(cmd, "DELKW") == 0) {
            char id[100], keyword[100];
            fscanf(fin, "%s %s", id, keyword);
            delkw(&sys, id, keyword, fout);

        } else if(strcmp(cmd, "FIND") == 0) {
            char keyword[100];
            fscanf(fin, "%s", keyword);
            find(&sys, keyword, fout);

        } else if(strcmp(cmd, "TOPK") == 0) {
            char keyword[100];
            int k;
            fscanf(fin, "%s %d", keyword, &k);
            topk(&sys, keyword, k, fout);

        } else if(strcmp(cmd, "PRINT") == 0) {
            print(&sys, fout);
        } else if(strcmp(cmd, "PREFIX") == 0) {
            char pref[100];
            fscanf(fin, "%s", pref);
            prefix(&sys, pref, fout);
        }
    }

    freeSystem(&sys);
    fclose(fin);
    fclose(fout);
    return 0;
}
