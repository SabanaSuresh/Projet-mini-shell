/**************************************/
/*            minishell.c             */
/**************************************/

#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "analex.h"

#define TAILLE_MAX 100 /* taille max d'un mot */
#define ARGS_MAX 10    /* nombre maximum d'arguments a une commande */

/* Execute la commande donnee par le tableau argv[] dans un nouveau processus ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande,
 * a rediriger eventuellement.
 * Renvoie le numero du processus executant la commande
 *
 * FONCTION À MODIFIER/COMPLÉTER.
 *
*/
pid_t execute(int entree, int sortie, char* argv[], int background,int* status) {
  
  pid_t pid;
  struct sigaction sig;
  int out_background;

  // Désactive l'interruption par Ctrl+c
    sig.sa_flags = 0;
    sig.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sig, NULL);
 
    pid = fork();
    if(pid < 0)
    {
            // La création de processus à échoué
        return -1;
    }
 
    if(pid==0)
    {
        // Réactive l'interruption par Ctrl+c
        sig.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sig, NULL);
 
        // Lancement de l'exécution du programme
        if(background == 1 || background == 7 || background == 8 ){
          out_background = open("/dev/null", O_WRONLY);
          dup2(out_background, STDOUT_FILENO);
          if(sortie != 1 && sortie != 2){
            dup2(sortie, out_background);
          }
          if(background == 7){
            close(entree);
            dup2(sortie, STDOUT_FILENO);
          }
          if(background == 8){
            close(sortie);
            dup2(entree, STDIN_FILENO);
          }
        }
        if(sortie != 1 && sortie != 2 && background != 5 && background != 6  && background != 7 && background != 8){
          dup2(sortie, STDOUT_FILENO);
        }
        if(background == 5){
          close(entree);
          dup2(sortie, STDOUT_FILENO);
        }
        if(background == 6){
          close(sortie);
          dup2(entree, STDIN_FILENO);
        }
        if(entree != 0 && background != 5 && background != 6  && background != 7 && background != 8){
          dup2(entree, STDIN_FILENO);
        }

        execvp(argv[0],argv); // exécution de la commande

        if(sortie != 1 && sortie != 2 && background != 5 && background != 7){
          close(sortie);
          dup2(STDOUT_FILENO,1);
        }
        if(background == 1 || background == 8){
          if(sortie != 1 && sortie != 2){
            dup2(out_background, sortie);
            close(sortie);
          }
          close(out_background);
          dup2(STDOUT_FILENO,1);
        }
        // Affichage en cas d'echec
        printf("Impossible d'executer %s %s\n",argv[0],strerror(errno)); 
        exit(1);
    }
    else
    {
      if(background == 0 || background == 6 || background == 5){
        // On bloque pas le processus parent jusqu'a ce que l'enfant termine puis
        // on kill le processus enfant
        waitpid(pid, status, 0);
        if( background == 5){
          close(sortie);
        }
        if(entree != 0 && background != 5 && background != 7){
          dup2(STDIN_FILENO,0);
          close(entree);
        }
        kill(pid, SIGTERM);
      }
      else if(background == 1 || background == 8 || background == 7){
        waitpid(pid, status, WNOHANG);
        if(background == 1 || background == 8){
          if(sortie != 1 && sortie != 2){
            close(sortie);
          }
        }
        if(entree != 0 && background != 5 && background != 7){
          dup2(STDIN_FILENO,0);
          close(entree);
        }
      }
    }
  return pid; 
}

