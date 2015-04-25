#ifndef SCANNER_H
#define SCANNER_H

struct token {
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
