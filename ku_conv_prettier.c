#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<string.h>

void makeMatrix(int** matrix, int x, int y);			// ku_func.o 에서 제공하는 함수 - input 담당
void devideIM(int** IM, int x, int y);					// InputMatrix를 나누어 메세지큐로 넘기는 함수
void makeCCP(int x, int y);								// CalCon 작업을 위한 프로세스를 필요한 만큼 생성하는 함수
void CalCon(int id);									// Convolutional Layer 처리를 하는 함수
void setCM(int** CM, int cx, int cy);					// Convolutinal 처리가 된 Matrix를 set하는 함수
void devideCM(int** CM, int cx, int cy);				// ConvolutinalMatrix를 나누어 메세지큐로 넘기는 함수
void makeMCP(int cx, int cy);							// CalMax 작업을 위한 프로세스를 필요한 만큼 생성하는 함수
void CalMax(int id);									// Pooling Layer 처리를 하는 함수
void printResult(int cx, int cy);						// 결과를 출력하는 함수

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


void printResult(int cx, int cy)								// 메세지큐에 들어있는 결과들을 출력하는 함수
{
	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);
	//


	ipckeym = ftok("./tmp/foo", 1947);
	mqdesm = msgget(ipckeym, IPC_CREAT|0600);
	if(mqdesm < 0)
	{
		perror("msgget()");
		exit(0);
	}


	// 순서대로 메세지큐의 내용을 받고 출력
	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			if(msgrcv(mqdesm, &mymsgm, buf_lenm, (i + 1) * cx + (j + 1), MSG_NOERROR) == -1)
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

void CalMax(int id)												// Pooling Layer의 처리(4개의 값중 가장 큰 값 찾기)하고 다시 메세지큐를 보내는 함수
{
	int result = 0;

	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);
	//

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
		// 최대값을 찾는 과정
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
		
		// 최대값을 찾고 다시 메세지큐로 보내는 과정
		if(msgsnd(mqdesm, &mymsgm, buf_lenm, 0) == -1)
		{
			perror("msgsnd()");
			exit(0);
		}		

	}
	exit(0);

}

void makeMCP(int cx, int cy)									// Pooling Layer 처리를 위한 프로세스를 만드는 함수
{
	int pid[cx * cy / 4];		// 프로세스 id를 저장하기 위한 배열
	int k = 0;					// pid 배열 index로 사용하기 위한 변수

	// 프로세스 생성 및 자식들 CalMax 함수 진행
	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			pid[k] = fork();
			if(pid[k++] == 0)
				CalMax((i + 1) * cx + (j + 1));
		}
	}
	

	// 프로세스 join 하는 과정
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

void devideCM(int** CM, int cx, int cy)							// Covolutional Layer 처리가 끝난 Matrix를 Pooling Layer 처리를 위해 나누고 메세지큐로 보내는 함수
{
	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeym;
	int mqdesm;
	size_t buf_lenm;
	struct {
		long id;
		int value[2][2];
	} mymsgm;

	buf_lenm = sizeof(mymsgm.value);
	//

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


	// 구간을 나누고 메세지큐로 보내는 과정
	for(int i = 0; i < cx; i += 2)
	{
		for(int j = 0; j < cy; j += 2)
		{
			mymsgm.id = (i + 1) * cx + (j +1);
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

void setCM(int** CM, int cx, int cy)							// Convoultional Layer 처리가 끝난 후 메세지큐로부터 값을 받아 Matrix를 구성하는 함수
{
	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);
	//

	ipckeyc = ftok("./tmp/foo", 1946);
	mqdesc = msgget(ipckeyc, IPC_CREAT|0600);
	if(mqdesc < 0)
	{
		perror("msgget()");
		exit(0);
	}

	// 메세지큐로부터 값을 받아 Matrix를 구성하는 과정
	for(int i = 0; i < cx; i++)
	{
		for(int j = 0; j < cy; j++)
		{
			if(msgrcv(mqdesc, &mymsgc, buf_lenc, (i + 1) * (cx + 2) + (j + 1), 0) == -1)
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

void CalCon(int id)												// Convolutional Layer 처리를 한 후 메세지큐 보내는 함수
{
	// Convolutional 처리를 위한 필터 변수
	int filter[3][3] = {
		{-1, -1, -1},
		{-1, 8, -1},
		{-1, -1, -1}
	};
	int result = 0;

	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);
	//

	ipckeyc = ftok("./tmp/foo", 1946);
	mqdesc = msgget(ipckeyc, IPC_CREAT|0600);
	if(mqdesc < 0)
	{
		perror("msgget()");
		exit(0);
	}


	// 메세지큐로 입력받고 Covolutional Layer 처리를 한 후 다시 메세지큐로 보내는 과정
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

void makeCCP(int x, int y)										// Covolutional Layer 처리를 위한 프로세스를 만드는 함수
{
	int pid[x * y];			// 프로세스 id를 저장하기 위한 배열
	int k = 0;				// pid 배열 index로 사용하기 위한 변수

	// 프로세스 생성 및 자식들 CalCon 함수 진행
	for(int i = 0; i < x - 2; i++)
	{
		for(int j = 0; j < y - 2; j++)
		{
			pid[k] = fork();
			if(pid[k++] == 0)
				CalCon((i + 1) * x + (j + 1));
			else
				continue;
		}
	}
	
	// 프로세스 join 과정
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

void devideIM(int** IM, int x, int y)							// InputMatrix를 Convolutional Layer 처리를 위해 나누고 메세지큐로 보내는 함수
{
	// 메세지큐 사용을 위해 필요한 변수들 선언
	key_t ipckeyc;
	int mqdesc;
	size_t buf_lenc;
	struct {
		long id;
		int value[3][3];
	} mymsgc;

	buf_lenc = sizeof(mymsgc.value);
	//

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

	// 구간을 나누고 메세지큐로 보내는 과정
	for(int i = 0; i < x - 2; i++)
	{
		for(int j = 0; j < y - 2; j++)
		{
			mymsgc.id = (i + 1) * x + (j +1);
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


