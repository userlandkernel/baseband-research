static int g1 = 0;
typedef long mylong;
static mylong z;

typedef int Length;
Length len, maxlen;
Length *lengths[];

typedef char *String;
String p, lineptr[100], alloc(int);
int strcmp(String, String);
/*
typedef struct tnode *Treeptr;

typedef struct tnode {
    char *word;
    int count;
    Treeptr left;
    Treeptr right;
} Treenode;

Treeptr talloc(void) {
    return (Treeptr) malloc(sizeof(Treenode));
}
           
int fun(int a, mylong b) {
    mylong c;
    return a+b;
}
*/
    
