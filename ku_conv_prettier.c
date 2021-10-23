#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<string.h>

void makeMatrix(int** matrix, int x, int y);
void setCM(int** CM, int cx, int cy);
void CalCon(int id);
void CalMax(int id);
void devideIM(int** IM, int x, int y);
void devideCM(int** CM, int cx, int cy);
void makeCCP(int x, int y);
void makeMCP(int cx, int cy);
void printResult(int cx, int cy);

int main(int argc, char* argv[]){

	int x = atoi(argv[1]);
	int y = atoi(argv[1]);

	int** InputMatrix;
	InputMatrix = (int**)malloc(sizeof(int*) * x);
	for(int i = 0; i < x; i++)
	{
		InputMatrix[i] = (int*)malloc(sizeof(int) * y);
	}

	
	makeMatrix(InputMatrix, x, y);
	
	devideIM(InputMatrix, x, y);
	makeCCP(x, y);

	int cx = x - 2;
	int cy = y - 2;

	int** ConMatrix;
	ConMatrix = (int**)malloc(sizeof(int*) * cx);
	for(int i = 0; i < cx; i++)
	{
		ConMatrix[i] = (int*)malloc(sizeof(int) * cy);
	}
	setCM(ConMatrix, cx, cy);

	devideCM(ConMatrix, cx, cy);
	makeMCP(cx, cy);

	printResult(cx, cy);
}

void setCM(int** CM, int cx, int cy)
{

	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);

	ipckeyc = ftok("./tmp/foo", 1946);
	mqdesc = msgget(ipckeyc, IPC_CREAT|0600);
	if(mqdesc < 0)
	{
		perror("msgget()");
		exit(0);
	}


	for(int i = 0; i < cx; i++)
	{
		for(int j = 0; j < cy; j++)
		{
			if(msgrcv(mqdesc, &mymsgc, buf_lenc, (i + 1) * 10 + (j + 1), 0) == -1)
			{
				perror("setCM msgrcv()");
				exit(0);
			}
			else
			{
				CM[i][j] = mymsgc.value[0][0];
			}
	
		}
	}

}

void printResult(int cx, int cy)
{
	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);

	ipckeym = ftok("./tmp/foo", 1947);
	mqdesm = msgget(ipckeym, IPC_CREAT|0600);
	if(mqdesm < 0)
	{
		perror("msgget()");
		exit(0);
	}


	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			if(msgrcv(mqdesm, &mymsgm, buf_lenm, (i + 1) * 10 + (j + 1), MSG_NOERROR) == -1)
			{
				perror("printResult msgrcv()");
				exit(0);
			}
			else
			{
				printf("%d ", mymsgm.value[0][0]);
			}
		}
	}

}

void CalMax(int id)
{
	int result = 0;

	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);

	ipckeym = ftok("./tmp/foo", 1947);
	mqdesm = msgget(ipckeym, IPC_CREAT|0600);
	if(mqdesm < 0)
	{
		perror("msgget()");
		exit(0);
	}

	if(msgrcv(mqdesm, &mymsgm, buf_lenm, id, MSG_NOERROR) == -1)
	{
		perror("CalMax msgrcv()");
		exit(0);
	}
	else
	{
		result = mymsgm.value[0][0];
		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				if(result < mymsgm.value[i][j])
				{
					result = mymsgm.value[i][j];
				}	

			}
		}
		mymsgm.id = id;
		mymsgm.value[0][0] = result;
		if(msgsnd(mqdesm, &mymsgm, buf_lenm, 0) == -1)
		{
			perror("msgsnd()");
			exit(0);
		}		

	}
	exit(0);

}

void makeMCP(int cx, int cy)
{
	int pid[cx * cy / 4];
	int k = 0;
	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			pid[k] = fork();
			if(pid[k++] == 0)
				CalMax((i + 1) * 10 + (j + 1));
		}
	}
	
	k = 0;
	int child_status;
	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			waitpid(pid[k++], &child_status, 0);
		}
	}



}

void devideCM(int** CM, int cx, int cy)
{
	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);

	ipckeym = ftok("./tmp/foo", 1947);
	mqdesm = msgget(ipckeym, IPC_CREAT|0600);
	if(mqdesm < 0)
	{
		perror("msgget()");
		exit(0);
	}

	int temp[2][2] = {
		{0, 0},
		{0, 0}
	};

	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			mymsgm.id = (i + 1) * 10 + (j +1);
			for(int k = 0; k < 2; k++)
			{
				for(int l = 0; l < 2; l++)
				{
					temp[k][l] = CM[i + k][j + l];
				}
			}
			memcpy(mymsgm.value, temp, sizeof(temp));
			if(msgsnd(mqdesm, &mymsgm, buf_lenm, 0) == -1)
			{
				perror("msgsnd()");
				exit(0);
			}
		}
	}

}

void CalCon(int id)
{
	int filter[3][3] = {
		{-1, -1, -1},
		{-1, 8, -1},
		{-1, -1, -1}
	};
	int result = 0;

	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);

	ipckeyc = ftok("./tmp/foo", 1946);
	mqdesc = msgget(ipckeyc, IPC_CREAT|0600);
	if(mqdesc < 0)
	{
		perror("msgget()");
		exit(0);
	}

	if(msgrcv(mqdesc, &mymsgc, buf_lenc, id, MSG_NOERROR) == -1)
	{
		perror("CalCon msgrcv()");
		exit(0);
	}
	else
	{
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				result += mymsgc.value[i][j] * filter[i][j];
			}
		}
		mymsgc.id = id;
		mymsgc.value[0][0] = result;
		if(msgsnd(mqdesc, &mymsgc, buf_lenc, 0) == -1)
		{
			perror("msgsnd()");
			exit(0);
		}		

	}
	exit(0);
}

void makeCCP(int x, int y)
{
	int pid[x * y];
	int k = 0;
	for(int i = 0; i < x - 2; i++)
	{
		for(int j = 0; j < y - 2; j++)
		{
			pid[k] = fork();
			if(pid[k++] == 0)
				CalCon((i + 1) * 10 + (j + 1));
			else
				continue;
		}
	}
	
	k = 0;
	int child_status;
	for(int i = 0; i < x - 2; i++)
	{
		for(int j = 0; j < y - 2; j++)
		{
			waitpid(pid[k++], &child_status, 0);
		}
	}


}

void devideIM(int** IM, int x, int y)
{
	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);

	ipckeyc = ftok("./tmp/foo", 1946);
	mqdesc = msgget(ipckeyc, IPC_CREAT|0600);
	if(mqdesc < 0)
	{
		perror("msgget()");
		exit(0);
	}

	int temp[3][3] = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};

	for(int i = 0; i < x - 2; i++)
	{
		for(int j = 0; j < y - 2; j++)
		{
			mymsgc.id = (i + 1) * 10 + (j +1);
			for(int k = 0; k < 3; k++)
			{
				for(int l = 0; l < 3; l++)
				{
					temp[k][l] = IM[i + k][j + l];
				}
			}
			memcpy(mymsgc.value, temp, sizeof(temp));
			if(msgsnd(mqdesc, &mymsgc, buf_lenc, 0) == -1)
			{
				perror("msgsnd()");
				exit(0);
			}
		}
	}
}


