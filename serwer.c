#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

#define MAX 256 //maksymalna dlugosc komunikatu

typedef struct komunikat//struktura komunikatu
{
	long typ;
	long nadawca;
	char wiad[MAX];
}komunik;

void sigint_wtd(int sig);

int main()
{
	int kol_id;
	komunik kom;
	int i;
	key_t klucz;

	klucz=ftok(".",'K');//tworzenie klucza dla kolejki
	if(klucz==-1)
	{
		printf("Blad funkcji ftok (S)\n");
		exit(1);
	}

	kol_id=msgget(klucz,IPC_CREAT|0666);//uzyskiwanie dostepu do kolejki
	if(kol_id==-1)
	{
		perror("Nie uzyskano dostępu dop kolejki (S)");
		exit(1);
	}
	printf("Uzyskano dostep do kolejki, ID kolejki wynosi %d (S)\n", kol_id);

	signal(SIGINT, sigint_wtd);
	printf("^C konczy prace serwera\n");

	while(1)
	{
		printf("\nSerwer oczekuje na komunikat od klienta\n");
		kom.typ=1;//serwer jako odbiorca komunikatu
		memset(kom.wiad,0,MAX);

		if(msgrcv(kol_id,(komunik*)&kom,MAX+sizeof(long),kom.typ,0)==-1)//pobieranie wiadomosci z kolejki
		{
			perror("Blad pobrania komunikatu (S)");
			sigint_wtd(SIGINT);
		}
		printf("Serwer odebral komunikat od procesu %ld o tresci \"%s\"\n",kom.nadawca,kom.wiad);

		int dlugosc = strlen(kom.wiad);
		for(i=0;i<dlugosc;i++)//zamiana na duze litery
		{
			kom.wiad[i]=toupper(kom.wiad[i]);
		}
		//sleep(40);
		
		kom.typ=kom.nadawca;
		printf("Serwer wysyla komunikat do %ld, o tresci %s\n", kom.nadawca,kom.wiad);

		if(msgsnd(kol_id,(komunik*)&kom,strlen(kom.wiad)+1+sizeof(long),0)==-1)
		{
			perror("Blad wysylania komunikatu (S)");
			sigint_wtd(SIGINT);
		}

	}
	return 0;
}

void sigint_wtd(int sig)
{
	key_t klucz2;
	int kol_id2;
	klucz2=ftok(".",'K');//tworzenie klucza dla kolejki
	if(klucz2==-1)
	{
		printf("Blad funkcji ftok (S)\n");
		exit(1);
	}

	if(sig==SIGINT)
	{
		printf("Koniec pracy serwera\n");

		kol_id2=msgget(klucz2,IPC_CREAT|0666);//uzyskanie dostepu do kolejki
		if(kol_id2==-1)
		{
			perror("Nie uzyskano dostepu do kolejki (S)");
			exit(1);
		}

		if(msgctl(kol_id2,IPC_RMID,0)==-1)//usuwanie kolejki
		{
			perror("Blad usuwania kolejki (S)");
			exit(1);
		}
		else
		{
			printf("Usunieto kolejke o ID %d\n", kol_id2);
		}
	exit(0);
	}
}