#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pwd.h>
#include<time.h>
#include<dirent.h>
#include<pthread.h>
#include<stdatomic.h>
#include<ctype.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include"../en_tete/define.h"
#include"../en_tete/interface.h"
#include"../en_tete/network.h"
#include"../en_tete/ram.h"

extern atomic_int run_network , run_connexion, run_sniff;
FILE* open_file(char* filename , int mode)
{
    FILE* fichier=NULL ; 
    if( mode==0 )
    {
        fichier=fopen(filename , "r") ; 
    }
    else if( mode==1 )
    {
        fichier=fopen(filename , "w") ; 
    }
    if(fichier==NULL)
    {
        printf("Couldn't open file %s\n" , filename) ; 
        exit(1) ; 
    }
    return fichier ; 
}

char* search_process_owner(int fd)
{
    DIR* proc_directory , *fd_directory ;
    struct dirent *entry1 , *entry2 ;
    char path[521] , target[100] , dir[266] , search[100];  
    proc_directory=opendir("/proc") ; 
    if(proc_directory==NULL)
    {
        printf("Couldn't open proc_directory\n") ;
        return "UNKNOW" ;
    }
    entry1=readdir(proc_directory) ;
    while(entry1!=NULL)
    {
        if(is_number(entry1->d_name))
        {
            sprintf(dir , "/proc/%s/fd" , entry1->d_name) ; 
            fd_directory=opendir(dir) ; 
            if(fd_directory==NULL)
            {
                continue ;
            }
            entry2=readdir(fd_directory) ;
            while(entry2!=NULL)
            {
                sprintf(path , "/proc/%s/fd/%s" , entry1->d_name , entry2->d_name) ; 
                ssize_t ln = readlink(path , target , sizeof(target)-1) ;
                if( ln <= 0)
                {
                    entry2=readdir(fd_directory) ;
                    continue ; 
                }
                target[ln]='\0';
                sprintf(search , "%d" , fd );
                if(strstr(target , search ) !=NULL)
                {
                    char file[269] , *name ;
                    name=calloc(100,sizeof(char)) ;
                    sprintf(file , "/proc/%s/status" , entry1->d_name) ;
                    FILE* fichier=fopen(file ,"r") ;
                    fscanf(fichier , "%*s %s" , name) ;
                    fclose(fichier);
                    closedir(fd_directory);
                    closedir(proc_directory) ;
                    return name ;  
                } 
                entry2=readdir(fd_directory) ;
            }
            closedir(fd_directory);
        }
        entry1=readdir(proc_directory) ;
    }
    closedir(proc_directory) ;
    return "UNKNOW" ;
}

char* hex_IPV4(char* hex , char* port )
{
    char  *result , section1[3] , section2[3] , section3[3] , section4[3] ;
    result=malloc(40) ; 
    section1[0]=hex[0] ; 
    section1[1]=hex[1] ;
    section1[2]='\0' ; 

    section2[0]=hex[2] ; 
    section2[1]=hex[3] ; 
    section2[2]='\0' ; 

    section3[0]=hex[4] ; 
    section3[1]=hex[5] ; 
    section3[2]='\0' ; 

    section4[0]=hex[6] ; 
    section4[1]=hex[7] ;
    section4[2]='\0' ; 

    port[0]=hex[9] ; 
    port[1]=hex[10] ; 
    port[2]=hex[11] ; 
    port[3]=hex[12] ; 
    port[4]='\0' ;
    
    sprintf(result , "%ld.%ld.%ld.%ld" ,
    strtol(section4 , NULL , 16 ) ,
    strtol(section3 , NULL , 16 ) , 
    strtol(section2 , NULL , 16 ) ,
    strtol(section1 , NULL , 16 ) 
    ) ;
    sprintf( port , "%ld" , strtol(port , NULL , 16 )) ; 
    return result ; 
}


