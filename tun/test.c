
void generate_network_json();

int
main (int argc, char **argv)
{
    char *template_path = "network.json.template";
    char *out_path = "network.json";
    generate_network_json(template_path, out_path, argv[1]);
}