#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"


static int scanner_next(struct tokenizer *context);
static void scanner_scan_redirect(struct tokenizer *context);
static void scanner_scan_redir(struct tokenizer *context);
static void scanner_scan_pipe(struct tokenizer *context);
static void scanner_scan_ident(struct tokenizer *context);
static void scanner_add_token(struct tokenizer *context, struct token *tok);

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
        if(t->value != NULL)
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
            switch(context->source[context->position]) {
                case '>':
                    scanner_scan_redir(context);
                    break;
                case '|':
                    scanner_scan_pipe(context);
                    break;
                default:
                    scanner_scan_ident(context);
                    break;
                
            }
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

static void scanner_scan_redir(struct tokenizer *context)
{
    struct token *tok = (struct token*)malloc(sizeof(struct token));
    memset(tok, 0, sizeof(struct token));
    scanner_next(context);
    if(context->source[context->position] == '>') {
        scanner_next(context);
        tok->type = TOK_RDIR;
    } else {
        tok->type = TOK_RDIR_A;
        
    }
    scanner_add_token(context, tok);
    
}

static void scanner_scan_pipe(struct tokenizer *context)
{
    struct token *tok = (struct token*)malloc(sizeof(struct token));
    memset(tok, 0, sizeof(struct token));
    scanner_next(context);
    tok->type = TOK_PIPE;
    scanner_add_token(context, tok);
    
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
    tok->type = TOK_STRING;
    scanner_add_token(context, tok);
}

static void scanner_add_token(struct tokenizer *context, struct token *tok)
{
    if(context->token_list) {
        struct token *n = context->token_list;
        while(n->next)
            n = n->next;
        n->next = tok;
    } else {
        context->token_list = tok;
    }
    tok->next = NULL;
    context->tokens++;
}
