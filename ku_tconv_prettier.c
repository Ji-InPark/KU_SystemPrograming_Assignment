#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>
#define BUF_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct{					// Cal_Con 함수에 넘길 구조체
	int i;
	int j;
}con_arg;

typedef struct{					// Cal_Con 함수의 리턴값을 받을 구조체
	int result;
}con_ret;

typedef struct{					// Cal_Pool 함수에 넘길 구조체
	int i;
	int j;
}pool_arg;

typedef struct{					// Cal_Pool 함수의 리턴값을 받을 구조체
	int result;
}pool_ret;

// convolutional layer 계산을 하는 함수
void Cal_Con(int conNum);

// convolutional layer 계산을 위한 쓰레드가 실행하는 함수
void *make_con_thread(void *arg);

// max pool layer 계산을 하는 함수
void Cal_Pool(int conNum);

// max pool layer 계산을 위한 쓰레드가 실행하는 함수
void *make_pool_thread(void *arg);

// 입력받은 데이터를 저장하는 배열 선언
int** InputMatrix;

// convolutional layer 계산 후 데이터를 저장하는 배열 선언
int** ConMatrix;

// max pool layer 계산 후 데이터를 저장하는 배열 선언
int** PoolMatrix;

int main(int argc, char *argv[])
{
	int fd, n;
	char rbuf[BUF_SIZE];
	size_t len;
	ssize_t ret;
	char *bufp = rbuf;

	fd = open(argv[1], O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		return 0;
	}
	len = BUF_SIZE;

	ret = read(fd, bufp, len);
	
	close(fd);

	
	char* token = strtok(rbuf, "\n");
	int arrayNum = atoi(token);
	
	// 입력 데이터 저장
	InputMatrix = (int**)malloc(sizeof(int*) * arrayNum);
	for(int i = 0; i < arrayNum; i++)
	{
		InputMatrix[i] = (int*)malloc(sizeof(int) * arrayNum);
	}

	for(int i = 0; i < arrayNum; i++)
	{
		for(int j = 0; j < arrayNum; j++)
		{
			if(j == arrayNum - 1)
			{
				token = strtok(NULL, "\n");
			}
			else
			{
				token = strtok(NULL, " ");
			}
			InputMatrix[i][j] = atoi(token);
		}
	}
	/*-------------------------------------------------*/
	
	// convolutional layer 계산 후 데이터 저장
	ConMatrix = (int**)malloc(sizeof(int*) * (arrayNum - 2));
	for(int i = 0; i < arrayNum - 2; i++)
	{
		ConMatrix[i] = (int*)malloc(sizeof(int) * (arrayNum - 2));
	}
	
	int conNum = arrayNum - 2;
	Cal_Con(InputMatrix, conNum, ConMatrix);
	/*-------------------------------------------------*/

	// max pool layer 계산 후 데이터 저장
	PoolMatrix = (int**)malloc(sizeof(int*) * (conNum / 2));
	for(int i = 0; i < conNum / 2; i++)
	{
		PoolMatrix[i] = (int*)malloc(sizeof(int) * (conNum / 2));
	}

	Cal_Pool(conNum);
	/*-------------------------------------------------*/
	
	// 모든 계산 이후 파일로 출력하는 과정
	int wfd;
	wfd = open(argv[2], O_WRONLY);
	len = 5;
	lseek(wfd, 0, SEEK_SET);
	for(int i = 0; i < conNum / 2; i++)
	{
		for(int j = 0; j < conNum / 2; j++)
		{
			char tempArr[6];
			if(j == conNum / 2 - 1)
			{
				
				sprintf(tempArr, "%4d\n", PoolMatrix[i][j]);
			}
			else
			{
				sprintf(tempArr, "%4d ", PoolMatrix[i][j]);	
			}
			write(wfd, tempArr, sizeof(tempArr));
			lseek(wfd, len, SEEK_CUR);
		}
	}
	close(wfd);
	
}

