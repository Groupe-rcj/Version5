#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<ctype.h>
#include<sys/ioctl.h>
#include<termios.h>
#include<sys/select.h>
#include<sys/time.h>
#include"../en_tete/define.h"
#include"../en_tete/interface.h"

void print_temperature_pattern(int y , int x )
{
    printf("\033[%d;%dH     ┌───┐\033[%d;%dH──── │   │\033[%d;%dH  ── │   │\033[%d;%dH──── │   │\033[%d;%dH  ── │   │\033[%d;%dH──── │   │\033[%d;%dH  ── │   │\033[%d;%dH──── │   │\033[%d;%dH  ── │   │\033[%d;%dH   ┌─┘   └─┐\033[%d;%dH   │       │\033[%d;%dH   │       │\033[%d;%dH   └───────┘" , 
    y , x , y+1 , x , y+2 , x , y+3 , x , y+4 , x , y+5 , x , y+6 , x , y+7 , x , y+8 , x , y+9 , x , y+10 , x , y+11 , x , y+12 , x ) ;
}


void print_battery_pattern(int x, int y)
{
    printf("\033[%d;%dH   ┌──┐\033[%d;%dH┌──┘  └──┐\033[%d;%dH│        │\033[%d;%dH│        │\033[%d;%dH│        │\033[%d;%dH│        │\033[%d;%dH│        │\033[%d;%dH│        │\033[%d;%dH└────────┘"
    , y , x , y+1 , x, y+2 , x, y+3 , x, y+4 , x, y+5 , x, y+6 , x, y+7 , x, y+8 , x);
}

void print_readme_file()
{
    char entry ;
    int largeur ,hauteur , y=1;
    get_screen_size(&largeur , &hauteur);
    char home[100] ;
    getcwd(home , sizeof(home)) ;
    printf("%s" , home);
    FILE* fichier=fopen("../donnees/README" , "r") ;
    if(fichier==NULL)
    {
        printf("Fichier de documentation inaccessible\n") ;
        return;
    }
    char line[512] ;
    efface_ecran() ;
    while(fgets(line , 512 , fichier)!=NULL)
    {
        printf("%s" , line);
        y++ ;
        if(y==hauteur-2)
        {
            scanf("%c" , &entry) ;
            if(entry=='\n')
            {
                efface_ecran() ;
                y=1 ;
            }
        }
    }
    fclose(fichier);
}

void get_array_size(int *largeur , int *decalage)
{
    int hauteur ; 
    get_screen_size(largeur, &hauteur) ;
    *decalage=(*largeur)/13 ;
    *largeur=(*decalage)*13 ;  
}

int is_number(char* name)
{
    int i=0 ; 
    while(name[i]!='\0')
    {
        if(isdigit(name[i])==0)
        {
            return 0 ; 
        }
        i++;
    }
    return 1 ;
}

int kbhit(void)
{
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);//attente d'evenment non bloquant
}

//configuration du terminal pour qu'il soit en mode canonique cad n'attend pas d'entrée apres la saisie de caractere
void configuration()
{
	struct termios nouv_param ;   //structure qui definit l'etat du termianl actuel
	tcgetattr(0,&nouv_param );    //prend les parametres actuels du terminal 
	nouv_param.c_lflag &= ~(ICANON | ECHO ) ;     //desactive le mode canonique et echo (affichage )
	tcsetattr(0,TCSANOW ,&nouv_param);    //appliquer les changements maaintenant  
}

//restauration des parametre du terminal
void reset()
{
	struct termios nouv_param;
	tcgetattr(0,&nouv_param);
	nouv_param.c_lflag |= (ICANON | ECHO ) ; 
	tcsetattr(0,TCSANOW,&nouv_param) ;
}

//prise des caracteres entrées au clavier
int get_char() 
{
	char seq[3];
    int flags=fcntl(0 , F_GETFL , 0 ) , n ;
    fcntl(0 , F_SETFL , flags | O_NONBLOCK ) ;
    n=read(0, &seq[0] , 1 ) ; 
    fcntl(0 , F_SETFL , flags ) ;
    //on lit les caractères entrées séquence par séquence
	if(n != 1 ) 
    {
        return 0 ;
    } 
    // s'il s'agit d'un caractere d'echappement 
	if(seq[0] == '\033' ) 
    {
    //on lit les caracteres suivants toujours par séquence
		if( read(0 , &seq[1] , 1 ) != 1 ) 
        {
            return 0 ;
        }
		if( read(0 , &seq[2] , 1 ) != 1 ) 
        {
            return 0 ; 
        }
		if(seq[1] == '[') 
        {
			switch (seq[2]) 
            {
				case 'A' : return 'U' ;  //fleche haut
				case 'B' : return 'D' ;  //fleche bas
				case 'C' : return 'R' ;  //fleche droite
				case 'D' : return 'L' ;  //fleche gauche
			}
		}
		return '\033'; // autre caractere d'echappement
	}
	else
    {
		return seq[0] ; //caractere normal
    }
    return 0 ; 
}

