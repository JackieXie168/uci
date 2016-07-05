#include <uci.h>

int main()
{
    uci_context *ctx = uci_alloc_context();
    uci_free_context(ctx);
    return 0;
}