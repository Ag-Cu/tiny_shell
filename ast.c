#include "ast.h"
#include "alloc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct node *make_node(char *data, struct node *next){
    struct node *t = malloc(sizeof(*t));
    t->data = data;
    t->next = next;
    return t;
}

// constructor of each AST node
Cmd_t Cmd_Seq_new(Cmd_t left, Cmd_t right){
    Cmd_Seq cmd;
    NEW(cmd);
    cmd->type = CMD_SEQ;
    cmd->left = left;
    cmd->right = right;
    return (Cmd_t)cmd;
}

/*
 * Exercise 1:
 * In the ast.c, we have given the construction method of leaf nodes(Cmd_t Cmd_Atom_new(struct node *))
 * and a construction method of internal nodes(Cmd_t Cmd_Seq_new(struct Cmd_t *left, struct Cmd_t *right)).
 * Hint: you can refer to the realization of the function Cmd_Seq_new above or data structure defined
 * in the ast.h.
 */
Cmd_t Cmd_Back_new(Cmd_t back){
    /*
     * Todo("Exercise 1: try to complete the lost part of this function.")
     */
    Cmd_Back cmd;
    NEW(cmd);
    cmd->type = CMD_BACK;
    cmd->back = back;
    return (Cmd_t)cmd;
}

Cmd_t Cmd_Pipe_new(Cmd_t left, Cmd_t right){
    /*
     * Todo("Exercise 1: try to complete the lost part of this function.")
     */
    Cmd_Pipe cmd;
    NEW(cmd);
    cmd->type = CMD_PIPE;
    cmd->left = left;
    cmd->right = right;
    return (Cmd_t)cmd;
}

Cmd_t Cmd_Redir_new(Cmd_t left, Cmd_t right, int fd){
    /*
     * Todo("Exercise 1: try to complete the lost part of this function.")
     */
    Cmd_Redir cmd;
    NEW(cmd);
    cmd->type = CMD_REDIR;
    cmd->left = left;
    cmd->right = right;
    cmd->fd = fd;
    return (Cmd_t)cmd;
}


Cmd_t Cmd_Atom_new(struct node *node){
    Cmd_Atom cmd;
    NEW(cmd);
    cmd->type = CMD_ATOM;
    cmd->node = node;
    return (Cmd_t)cmd;
}

/*
 * Exercise 2:
 * The function of the void Cmd_print(Cmd_t cmd) is to print out the commands entered by the user to check
 * whether the abstract syntax tree(AST) of the Step 2 is constructed correctly.
 */
void Cmd_print(struct Cmd_t *cmd){
    switch(cmd->type){
        case CMD_ATOM: {
            struct Cmd_Atom  *t = (struct Cmd_Atom *) cmd;
            struct node *node = t->node;
            while (node) {
                printf("%s ", node->data);
                node = node->next;
            }
            break;
        }

        case CMD_SEQ: {
//            printf("enter\n");
            struct Cmd_Seq *t = (struct Cmd_Seq *) cmd;
//            printf("enter\n");
            Cmd_t left = t->left;
            Cmd_t right = t->right;

            Cmd_print(left);
            printf("; ");
            Cmd_print(right);
            break;
        }

        case CMD_BACK: {
            /*
             * Todo("Exercise 2: try to complete the lost part of this function.")
             */
            struct Cmd_Back *t = (struct Cmd_Back *) cmd;
            Cmd_t back = t->back;
            Cmd_print(back);
            printf("&");
            break;
        }

        case CMD_PIPE: {
            /*
             * Todo("Exercise 2: try to complete the lost part of this function.")
             */
            struct Cmd_Pipe *t = (struct Cmd_Pipe *)cmd;
            Cmd_t left = t->left;
            Cmd_t right = t->right;
            Cmd_print(left);
            printf("| ");
            Cmd_print(right);
            break;
        }

        case CMD_REDIR: {
            /*
             * Todo("Exercise 2: try to complete the lost part of this function.")
             */
            struct Cmd_Redir *t = (struct Cmd_Redir *)cmd;
            Cmd_t left = t->left;
            Cmd_t right = t->right;
            int fd = t->fd;
            Cmd_print(left);
            if (fd == 0) {
                printf("< ");
            } else {
                printf("> ");
            }
            Cmd_print(right);
            break;
        }

        default:
            break;
    }
}

