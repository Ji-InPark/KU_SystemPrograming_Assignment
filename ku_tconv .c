#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>
#define BUF_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	int i;
	int j;
	int array[3][3];
}con_arg;

typedef struct{
	int i;
	int j;
	int result;
}con_ret;

typedef struct{
	int i;
	int j;
	int array[2][2];
}pool_arg;

typedef struct{
	int i;
	int j;
	int result;
}pool_ret;

void Cal_Con(int **IM, int conNum, int **CM);

void *make_con_thread(void *arg);

void Cal_Pool(int **CM, int conNum, int **PM);

void *make_pool_thread(void *arg);


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
	
	int** InputMatrix;
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
	
	int** ConMatrix;
	ConMatrix = (int**)malloc(sizeof(int*) * (arrayNum - 2));
	for(int i = 0; i < arrayNum - 2; i++)
	{
		ConMatrix[i] = (int*)malloc(sizeof(int) * (arrayNum - 2));
	}

	
	int conNum = arrayNum - 2;
	Cal_Con(InputMatrix, conNum, ConMatrix);

	int** PoolMatrix;
	PoolMatrix = (int**)malloc(sizeof(int*) * (conNum / 2));
	for(int i = 0; i < conNum / 2; i++)
	{
		PoolMatrix[i] = (int*)malloc(sizeof(int) * (conNum / 2));
	}

	Cal_Pool(ConMatrix, conNum, PoolMatrix);
	
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

void Cal_Con(int **IM, int conNum, int **CM)
{
	pthread_t* thread_id;
	thread_id = (pthread_t*)malloc(sizeof(pthread_t) * conNum * conNum);
	int status;
	con_arg *CA;
	CA = (con_arg*)malloc(sizeof(con_arg) * conNum * conNum);
	con_ret *CR;
	
	for(int i = 0; i < conNum; i++)
	{
		for(int j = 0; j < conNum; j++)
		{
			CA[i * conNum + j].i = i;
			CA[i * conNum + j].j = j;
			for(int k = 0; k < 3; k++)
			{
				for(int l = 0;  l < 3; l++)
				{
					CA[i * conNum + j].array[k][l] = IM[i + k][j + l];
				}
			}
		}
	}


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
	

	for(int i = 0; i < conNum; i++)
	{
		for(int j = 0; j < conNum; j++)
		{
			status = pthread_join(thread_id[i * conNum + j], (void**)&CR);
			if(status != 0)
			{
				perror("join con thread");
			}
			CM[i][j] = CR->result;
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
	con_arg *CA = (con_arg*)arg;
	con_ret *CR = malloc(sizeof(con_ret));
	int sum = 0;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j <3; j++)
		{
			sum += CA->array[i][j] * filter[i][j];
		}
	}
	
	CR->j = CA->j;
	CR->i = CA->i;
	pthread_mutex_unlock(&mutex);
	CR->result = sum;
	return CR;
}

void Cal_Pool(int **CM, int conNum, int **PM)
{

	pthread_t* thread_id;
	thread_id = (pthread_t*)malloc(sizeof(pthread_t) * conNum * conNum / 4);
	int status;
	pool_arg *PA;
	PA = (pool_arg*)malloc(sizeof(pool_arg) * conNum * conNum / 4);
	pool_ret *PR;
	

	int cnt = 0;
	for(int i = 0; i < conNum; i += 2)
	{
		for(int j = 0; j < conNum; j += 2)
		{
			PA[cnt].i = i;
			PA[cnt].j = j;
			for(int k = 0; k < 2; k++)
			{
				for(int l = 0;  l < 2; l++)
				{
					PA[cnt].array[k][l] = CM[i + k][j + l];
				}
			}
			cnt++;
		}
	}

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
			PM[i][j] = PR->result;
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
			if(result < PA->array[i][j])
					result = PA->array[i][j];
		}
	}
	
	PR->j = PA->j;
	PR->i = PA->i;
	pthread_mutex_unlock(&mutex);
	PR->result = result;
	return PR;

}

