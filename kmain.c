
extern void main(int argc, char **argv);

void kmain()
{
    static char *argv[3] = { "frotz", 0 };

    main(1, argv);
}