/*
 * Exercise 3:
 * Please complete the void Cmd_run(Cmd_t cmd). Regarding redirection commands, you only need to implement
 * output redirection. Commands like this: cat hello.txt > c.txt
 */
void Cmd_run(struct Cmd_t *cmd){
        switch(cmd->type){
            case CMD_ATOM: {
                char *arg[10] = {0};
                int i = 0;
                struct Cmd_Atom  *t = (struct Cmd_Atom *) cmd;
                struct node *node = t->node;
                while(node){
                    arg[i] = node->data;
                    node = node->next;
                    i++;
                }
                char *root = "/bin/";
                char binPath[50];
                strcpy(binPath, root);
                strcat(binPath, arg[0]);
                if(execv(binPath, arg) == -1){
                    char *path= "/usr/bin/";
                    char usrBinPath[50];
                    strcpy(usrBinPath, path);
                    strcat(usrBinPath, arg[0]);
                    if(execv(usrBinPath, arg) == -1){
                        fprintf(stderr,"cannot run command, check your input.\n");
                    }
                }
                break;
            }

            case CMD_SEQ: {
                struct Cmd_Seq *t = (struct Cmd_Seq *) cmd;
                Cmd_t left = t->left;
                Cmd_t right = t->right;

                if(fork() == 0){
                    Cmd_run(left);
                }
                wait(0);
                Cmd_run(right);
                break;
            }

            case CMD_BACK: {
                /*
                 * Todo("Exercise 3: try to complete the lost part of this function.")
                 */
                struct Cmd_Back *t = (struct Cmd_Back *)cmd;
                Cmd_t back = t->back;
                signal(SIGCHLD,SIG_IGN);
                if (fork() == 0) {
                    Cmd_run(back);
                } else {
                    exit(0);
                }
                break;
            }

            case CMD_PIPE: {
                /*
                 * Todo("Exercise 3: try to complete the lost part of this function.")
                 */
                struct Cmd_Pipe *t = (struct Cmd_Pipe *)cmd;
                Cmd_t left = t->left;
                Cmd_t right = t->right;
                int p[2];
                pipe(p);
                if (fork() == 0) {
                    close(p[0]);
                    dup2(p[1], 1);
                    close(p[1]);
                    Cmd_run(left);
                } else {
                    close(p[1]);
                    wait(0);
                    dup2(p[0], 0);
                    close(p[0]);
                    Cmd_run(right);
                }
                break;
            }

            case CMD_REDIR: {
                /*
                 * Todo("Exercise 3: try to complete the lost part of this function.")
                 */
                struct Cmd_Redir *t = (struct Cmd_Redir *)cmd;
                Cmd_t left = t->left;
                Cmd_t right = t->right;
                int fd = t->fd;
                Cmd_Atom file = (Cmd_Atom)(t->right);
                char *filename = file->node->data;
                int p[2];
                pipe(p);
                if (fd == 1) {
                    if (fork() == 0) {
                        close(p[0]);
                        dup2(p[1], 1);
                        close(p[1]);
                        Cmd_run(left);
                    } else {
                        wait(0);
                        close(p[1]);
                        int fd_right = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0666);
                        char buf[512] = {0};
                        int n = read(p[0], buf, 512);
                        if (n == -1) {
                            perror("read error");
                            exit(1);
                        } else {
                            int written_num = write(fd_right, buf, n);
                            if (written_num == -1) {
                                perror("write error");
                                exit(0);
                            }
                        }
                        close(p[0]);
                        close(fd_right);
                    }
                    break;
                } else if (fd == 0){
                    int fd_right = open(filename, O_RDONLY);
                    if (fd_right == -1) {
                        perror("No such file in '<' operation \n");
                        exit(1);
                    }
                    char buf[512] = {0};
                    int n = read(fd_right, buf, 512);
                    if (n == -1) {
                        perror("read error");
                        exit(1);
                    }
                    int written_num = write(p[1], buf, n);
                    if (written_num == -1) {
                        perror("write error");
                        exit(1);
                    }
                    if (fork() == 0) {
                        close(p[1]);
                        dup2(p[0], 0);
                        close(p[0]);
                        Cmd_run(left);
                    } else {
                        close(p[0]);
                        close(p[1]);
                    }
                    close(fd_right);
                    break;
                }
            }
            default:
                break;
        }
    exit(0);
}
