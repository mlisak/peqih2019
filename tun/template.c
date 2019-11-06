
#include <stdio.h>

void
generate_network_json(char *template_path, char *out_path, char *dst)
{
    char buf[4096];
    char buf2[4096];
    FILE *f = fopen(template_path, "r");
    char *l = "0.0.0.0";
    fread(buf, 1, sizeof(buf), f);
    fclose(f);
    snprintf(buf2, sizeof(buf2), buf, l, l, l, dst, dst, dst);

    printf("%s", buf2);
}