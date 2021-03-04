#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1025

typedef struct h_node
{
    int h_addr1;
    int h_addr2;
    int h_last_row;
    int difference;
    char h_command;
    char **h_old_text;
    char **h_new_text;
} h_node;

char **PAGE = NULL;
int size = 150000;
char buf[BUF_SIZE];
int last_row = 0;
h_node **U_HEAD = NULL; //array che implementa la pila undo
h_node **R_HEAD = NULL; //array che implementa la pila redo
int u_alloc = 10000; //dimensione allocata di U_HEAD
int r_alloc = 10000; //dimensione allocata di R_HEAD
int u_size = 0; //numero di elementi presenti in U_HEAD
int r_size = 0; //numero di elementi presenti R_HEAD
int u = 0;
int r = 0;

void u_push(h_node *node);
void r_push(h_node *node);
h_node *makeNode(int addr1, int addr2, int h_last_row, int h_difference, char command, char **old_text, char **new_text);
void printLines(int addr1, int addr2);
void change(int addr1, int addr2);
void delete (int addr1, int addr2);
void undo();
void redo();
char **copyOldText(int addr1, int addr2, int h_difference);
void readCommands();

int main()
{
    PAGE = malloc(size * sizeof(char *));
    U_HEAD = malloc(u_alloc * sizeof(h_node *));
    R_HEAD = malloc(r_alloc * sizeof(h_node *));
    readCommands();
}
void u_push(h_node *node)
{
    if (u_size == u_alloc - 5)
    {
        u_alloc = u_alloc + 30;
        U_HEAD = realloc(U_HEAD, u_alloc * sizeof(h_node *));
    }

    U_HEAD[u_size] = node;
    u_size++;
}

void r_push(h_node *node)
{
    if (r_size == r_alloc - 5)
    {
        r_alloc = r_alloc + 30;
        R_HEAD = realloc(R_HEAD, r_alloc * sizeof(h_node *));
    }

    R_HEAD[r_size] = node;
    r_size++;
    u_size--;
}

h_node *makeNode(int addr1, int addr2, int h_last_row, int h_difference, char command, char **old_text, char **new_text)
{
    h_node *tmp = malloc(sizeof(h_node));
    tmp->h_addr1 = addr1;
    tmp->h_addr2 = addr2;
    tmp->h_last_row = h_last_row;
    tmp->difference = h_difference;
    tmp->h_command = command;
    tmp->h_old_text = old_text;
    tmp->h_new_text = new_text;
    return tmp;
}

char **copyOldText(int addr1, int addr2, int h_difference)
{
    char **tmp = malloc((h_difference) * sizeof(char *));
    
    for (int i = addr1; i <= addr2 && i <= last_row; i++) //copio le vecchie righe
    {
        tmp[i - addr1] = PAGE[i];
    }

    return tmp;
}

void change(int addr1, int addr2)
{
    int difference = 0;
    if (addr2 > last_row)
    {
        if (addr1 <= last_row)
        {
            difference = last_row - addr1 + 1;
        }
    }
    else
    {
        difference = addr2 - addr1 + 1;
    }

    char **old_text = copyOldText(addr1, addr2, difference);
    char **new_text = malloc((addr2 - addr1 + 1) * sizeof(char *));

    if (addr2 > size)
    {
        size = addr2 + 10;
        PAGE = realloc(PAGE, (size * sizeof(char *)));
    }

    int i = addr1;
    while (fgets(buf, BUF_SIZE, stdin) && buf[0] != '.')
    {
        char *line = malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(line, buf);
        PAGE[i] = line;
        new_text[i - addr1] = line;

        i++;
    }
    u_push(makeNode(addr1, addr2, last_row, difference, 'c', old_text, new_text));

    if (addr2 > last_row)
    {
        last_row = addr2;
    }
}

void delete (int addr1, int addr2)
{
    int difference = 0;
    if (addr2 > last_row)
    {
        if (addr1 <= last_row)
        {
            difference = last_row - addr1 + 1;
        }
    }
    else
    {
        difference = addr2 - addr1 + 1;
    }
    char **old_text = copyOldText(addr1, addr2, difference);
    u_push(makeNode(addr1, addr2, last_row, difference, 'd', old_text, NULL));

    if (addr2 < last_row)
    {
        if (addr1 != 0)
        {
            int deleted = addr2 - addr1 + 1;
            for (int i = addr2 + 1; i <= last_row; i++)
            {
                PAGE[i - deleted] = PAGE[i];
            }
            last_row = last_row - deleted;
        }
        else
        {
            int deleted = addr2;
            for (int i = addr2 + 1; i <= last_row; i++)
            {
                PAGE[i - deleted] = PAGE[i];
            }
            last_row = last_row - deleted;
        }
    }
    else //addr2 >= last_row
    {
        if (addr1 <= last_row)
        {
            if (addr1 != 0)
            {
                last_row = addr1 - 1;
            }
            else
            {
                last_row = 0;
            }
        }
        else
        {
            last_row = last_row;
        }
    }
}

void printLines(int addr1, int addr2)
{
    for (int i = addr1; i <= addr2; i++)
    {
        if (i > 0 && i <= last_row)
        {
            fputs(PAGE[i], stdout);
        }
        else
        {
            fputs(".\n", stdout);
        }
    }
}

