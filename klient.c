#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

#define MAX 256 //maksymalna dlugosc komunikat
#define MSGMNB "/proc/sys/kernel/msgmnb"//sciezka do pliku z maksymalna liczba bajtow kolejki

struct msqid_ds check;

void *send_komunik();
void *rec_komunik();

typedef struct komunikat//struktura komunikatu
{
	long typ;
	long nadawca;
	char wiad[MAX];
}komunik;

komunik kom1, kom2;
char wiadomosc[MAX];
int kol_id;
int i=0;

int main()
{
	key_t klucz;
	pthread_t tid1, tid2;

	klucz=ftok(".",'K');//tworzenie klucza dla kolejki
	if(klucz==-1)
	{
		printf("Blad funkcji ftok (K)\n");
		exit(1);
	}

	kol_id=msgget(klucz,IPC_CREAT|0666);//uzyskiwanie dostepu do kolejki
	if(kol_id==-1)
	{
		perror("Nie uzyskano dostępu dop kolejki (K)");
		exit(1);
	}
	printf("Uzyskano dostep do kolejki, ID kolejki wynosi %d (K)\n", kol_id);

	if(pthread_create(&tid1,NULL,rec_komunik,NULL)==-1)//tworzenie watku do odbierania komunikatu
	{
		perror("Blad tworzenia watku do odbierania komunikatu (K)");
		pthread_exit((void*)0);
	}

	if(pthread_create(&tid2,NULL,send_komunik,NULL)==-1)//tworzenie watku do wysylania komunikatu
	{
		perror("Blad tworzenia watku do wysylania komunikatu (K)");
		pthread_exit((void*)0);
	}

	if(pthread_join(tid1,NULL)==-1)//przylaczanie watku
	{
		perror("Blad przylaczenia watku (K)");
		pthread_exit((void*)0);
	}

	if(pthread_join(tid2,NULL)==-1)//przylaczanie watku
	{
		perror("Blad przylaczenia watku (K)");
		pthread_exit((void*)0);
	}
return 0;
}

void *send_komunik()
{
	unsigned int msgmnb;
		FILE *f=fopen(MSGMNB,"r");
		fscanf(f,"%u",&msgmnb);//pobranie wartosci maksymalnej liczby bajtow w kolejce
		fclose(f);
	while(1)
	{
		kom2.typ=1;
		kom2.nadawca=getpid();

		sleep(1);

		//wprowadzenie wiadomosci do tablicy
		printf("\nKliencie o PIDzie %d podaj tekst do wyslania: ", getpid());
		i=0;
		while(1)
		{
			wiadomosc[i]=getchar();
			if(wiadomosc[i]=='\n')
			{
				wiadomosc[i]='\0';
				break;
			}
			i++;
		}
		if(i>MAX-1)
		{
			printf("Wiadomosc jest za dluga, nie zostanie wyslana\n");
			continue;
		}

		msgctl(kol_id,IPC_STAT,&check);
		int zmienna = strlen(wiadomosc);
		
		if(check.__msg_cbytes+zmienna+sizeof(long)+1+MAX+sizeof(long)>msgmnb)//sprawdzenie czy po wyslaniu komunikatu w tablicy zostanie miejsce na komunikat z serwera
		{
			printf ("Blad, przepelnienie kolejki\n");
			continue;
		}

		strcpy(kom2.wiad,wiadomosc);

		//wysylanie wiadomosci do kolejki
		if(msgsnd(kol_id,(komunik*)&kom2,strlen(kom2.wiad)+1+sizeof(long),0/*IPC_NOWAIT*/)==-1)
		{
			perror("Blad wysylania komunikatu (K)");
			exit(1);
		}
		printf("Komunikat \"%s\" zostal wyslany do serwera przez proces o PID %d\n", kom2.wiad, getpid());
	}
}

void *rec_komunik()
{
	while(1)
	{
		kom1.typ=getpid();
		memset(kom1.wiad,0,MAX);
		if(msgrcv(kol_id,(komunik*)&kom1,MAX+sizeof(long),kom1.typ,0)==-1)//odbieranie informacji z kolejki
		{
			perror("Blad pobierania komunikatu (K)");
			exit(1);
		}
		printf("Komunikat \"%s\" zostal odebrany przez klienta o PID %d \n",kom1.wiad,getpid());
	}
}