void print_left_arrow(int y , int x , char*en_tete)
{
    printf("\033[%d;%dH%s" , y-1 , x , en_tete) ;
    printf("\033[%d;%dH╱│" , y,x+2);
    printf("\033[%d;%dH╱ │" , y+1,x+1);
    printf("\033[%d;%dH╱  └────┐" , y+2,x);
    printf("\033[%d;%dH╲  ┌────┘" ,y+3,x);
    printf("\033[%d;%dH╲ │" ,y+4,x+1);
    printf("\033[%d;%dH╲│" , y+5,x+2);
}

void print_right_arrow(int y , int x , char*en_tete)
{
    printf("\033[%d;%dH%s" , y-1 , x , en_tete) ;
    printf("\033[%d;%dH│╲" , y,x+6);
    printf("\033[%d;%dH│ ╲" ,y+1,x+6);
    printf("\033[%d;%dH┌─────┘  ╲" ,y+2,x);
    printf("\033[%d;%dH└─────┐  ╱" ,y+3,x);
    printf("\033[%d;%dH│ ╱" ,y+4,x+6);
    printf("\033[%d;%dH│╱" ,y+5,x+6);
}


char*** malloc_3D()
{
    int i , j ;
    char ***pattern_number=NULL ;
    pattern_number=malloc(10*sizeof(char**)) ;
    for( i=0 ; i<10 ; i++ )
    {
        pattern_number[i]=malloc(7*sizeof(char*)) ;
    }
    for( i=0 ; i<10 ; i++ )
    {
            for( j=0 ; j<7 ; j++ )
            {
                pattern_number[i][j]=malloc(31*sizeof(char)) ;
            }
    }
    return pattern_number ;
}

void initiate_pattern(char**pattern ,  char* ligne1 , char* ligne2 , char* ligne3 , char* ligne4 , char* ligne5 , char* ligne6 , char* ligne7 )
{
    strncpy(pattern[0] , ligne1 , 30 ); 
    pattern[0][30] = '\0' ;
    strncpy(pattern[1] , ligne2 , 30 ); 
    pattern[1][30] = '\0' ;
    strncpy(pattern[2] , ligne3 , 30 ); 
    pattern[2][30] = '\0' ;
    strncpy(pattern[3] , ligne4 , 30 ); 
    pattern[3][30] = '\0' ;
    strncpy(pattern[4] , ligne5 , 30 ); 
    pattern[4][30] = '\0' ;
    strncpy(pattern[5] , ligne6 , 30 ); 
    pattern[5][30] = '\0' ;
    strncpy(pattern[6] , ligne7 , 30 ); 
    pattern[6][30] = '\0' ;
}

char*** get_pattern()
{
    char*** pattern_number=NULL ; 
    pattern_number=malloc_3D();
    initiate_pattern(pattern_number[0] , "┌───────┐" ,"│  ┌─┐  │","│  │ │  │","│  │ │  │","│  │ │  │","│  └─┘  │","└───────┘");
    initiate_pattern(pattern_number[1] , "┌──┐","│  │","│  │","│  │","│  │","│  │","└──┘");
    initiate_pattern(pattern_number[2] , "┌───────┐" ,"│  ┌─┐  │","└──┘ │  │","┌────┘  │","│  ┌────┘","│  └────┐","└───────┘ " );
    initiate_pattern(pattern_number[3] , "┌───────┐","└────┐  │","  ┌──┘  │","  └──┐  │","     │  │","┌────┘  │","└───────┘");
    initiate_pattern(pattern_number[4] , "┌──┐","│  │","│  │┌─┐ ","│  └┘ └─┐","└───┐ ┌─┘","    │ │","    └─┘"); 
    initiate_pattern(pattern_number[5] , "┌───────┐", "│  ┌────┘","│  │","│  └────┐","└─────┐ │","┌─────┘ │","└───────┘");
    initiate_pattern(pattern_number[6] , "┌───────┐" ,"│  ┌────┘","│  │ ","│  └────┐","│  ┌──┐ │", "│  └──┘ │", "└───────┘");
    initiate_pattern(pattern_number[7] , "┌───────┐" , "│  ┌─┐  │","│  │ │  │","└──┘ │  │","     │  │","     │  │","     └──┘" );
    initiate_pattern(pattern_number[8] , "┌───────┐" , "│  ┌─┐  │" , "│  └─┘  │" ,"│       │","│  ┌─┐  │" , "│  └─┘  │" , "└───────┘") ; 
    initiate_pattern(pattern_number[9] , "┌───────┐" , "│  ┌─┐  │" , "│  └─┘  │" , "└─────┐ │" , "      │ │" , "      │ │" , "      └─┘" );
    return pattern_number ;
}