void Cal_Con(int conNum)
{
	pthread_t* thread_id;													// join을 하기 위해 쓰레드 id를 저장하는 변수
	thread_id = (pthread_t*)malloc(sizeof(pthread_t) * conNum * conNum);	// 필요한 쓰레드 개수만큼 할당

	int status;																// 쓰레드의 상태를 저장하는 변수 (에러 감지 목적)
	con_arg *CA;															// 각 쓰레드에 파라매터로 넘기기 위해서 구조체 포인터 변수를 선언
	CA = (con_arg*)malloc(sizeof(con_arg) * conNum * conNum);				// 필요한 쓰레드 개수만큼 할당
	con_ret *CR;															// 쓰레드의 리턴을 받기위한 구조체 포인터 변수 선언
	

	// 파라매터로 넘기기 위해 구조체 배열에 값을 넣는 과정
	for(int i = 0; i < conNum; i++)
	{
		for(int j = 0; j < conNum; j++)
		{
			CA[i * conNum + j].i = i;
			CA[i * conNum + j].j = j;
		}
	}

	// 쓰레드를 만들고 쓰레드가 실행할 함수 그리고 파라매터를 넘기는 과정
	for(int i = 0; i < conNum; i++)
	{
		for(int j = 0; j < conNum; j++)
		{

			status = pthread_create(&thread_id[i * conNum + j], NULL, make_con_thread, &CA[i * conNum + j]);
			if(status != 0)
			{
				perror("create con thread");
				exit(1);
			}

		}
	}
	

	// 쓰레드의 리턴값을 받고 조인하고 ConMatrix 배열에 데이터 채우는 과정
	for(int i = 0; i < conNum; i++)
	{
		for(int j = 0; j < conNum; j++)
		{
			status = pthread_join(thread_id[i * conNum + j], (void**)&CR);
			if(status != 0)
			{
				perror("join con thread");
			}
			ConMatrix[i][j] = CR->result;
		}
	}
}

void *make_con_thread(void *arg)
{
	int filter[3][3] = {
		{-1, -1, -1},
		{-1, 8, -1},
		{-1, -1, -1} };
	
	pthread_mutex_lock(&mutex);	
	con_arg *CA = (con_arg*)arg;				// 파라매터 캐스팅
	con_ret *CR = malloc(sizeof(con_ret));
	int sum = 0;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j <3; j++)
		{
			sum += InputMatrix[CA->i + i][CA->j + j] * filter[i][j];
		}
	}

	pthread_mutex_unlock(&mutex);
	CR->result = sum;
	return CR;
}

void Cal_Pool(int conNum)
{
	pthread_t* thread_id;																	// join을 하기 위해 쓰레드 id를 저장하는 변수
	thread_id = (pthread_t*)malloc(sizeof(pthread_t) * conNum * conNum / 4);				// 필요한 쓰레드 개수만큼 할당

	int status;																				// 쓰레드의 상태를 저장하는 변수 (에러 감지 목적)
	pool_arg *PA;																			// 각 쓰레드에 파라매터로 넘기기 위해서 구조체 포인터 변수를 선언
	PA = (pool_arg*)malloc(sizeof(pool_arg) * conNum * conNum / 4);							// 필요한 쓰레드 개수만큼 할당
	pool_ret *PR;																			// 쓰레드의 리턴을 받기위한 구조체 포인터 변수 선언
	

	// 파라매터로 넘기기 위해 구조체 배열에 값을 넣는 과정
	int cnt = 0;
	for(int i = 0; i < conNum; i += 2)
	{
		for(int j = 0; j < conNum; j += 2)
		{
			PA.i = i;
			PA.j = j;
			cnt++;
		}
	}

	// 쓰레드를 만들고 쓰레드가 실행할 함수 그리고 파라매터를 넘기는 과정
	cnt = 0;
	for(int i = 0; i < conNum; i += 2)
	{
		for(int j = 0; j < conNum; j += 2)
		{

			status = pthread_create(&thread_id[cnt], NULL, make_pool_thread, &PA[cnt]);
			if(status != 0)
			{
				perror("create con thread");
				exit(1);
			}
			cnt++;

		}
	}
	
	// 쓰레드의 리턴값을 받고 조인하고 PoolMatrix 배열에 데이터 채우는 과정
	cnt = 0;
	for(int i = 0; i < conNum / 2; i++)
	{
		for(int j = 0; j < conNum / 2; j++)
		{
			status = pthread_join(thread_id[cnt], (void**)&PR);
			if(status != 0)
			{
				perror("join con thread");
			}
			PoolMatrix[i][j] = PR->result;
			cnt++;
		}
	}
}

void *make_pool_thread(void *arg)
{ 
	pthread_mutex_lock(&mutex);	
	pool_arg *PA = (pool_arg*)arg;
	pool_ret *PR = malloc(sizeof(pool_ret));
	int result = PA->array[0][0];
	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j <2; j++)
		{
			if(result < ConMatrix[PA->i + i][PA->j + j])
					result = ConMatrix[PA->i + i][PA->j + j];
		}
	}
	pthread_mutex_unlock(&mutex);
	PR->result = result;
	return PR;

}

