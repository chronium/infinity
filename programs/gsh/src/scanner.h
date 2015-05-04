#ifndef SCANNER_H
#define SCANNER_H

typedef enum {
    TOK_STRING  = 0,
    TOK_PIPE    = 1,
    TOK_RDIR    = 2, // >>
    TOK_RDIR_A  = 3  // >  
} token_type;

struct token {
    token_type              type;
    const char  *           value;
    struct token *          next;
};

struct tokenizer {
    int                     position;
    char    *               source;
    int                     tokens;
    struct token *          token_list;  
    
};

void init_tokenizer(struct tokenizer *scanner, const char *source);
void uninit_tokenizer(struct tokenizer *scanner);
int tokenize(struct tokenizer *context);

#endif