int interface_number()
{
    int i=0 ; 
    FILE* fichier=open_file("/proc/net/dev" , 0) ;
    char line[256] ; 
    fgets( line , 256 ,fichier) ;
    fgets( line , 256 ,fichier) ;  
    while(fgets( line , 256 ,fichier)!=NULL)
    {
        i++ ; 
    }
    fclose(fichier) ; 
    return i ;    
}

void get_connexion_state(char* filename , int mode)
{
    struct passwd *user ; 
    FILE* fichier ; 
    connexion_info con1 ; 
    fichier=open_file(filename , 0) ; 
    int largeur , hauteur , i=5 ;
    char* line , port1[20] , port2[20];
    get_screen_size(&largeur , &hauteur) ; 
    line=malloc(256) ; 
    fgets( line , 256 ,fichier) ;

    efface_ecran() ;

    bouge_curseur(1 , (largeur/2)-strlen("Connexion par le protocole TCP")) ; 
    if(mode==1)
    {
        printf("Connexion par le protocole TCP");
    }
    else if (mode==2)
    {
        printf("Connexion par le protocole UDP");
    }
    bouge_curseur(3,1) ; 
    printf("LOCAL_IP      LOCAL_PORT   REMOTE_IP  REMOTE_PORT     RX_QUEUE  TX_QUEUE   STATUS      OWNER    INODE PROCESS_OWNER");         
    while(fgets( line , 256 ,fichier)!=NULL)
    {
        if(!atomic_load(&run_connexion))
        {
            break ;
        }
        if(sscanf(line , "%*d: %s %s %s %s %*s  %*s %d  %*s %d  %*s  %*s  %*s   %*s  %*s %*s %*s ", 
            con1.local_port_addr , 
            con1.remote_port_addr , 
            con1.status ,
            con1.rx_tx_queue , 
            &con1.uid , 
            &con1.inode
        ) == 6 )
        {
            if(strcmp(con1.status,"0A")==0 || strcmp(con1.status,"01")==0 )
            {
                bouge_curseur(i,1) ; 
                user=getpwuid(con1.uid) ; 
                if(user->pw_name==NULL)
                {
                    strcpy(user->pw_name , "UNKNOW") ; 
                }
                unsigned int rx , tx ;
                if(sscanf(con1.rx_tx_queue , "%x:%x" , &rx , &tx)==2)
                {
                    printf("%s  %-8s %s %-8s %d %d %ld %s %d %s" , hex_IPV4(con1.local_port_addr , port1 ) ,port1 , hex_IPV4(con1.remote_port_addr , port2) , port2 , rx , tx  , strtol(con1.status , NULL , 16 ) , user->pw_name , con1.inode , search_process_owner(con1.inode));         
                }
                else
                {
                    printf("%s %-8s %s %-8s %-5s %ld %s %d %s" , hex_IPV4(con1.local_port_addr , port1 ) , port1 , hex_IPV4(con1.remote_port_addr , port2) , port2 , con1.rx_tx_queue , strtol(con1.status , NULL , 16 ) , user->pw_name , con1.inode  , search_process_owner(con1.inode));         
                }
                i++ ;
            }

            if(i>hauteur)
            {
                break ; 
            } 
        }
        fflush(stdout);
        usleep(500000);
    }
    fclose(fichier) ; 
    free(line) ; 
}


