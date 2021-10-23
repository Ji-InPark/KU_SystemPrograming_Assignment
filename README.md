# KU_SystemPrograming_Assignment

<br/>

## 개요

시스템 프로그래밍이라는 과목에서 제출했었던 코드들과 보기 좋게 고친 코드들을 올릴 레포입니다.

<br/>

병렬 프로그래밍을 위한 프로세스와 스레드를 사용한 과제이며, 과제의 기능은 같으나 프로세스를 사용하느냐, 스레드를 사용하느냐의 차이가 있었던 과제였습니다.

<br/>

## 과제 설명

![과제 설명 사진1](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-1.PNG?raw=true)

위 Convolutional Neural Networks(합성곱 신경망)의 과정 중 빨간 네모로 표시한 부분을 병렬 프로그래밍으로 간단하게 구현하는 것이 과제입니다.

<br/>

![과제 설명 사진2](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-2.PNG?raw=true)

메인 프로세스(스레드)에서 Convolutional Layer에 필요한 만큼 프로세스(스레드)를 만들어 계산을 한 뒤 그 값들을 다시 메인 프로세스(스레드)로 넘겨줍니다.

메인 프로세스(스레드)에서 Max_pooling Layer에 필요한 만큼 프로세스(스레드)를 만들어 계산을 한 뒤 그 값들을 다시 메인 프로세스(스레드)로 넘겨줍니다.

<br/>

프로세스를 사용할 때는 **프로세스간 통신**을 사용하여 값들을 넘겨줍니다.

스레드를 사용할 때는 **Lock**을 사용하여 **Critical Section**에 접근해 값들을 넘겨줍니다.

<br/>

![과제 설명 사진3](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-3.PNG?raw=true)

Convolutional Layer에 생성된 프로세스들(스레드)은 각각 맡은 구역에 대해서 계산을 해줍니다.

계산 방법은 과제에 정의된 방법으로 진행합니다.

<br/>

![과제 설명 사진4](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-4.PNG?raw=true)

Max_pooling Layer에 생성된 프로세스들(스레드)도 각각 맡은 구역에 대해서 계산을 해줍니다.

계산 방법은 과제에 정의된 방법으로 진행합니다.

<br/>

![과제 설명 사진5](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-5.PNG?raw=true)

입력으로 받는 행렬은 미리 제공받은 ku_func.o 목적 파일에 정의된 함수를 통해서 받습니다.

<br/>

![과제 설명 사진6](https://github.com/Ji-InPark/ForImage/blob/master/System_Programming/SPA-6.PNG?raw=true)

입출력 형식입니다.

<br/>

## 파일 설명

**ku_conv.c** 는 프로세스를 사용한 과제이고 **프로세스를 통한 병렬 처리**, **프로세스간 통신**을 잘 처리해야하는 과제였습니다.

**ku_tconv.c** 는 스레드를 사용한 과제이고 **스레드를 통한 병렬 처리**, **Critical Section에 대한 Lock**을 잘 처리해야 하는 과제였습니다.

<br/>

**ku_conv_prettier.c, ku_tconv_prettier.c** 파일은 제가 github에 올리면서 코드들을 더욱 보기 좋게, 틀렸다고 생각되는 부분들은 수정해서 올리는 파일들 입니다.

<br/>

## 환경

개발 환경은 우분투 어쩌고 저쩌고 이고

os는 리눅스 어쩌고 이고

사용 언어는 c입니다.

<br/>
