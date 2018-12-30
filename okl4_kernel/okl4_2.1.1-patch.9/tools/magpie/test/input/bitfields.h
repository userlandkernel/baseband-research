typedef unsigned int Word;

typedef struct {
  Word a : 10;
  
  Word c : 2-3+4*5;

  Word b : (sizeof(Word) * 8 - 10);

} foo;