void print_device_head(int largeur , int x , int y )
{
    int interf=interface_number() , espacement ;
    espacement=largeur/9 ; 
    dessine_boite(x , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+espacement , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(2*espacement) , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(3*espacement) , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(4*espacement)  , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(5*espacement)  , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(6*espacement)  , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(7*espacement)  , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;
    dessine_boite(x+(8*espacement)  , y , (2*(interf+2))+3 , espacement-1 , ROUGE ) ;

    printf("\033[%d;%dHINTERFACE" , y+2 , x+1 );
    printf("\033[%d;%dHRbytes" , y+2 , x+espacement+1);
    printf("\033[%d;%dHRpackets" , y+2 , x+(2*espacement)+1);
    printf("\033[%d;%dHRerrs" , y+2 , x+(3*espacement)+1);
    printf("\033[%d;%dHRDrop" , y+2 , x+(4*espacement)+1);
    printf("\033[%d;%dHTbytes(B/s)" , y+2 , x+(5*espacement)+1);
    printf("\033[%d;%dHTpackets(P/s)" , y+2 , x+(6*espacement)+1);
    printf("\033[%d;%dHTerrs" , y+2 , x+(7*espacement)+1);
    printf("\033[%d;%dHTDrop" , y+2 , x+(8*espacement)+1);

    printf("\033[%d;%ldHEVERY SECOND" , y , (largeur/2)-(strlen("EVERY SECOND"))) ;

    printf("\033[%d;%ldHFROM THE START" , interf+4 , (largeur/2)-(strlen("FROM THE START"))) ;
    fflush(stdout) ;
}

void print_correctly(unsigned long int info , int y, int x )
{
    printf("\033[%d;%dH         " , y , x ) ;
    if(info<1024)
    {
        printf("\033[%d;%dH%lu o" , y , x , info) ;
        return ;
    }
    info/=1024 ; //en ko
    if(info >= 1024)
    {
        info/=1024 ; //en Mo
        if(info >= 1024)
        {
            info/=1024 ; //en Go
            printf("\033[%d;%dH%lu Go" , y , x , info) ;
            return ;
        }
        else
        {
            printf("\033[%d;%dH%lu Mo" , y , x , info) ;
            return ; 
        }
    }
    else
    {
        printf("\033[%d;%dH%lu Ko" , y , x , info) ;
        return ;
    }


}

void print_dev(net_device_info info , int espacement , int y, int x )
{
    printf("\033[%d;%dH%s     ", y+1 , x+1 , info.dev_name);
    print_correctly( info.received_bytes, y+1, x+espacement+1  ) ;
    printf("\033[%d;%dH%lu   ", y+1 , x+(2*espacement)+1 , info.received_packets);
    printf("\033[%d;%dH%lu   ", y+1 , x+(3*espacement)+1 , info.received_erros);
    printf("\033[%d;%dH%lu   ", y+1 , x+(4*espacement)+1 , info.received_drop);
    print_correctly( info.transmitted_bytes, y+1, x+(5*espacement)+1) ;

    printf("\033[%d;%dH%lu   ", y+1 , x+(6*espacement)+1 , info.transmitted_packets);
    printf("\033[%d;%dH%lu   ", y+1 , x+(7*espacement)+1 , info.transmitted_erros);
    printf("\033[%d;%dH%lu   ", y+1 , x+(8*espacement)+1 , info.transmitted_drop);
}

void get_network_state(int from_start)
{
    int largeur , hauteur , interf=interface_number() , i ; 
    FILE* fichier ; 
    net_device_info *info1 , *info2 , *real_info ; 
    char* line ;
    get_screen_size(&largeur , &hauteur) ; 
    fichier=open_file("/proc/net/dev" , 0) ;
    if(fichier==NULL)
    {
        return;
    }
    line=malloc(256) ; 
    fgets( line , 256 ,fichier) ;
    fgets( line , 256 ,fichier) ;
    

    info1=malloc(interf*sizeof(net_device_info)) ; 
    info2=malloc(interf*sizeof(net_device_info)) ; 
    real_info=malloc(interf*sizeof(net_device_info)) ; 

    i=0 ; 
    while(fgets( line , 256 ,fichier)!=NULL)
    {
        if(!atomic_load(&run_network))
        {
            fclose(fichier) ;
            free(line) ;
            return ;
        }
        sscanf(line , "%s %lu %lu %lu %lu %*d %*d %*d %*d %lu %lu %lu %lu %*d %*d %*d %*d", 
        info1[i].dev_name, 
        &info1[i].received_bytes , 
        &info1[i].received_packets , 
        &info1[i].received_erros ,
        &info1[i].received_drop , 
        &info1[i].transmitted_bytes , 
        &info1[i].transmitted_packets , 
        &info1[i].transmitted_erros , 
        &info1[i].transmitted_drop 
        );
        i++ ; 
    }
    fclose(fichier) ;
    
    for (int s = 0; s < 10 && atomic_load(&run_network); ++s) 
    {
        usleep(100000);
    }

    fichier=open_file("/proc/net/dev" , 0) ;
    if(fichier==NULL)
    {
        return;
    }
    fgets( line , 256 ,fichier) ;
    fgets( line , 256 ,fichier) ;
    
    i=0 ;
    while(fgets( line , 256 ,fichier)!=NULL)
    {
        if(!atomic_load(&run_network))
        {
            fclose(fichier) ;
            free(line) ;
            return ;
        }
        sscanf(line , "%s %lu %lu %lu %lu %*d %*d %*d %*d %lu %lu %lu %lu ", 
        info2[i].dev_name, 
        &info2[i].received_bytes , 
        &info2[i].received_packets , 
        &info2[i].received_erros ,
        &info2[i].received_drop , 
        &info2[i].transmitted_bytes , 
        &info2[i].transmitted_packets , 
        &info2[i].transmitted_erros , 
        &info2[i].transmitted_drop 
        ) ;
        i++ ;
    }
    fclose(fichier) ;  

    for(i=0 ; i<interf ; i++)
    {
        strcpy(real_info[i].dev_name , info2[i].dev_name);  

        real_info[i].received_bytes = (info2[i].received_bytes >= info1[i].received_bytes)
            ? (info2[i].received_bytes - info1[i].received_bytes) : 0;

        real_info[i].received_packets = (info2[i].received_packets >= info1[i].received_packets)
            ? (info2[i].received_packets - info1[i].received_packets) : 0;

        real_info[i].received_erros = (info2[i].received_erros >= info1[i].received_erros)
            ? (info2[i].received_erros - info1[i].received_erros) : 0;

        real_info[i].received_drop = (info2[i].received_drop >= info1[i].received_drop)
            ? (info2[i].received_drop - info1[i].received_drop) : 0;

        real_info[i].transmitted_bytes = (info2[i].transmitted_bytes >= info1[i].transmitted_bytes)
            ? (info2[i].transmitted_bytes - info1[i].transmitted_bytes) : 0;

        real_info[i].transmitted_packets = (info2[i].transmitted_packets >= info1[i].transmitted_packets)
            ? (info2[i].transmitted_packets - info1[i].transmitted_packets) : 0;

        real_info[i].transmitted_erros = (info2[i].transmitted_erros >= info1[i].transmitted_erros)
            ? (info2[i].transmitted_erros - info1[i].transmitted_erros) : 0;

        real_info[i].transmitted_drop = (info2[i].transmitted_drop >= info1[i].transmitted_drop)
            ? (info2[i].transmitted_drop - info1[i].transmitted_drop) : 0;
        if(from_start==0)
        {
            print_dev(real_info[i] , largeur/9 , i+3 , 1) ; 
        }
        else
        {
            print_dev(info1[i] , largeur/9 , i+interf+7 , 1) ; 
        }
    }
    fflush(stdout) ; 
    free(line) ; 
}

const char *protocol_name(int proto) 
{
    switch(proto) 
    {
        case 1:  return "ICMP";
        case 2:  return "IGMP";
        case 6:  return "TCP";
        case 17: return "UDP";
        case 41: return "IPv6";
        case 58: return "ICMPv6";
        default: return "Unknown";
    }
}

/* Affiche payload en hex + ASCII comme Wireshark */
void hexdump(const unsigned char *data, int len) 
{
    int i, j;
    for (i = 0; i < len; i += 16) 
    {
        printf("  %04x  ", i);

        for (j = 0; j < 16; j++) 
        {
            if (i + j < len) printf("%02x ", data[i + j]);
            else printf("   ");
        }

        printf(" ");

        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                unsigned char c = data[i + j];
                printf("%c", isprint(c) ? c : '.');
            }
        }
        printf("\n");
    }
}


