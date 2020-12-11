#include <md4c-html.h>

#include "md2html.h"

typedef struct {
  gchar *text;
  gsize len;
} Context;

static void
process_output (const char *output, unsigned int size, gpointer user_data)
{
  Context *ctx = (Context *) user_data;
  ctx->len += size;
  ctx->text = g_realloc (ctx->text, ctx->len);
  ctx->text = strncat (ctx->text, output, size);
}

gchar *
md2html (gchar *input, GError **err) {
    Context ctx;
    ctx.len = 1; // reserve '\0'
    ctx.text = g_realloc (NULL, ctx.len);
    ctx.text[0] = '\0';

    md_html (input, strlen(input), process_output, &ctx, 0, 0);

    return ctx.text;
}