void print_point(int y , int x )
{
    bouge_curseur(y,x);
    printf("┌─┐");
    bouge_curseur(y+1,x);
    printf("└─┘") ;
}
void print_percent(int y , int x)
{
    print_point(y ,  x ) ; 
    print_point(y+2 ,  x+5 ) ; 
    bouge_curseur(y,x+4) ; 
    printf("╱ ╱") ;
    bouge_curseur(y+1,x+3) ;
    printf("╱ ╱") ;
    bouge_curseur(y+2,x+2) ;
    printf("╱ ╱") ;  
    bouge_curseur(y+3,x+1) ;
    printf("╱ ╱") ;
}

void get_screen_size(int* largeur , int* hauteur)
{
	struct winsize ecran ; 
	ioctl( STDOUT_FILENO , TIOCGWINSZ , &ecran ) ; 
	*largeur=ecran.ws_col ; 
	*hauteur=ecran.ws_row ; 
}

void efface_ecran()
{
	printf("\033[H\033[J") ; 
}

void bouge_curseur( int ligne , int colonne )
{
	printf("\033[%d;%dH" , ligne , colonne) ; 
}

void affiche_au_centre( int hauteur , char* texte , const char* couleur )
{
	int centre_x , largeur_ecran , hauteur_ecran ;
	get_screen_size(&largeur_ecran , &hauteur_ecran) ;  
	centre_x = (largeur_ecran- strlen(texte)) / 2 ;
	bouge_curseur( hauteur , centre_x ) ;
	printf("%s%s%s" , couleur , texte , NEUTRE) ;  
}

void print_colored(number *num , char type ,  int max , int middle , const char* color_max , const char* color_middle , const char* color_min)
{
    if(num==NULL)
    {
        printf("pointeur null\n");
        return ;  
    }
    switch(type)
    {
        case 'l' :
        {
            if(num[0].lo>=max)
            {
                printf("%s%.2Lf "NEUTRE , color_max , num[0].lo) ; 
            }
            else if(num[0].lo>=middle)
            {
                printf("%s%.2Lf "NEUTRE , color_middle , num[0].lo) ; 
            }
            else
            {
                printf("%s%.2Lf "NEUTRE , color_min , num[0].lo) ; 
            }
        }
        break ; 
    }
}

void print_array(int ligne , int colonne , int espacement)
{
    int i , j ; 
    for(i=1 ; i<=colonne ; i++)
    {
        for(j=1 ; j<=ligne ; j++)
        {
            bouge_curseur(j , i) ;
            if( i%espacement==0 )
            {
                printf(BARRE_VERTICALE_EP) ; 
            }
            else if(i==1)
            {
                printf(GAUCHE) ;
            }
            else if(i==colonne)
            {
                printf(DROITE) ;
            }
            else
            {
                j++ ; 
                printf(BARRE_HORIZONTALE) ;
            }
        }
    } 
    bouge_curseur( 1 , 1 ) ;
    printf(BARRE_SUP_GAUCHE) ; 
    bouge_curseur( 1 , colonne) ;
    printf(BARRE_SUP_DROITE) ; 
    bouge_curseur( ligne , 1 ) ;
    printf(BARRE_INF_GAUCHE) ; 
    bouge_curseur( ligne , colonne) ;
    printf(BARRE_INF_DROITE) ; 

}

void dessine_boite(int x , int y , int hauteur , int largeur , char* color ) 
{
	int i , largeur_ecran , hauteur_ecran ;   
 	get_screen_size( &largeur_ecran , &hauteur_ecran) ; 
	bouge_curseur(y , x ) ; 
	printf( "%s%s%s" , color , BARRE_SUP_GAUCHE , NEUTRE ) ;
	bouge_curseur(y , x+1 ) ; 
 	for( i=1 ; i<=largeur ; i++ )	
	{
		printf("%s%s%s" , color ,  BARRE_HORIZONTALE , NEUTRE) ;  
	}
	bouge_curseur(y , x+largeur ) ; 
	printf( "%s%s%s" , color , BARRE_SUP_DROITE , NEUTRE ) ;

 	for( i=1 ; i<hauteur ; i++ )	
	{
		bouge_curseur(y+i , x ) ; 
		printf("%s%s%s" , color ,  BARRE_VERTICALE , NEUTRE) ; 
		bouge_curseur(y+i , x+largeur) ; 
		printf("%s%s%s" , color ,  BARRE_VERTICALE , NEUTRE) ; 
	}

	bouge_curseur(y+hauteur , x ) ; 
	printf( "%s%s%s" , color , BARRE_INF_GAUCHE , NEUTRE ) ;
	bouge_curseur(y+hauteur , x+1 ) ; 
	for( i=1 ; i<=largeur; i++ )	
	{
	    printf("%s%s%s" , color ,  BARRE_HORIZONTALE , NEUTRE) ;
	}
	bouge_curseur(y+hauteur , x+largeur ) ; 
	printf( "%s%s%s" , color , BARRE_INF_DROITE , NEUTRE ) ;
}