void* sniff() 
{
    int sockfd;
    unsigned char buffer[65536];

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) 
    { 
        perror("socket"); 
        exit(1); 
    }

    printf(VERT) ;
    printf(">>> Sniffer actif sur toutes interfaces...\n");
    while (atomic_load(&run_sniff)) 
    {
        ssize_t length = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (length <= 0) continue;
        struct ethhdr *eth = (struct ethhdr *) buffer;

        printf("\n==================== Paquet (%ld octets) ====================\n", length);

        printf("MAC src: %02x:%02x:%02x:%02x:%02x:%02x\n",
               eth->h_source[0], eth->h_source[1], eth->h_source[2],
               eth->h_source[3], eth->h_source[4], eth->h_source[5]);

        printf("MAC dst: %02x:%02x:%02x:%02x:%02x:%02x\n",
               eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
               eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

        printf("Ethertype: 0x%04x\n", ntohs(eth->h_proto));

        int header_size = sizeof(struct ethhdr);
        const unsigned char *payload = NULL;
        int payload_len = 0;

        /* IPv4 */
        if (ntohs(eth->h_proto) == ETH_P_IP) {
            struct iphdr *ip = (struct iphdr *) (buffer + header_size);
            struct in_addr src, dst;

            src.s_addr = ip->saddr;
            dst.s_addr = ip->daddr;

            printf("IPv4 src: %s\n", inet_ntoa(src));
            printf("IPv4 dst: %s\n", inet_ntoa(dst));
            printf("Protocol: %d (%s)\n", ip->protocol, protocol_name(ip->protocol));

            int ip_header_len = ip->ihl * 4;
            header_size += ip_header_len;

            payload = buffer + header_size;
            payload_len = length - header_size;
        }

        /* IPv6 */
        else if (ntohs(eth->h_proto) == ETH_P_IPV6) 
        {
            struct ip6_hdr *ip6 = (struct ip6_hdr *) (buffer + header_size);

            char src6[INET6_ADDRSTRLEN];
            char dst6[INET6_ADDRSTRLEN];

            inet_ntop(AF_INET6, &ip6->ip6_src, src6, sizeof(src6));
            inet_ntop(AF_INET6, &ip6->ip6_dst, dst6, sizeof(dst6));

            printf("IPv6 src: %s\n", src6);
            printf("IPv6 dst: %s\n", dst6);
            printf("Next Header: %d (%s)\n",
                   ip6->ip6_nxt, protocol_name(ip6->ip6_nxt));

            header_size += sizeof(struct ip6_hdr);
            payload = buffer + header_size;
            payload_len = length - header_size;
        }

        if (payload_len > 0) {
            printf("\n--- Payload (%d octets) ---\n", payload_len);
            hexdump(payload, payload_len);
        } else {
            printf("\n(Pas de payload)\n");
        }
    }
        
    printf(NEUTRE) ;

    close(sockfd);
    return NULL ;
}

void* run_network_track()
{
    int hauteur , largeur ;
    get_screen_size(&largeur , &hauteur) ;
    while(atomic_load(&run_network))
    {
        print_device_head(largeur , 1 , 1 ) ;
        get_network_state(0) ;
        get_network_state(1) ;
        printf("\033[%d;%dH t (TCP CONNECTION) / u (UDP CONNECTION) / q (quitter) / s (intercepter les paquets)" , hauteur-2 , 1 ) ; 
        fflush(stdout);
        for(int i=0 ; i<20 ; i++)
        {
            if(!atomic_load(&run_network))
            {
                return NULL ;
            }
            usleep(10000); // refresh toutes les 10ms
        }
    }
    return NULL ; 
}