void redo()
{
    h_node *tmp = R_HEAD[r_size - 1];
    if (tmp->h_command == 'c')
    {
        for (int i = tmp->h_addr1; i <= tmp->h_addr2; i++)
        {
            PAGE[i] = tmp->h_new_text[i - tmp->h_addr1];
        }
        u_push(R_HEAD[r_size - 1]);
        r_size--;
        if (tmp->h_addr2 > last_row)
        {
            last_row = tmp->h_addr2;
        }
    }
    else
    {
        delete (tmp->h_addr1, tmp->h_addr2);
        r_size--;
    }
}

void undo()
{
    h_node *tmp = U_HEAD[u_size - 1];

    if (tmp->h_command == 'c')
    {
        for (int i = 0; i < tmp->difference; i++)
        {
            PAGE[i + tmp->h_addr1] = tmp->h_old_text[i];
        }
    }
    else
    {

        if (tmp->h_addr1 <= last_row)
        {
            if (tmp->h_addr1 != 0)
            {
                int deleted = tmp->h_addr2 - tmp->h_addr1 + 1;
                for (int i = last_row; i >= tmp->h_addr1; i--)
                {
                    PAGE[i + deleted] = PAGE[i];
                }
                for (int i = 0; i < tmp->difference; i++)
                {
                    PAGE[i + tmp->h_addr1] = tmp->h_old_text[i];
                }
            }
            else
            {
                int deleted = tmp->h_addr2;
                if (deleted)
                {
                    for (int i = last_row; i >= tmp->h_addr1; i--)
                    {
                        PAGE[i + deleted] = PAGE[i];
                    }
                    for (int i = 0; i < tmp->difference; i++)
                    {
                        PAGE[i + tmp->h_addr1] = tmp->h_old_text[i];
                    }
                }
                else
                {
                    //do nothing...
                }
            }
        }
        else
        {
            if (tmp->h_addr1 = last_row + 1)
            {
                for (int i = tmp->h_addr1; i <= tmp->h_addr2 && i <= tmp->h_last_row; i++)
                {
                    PAGE[i] = tmp->h_old_text[i - tmp->h_addr1];
                }
            }
        }
    }
    last_row = tmp->h_last_row;
    r_push(U_HEAD[u_size - 1]);
}

void readCommands()
{
    char *addr1_str = NULL;
    char *addr2_str = NULL;
    int addr1 = 0;
    int addr2 = 0;
    char command = '\0';
    size_t command_lenght = 0;
    int numero = 0;

    while (fgets(buf, BUF_SIZE, stdin) && buf[0] != 'q')
    {
        command_lenght = strlen(buf);
        command = buf[command_lenght - 2];
        buf[command_lenght - 2] = '\0';

        if (command == 'u' || command == 'r')
        {
            int u_size_tmp = u_size;
            int r_size_tmp = r_size;
            u = 0;
            r = 0;
            do
            {
                numero = atoi(buf);

                if (command == 'u')
                {
                    if (numero >= u_size_tmp)
                    {
                        u += u_size_tmp;
                        r_size_tmp = r_size_tmp + u_size_tmp;
                        u_size_tmp = 0;
                    }
                    else
                    {
                        u += numero;
                        r_size_tmp = r_size_tmp + numero;
                        u_size_tmp = u_size_tmp - numero;
                    }
                }
                else //command == 'r'
                {
                    if (numero >= r_size_tmp)
                    {
                        r += r_size_tmp;
                        u_size_tmp = u_size_tmp + r_size_tmp;
                        r_size_tmp = 0;
                    }
                    else
                    {
                        r += numero;
                        u_size_tmp = u_size_tmp + numero;
                        r_size_tmp = r_size_tmp - numero;
                    }
                }

                fgets(buf, BUF_SIZE, stdin);
                if (buf[0] == 'q')
                    return;

                command_lenght = strlen(buf);
                command = buf[command_lenght - 2];
                buf[command_lenght - 2] = '\0';

            } while (command == 'u' || command == 'r');

            if (u > r)
            {
                u = u - r;
                if (u >= u_size / 2)
                {
                    last_row = 0;
                    int usize = u_size;
                    for (int i = 0; i < usize; i++)
                    {
                        r_push(U_HEAD[u_size - 1]);
                    }
                    last_row = 0;

                    for(int i = 0; i < (usize - u); i++)
                    {
                        redo();
                    }
                }
                else
                {
                    for (int i = 0; i < u; i++)
                    {
                        undo();
                    }
                }
            }
            else if (r > u)
            {
                r = r - u;
                for (int i = 0; i < r; i++)
                {
                    redo();
                }
            }
            else
            {
                //no undos or redos
            }
        }

        addr1_str = strtok(buf, ",");
        addr2_str = strtok(NULL, ",");
        addr1 = atoi(addr1_str);
        addr2 = atoi(addr2_str);
        if (command == 'c')
        {
            change(addr1, addr2);
            r_size = 0;
        }
        else if (command == 'd')
        {
            delete (addr1, addr2);
            r_size = 0;
        }
        else if (command == 'p')
        {
            printLines(addr1, addr2);
        }
        else
        {
            printf("\n\n\nERROR\n\n");
        }
    }
}