/* Lit et execute une commande en recuperant ses tokens ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande ;
 * pid contient le numero du processus executant la commande et
 * background est non nul si la commande est lancee en arriere-plan
 *
 * FONCTION À MODIFIER/COMPLÉTER.
*/
TOKEN commande(int entree, int sortie, pid_t* pid, int* background, int* status) {

  /* ... */
  TOKEN t; 
  static int ind1 = 0;
  static int ind2 = 0;
  char word[ARGS_MAX][TAILLE_MAX];
  char **tabArgs = NULL;

  tabArgs = (char **)calloc(ARGS_MAX,sizeof(char*));

  if (tabArgs == NULL) {
    perror("Malloc failure");
    return (EXIT_FAILURE);
  }

  while(1){
    t = getToken(word[ind2++]);
    if(t == T_WORD){
      tabArgs[ind1++] = word[ind2-1];
    }
    else if(t == T_AMPER){
      if(*background == 5){
        *background = 7;
      }
      else if(*background == 6){
        *background = 8;
      }
      else if(*background == 0){
        *background = 1;
      }
    }
    else if(t == T_GT){
      getToken(word[ind2++]);
      sortie = creat(word[ind2-1], 0642);
    }
    else if(t == T_GTGT){
      getToken(word[ind2++]);
      sortie = open(word[ind2-1],O_WRONLY|O_APPEND);
    }
    else if(t == T_LT){
      getToken(word[ind2++]);
      entree = open(word[ind2-1],O_RDONLY);
      FILE* file = fopen(word[ind2-1], "r"); 
      if(file == NULL){
        return EXIT_FAILURE;       
      }
      while(fscanf(file, "%99s", word[ind2]) > 0){
        tabArgs[ind1++]=word[ind2++];
      }
      fclose(file);
    }
    else if(t == T_NL && *background != 5 && *background != 6 && *background != 7 && *background != 8){
      tabArgs[ind1] = NULL;
      *pid = execute(entree,sortie,tabArgs,*background,status);
      tabArgs = NULL;
      ind1 = 0;
      ind2 = 0;
      *background=0;
      return T_NL;
    }
    else if(t == T_SEMI){
      tabArgs[ind1] = NULL;
      *pid = execute(entree,sortie,tabArgs,*background,status);
      tabArgs = NULL;
      ind1 = 0;
      ind2 = 0;
      *background=0;
      return T_SEMI;
    }
    else if(t == T_BAR || (t == T_NL && (*background == 6 || *background == 8)) ){
      static int fd[2];
      if(*background != 6 && *background != 8 && *background != 5 && *background != 7){
        pipe(fd);
        tabArgs[ind1] = NULL;
        *background=5;
        *pid = execute(fd[0],fd[1],tabArgs,*background,status);
        tabArgs = NULL;
        ind1 = 0;
        ind2 = 0;
        *background=6;
        commande(fd[0],fd[1],pid, background, status);
      }
      else if((*background == 6 || *background == 8) && t == T_NL){
        tabArgs[ind1] = NULL;
        *pid = execute(fd[0],fd[1],tabArgs,*background,status);
        tabArgs = NULL;
        ind1 = 0;
        ind2 = 0;
        *background=0;
      }
    }
    else if(t == T_EOF){
      return T_EOF;
    }
  }
}

/* Retourne une valeur non-nulle si minishell est en train de s'exécuter en mode interactif, 
 * c'est à dire, si l'affichage de minishell n'est pas redirigé vers un fichier.
 * (Important pour les tests automatisés.)
 *
 * NE PAS MODIFIER CETTE FONCTION.
 */
int is_interactive_shell(){
  return isatty(1);
}

/* Affiche le prompt "minishell>" uniquement si l'affichage n'est pas redirigé vers un fichier. 
 * (Important pour les tests automatisés.)
 *
 * NE PAS MODIFIER CETTE FONCTION.
 */
void print_prompt(){
  if(is_interactive_shell()){ 
    printf("mini-shell>"); 
    fflush(stdout);
  }  
}

/* Fonction main 
 * FONCTION À MODIFIER/COMPLÉTER.
 */
int main(int argc, char* argv[], char** envp) {
	TOKEN t;
	pid_t pid /*fid*/;
	int background = 0;
	int status = 0;
	print_prompt(); // affiche le prompt "minishell>" 
	
	while ( (t = commande(0, 1, &pid, &background,&status)) != T_EOF) {

	  /* Boucle principale: à modifier/compléter */	

	  
	  if (t == T_NL) {
	    print_prompt(); 
	  }
	}

	if(is_interactive_shell())
	  printf("\n") ; 
	return WEXITSTATUS(status); // Attention à la valeur de retour du wait, voir man wait et la macro WEXITSTATUS
}