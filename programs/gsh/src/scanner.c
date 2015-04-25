#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"


static int scanner_next(struct tokenizer *context);
static void scanner_scan_ident(struct tokenizer *context) ;

void init_tokenizer(struct tokenizer *scanner, const char *source)
{
    memset(scanner, 0, sizeof(struct tokenizer));
    scanner->source = source;
}


void uninit_tokenizer(struct tokenizer *scanner)
{
    struct token *tok = scanner->token_list;
    while(tok) {
        struct token *t = tok;
        tok = tok->next;
        free(t->value);
        free(t);
    }
}

int tokenize(struct tokenizer *context)
{
    int i = 0;
    while(context->source[context->position]) {
        while(context->source[context->position] == ' ')
            context->position++;
        if(context->source[context->position]) {
            scanner_scan_ident(context);
        }
    }
    return context->tokens;
}

static int scanner_next(struct tokenizer *context)
{
    if(context->source[context->position])
        return (int)context->source[context->position++];
    else
        return -1;
}

static void scanner_scan_ident(struct tokenizer *context) 
{
    char *buf = (char*)malloc(128);
    memset(buf, 0, 128);
    int i = 0;
    int c = scanner_next(context);
    while(c != -1 && c != ' ') {
        buf[i++] = (char)c;
        c = scanner_next(context);
    }
    struct token *tok = (struct token*)malloc(sizeof(struct token));
    memset(tok, 0, sizeof(struct token));
    tok->value = buf;
    if(context->token_list) {
        struct token *n = context->token_list;
        while(n->next)
            n = n->next;
        n->next = tok;
    } else {
        context->token_list = tok;
    }
    context->tokens++;
}
