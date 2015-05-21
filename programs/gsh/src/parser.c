#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"


static void node_add_child(struct ast_node *parent, struct ast_node *child);
static void node_free(struct ast_node *node);
static struct ast_node *parse_redirect(struct parser_context *ctx);
static struct ast_node *parse_pipe(struct parser_context *context);
static struct ast_node *parse_val(struct parser_context *ctx);
static int token_match(struct parser_context *context, token_type type);
static struct ast_node *node_create(node_type type, int size);
static struct token *token_read(struct parser_context *context);
static struct token *parser_get_token(struct token *list, int index);

void init_parser(struct parser_context *ctx, struct token *tokens)
{
    ctx->p_tokens = tokens;
    ctx->p_position = 0;
}


void uninit_parser(struct parser_context *ctx)
{
    node_free(ctx->p_root);
}

struct ast_node *parse(struct parser_context *ctx)
{
    ctx->p_root = parse_redirect(ctx);
    return ctx->p_root;
}

static struct ast_node *parse_redirect(struct parser_context *context)
{
    struct ast_node *left = parse_pipe(context);
    if(token_match(context, TOK_RDIR)) {
        token_read(context);
        struct ast_node *rdir = node_create(NODE_RDIR, sizeof(struct ast_node));
        node_add_child(rdir, left);
        node_add_child(rdir, parse_redirect(context));
        return rdir;
    }
    return left;
}

static struct ast_node *parse_pipe(struct parser_context *context)
{
    struct ast_node *left = parse_val(context);
    if(token_match(context, TOK_PIPE)) {
        token_read(context);
        struct ast_node *rdir = node_create(NODE_PIPE, sizeof(struct ast_node));
        node_add_child(rdir, left);
        node_add_child(rdir, parse_pipe(context));
        return rdir;
    }
    return left;
}

static struct ast_node *parse_val(struct parser_context *ctx)
{
    struct gsh_command *nod = (struct gsh_command*)node_create(NODE_CMD, sizeof(struct gsh_command));
    char **args = (char*)malloc(512);
    memset(args, 0, 512);
    int i = 0;
    while(token_match(ctx, TOK_STRING)) {
        struct token *tok = token_read(ctx);
        args[i++] = tok->value;
    }
    nod->c_args = args;
    return nod;
}

static struct ast_node *node_create(node_type type, int size)
{
    struct ast_node *node = (struct ast_node*)malloc(size);
    memset(node, 0, sizeof(struct ast_node));
    node->n_type = type;
    return node;
}

static void node_add_child(struct ast_node *parent, struct ast_node *child)
{
    struct ast_node *i = parent->n_children;
    if(i) {
        while(i->next) {
            i = i->next;
        }
        i->next = child;
    } else {
        parent->n_children = child;
    }
    child->next = NULL;
}

static void node_free(struct ast_node *node)
{
    struct ast_node *i = node->n_children;
    while(i) {
        struct ast_node *n = i;
        i = i->next;
        node_free(n);
    }
    if(node->n_type == NODE_CMD) {
        struct gsh_command *cmd = (struct gsh_command*)node;
        free(cmd->c_args);
    }
    free(node);
}

static int token_match(struct parser_context *context, token_type type)
{
    struct token *tok = parser_get_token(context->p_tokens, context->p_position);
    
    return tok != NULL && tok->type == type;
}

static struct token *token_read(struct parser_context *context)
{
    return parser_get_token(context->p_tokens, context->p_position++);
}

static struct token *parser_get_token(struct token *list, int index)
{
    int i = 0;
    struct token *n = list;
    while(n) {
        if(i == index)
            return n;
        n = n->next;
        i++;
    }
    return NULL;
}
