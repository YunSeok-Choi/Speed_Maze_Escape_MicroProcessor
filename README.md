# MicroProcessor
<h1>스피드 미로 탈출 게임</h2>
<h3>MicroProcessor Project.</h3>
<h4><b>tm4c123gh6pm</b> 이용, 115200 통신속도, Tera Term 사용</h4>
<br>
<h4>게임 간단 설명 </h4>
<ol>
    <li>난이도를 입력 (UART 사용)</li>
    <li>조이스틱을 이용하여 이동 (ADC 사용)</li>
    <li>시간안에 목적지 안에 이동 (TIMER 사용)</li>
    <li>목적지에 가까워 질수록 LED세기가 커짐 (PWM 사용)</li>
</ol><hr />
<h5>
    UART는 Port A에 연결하여 입력을 받을시 인터럽트 발생<br />
    ADC는 Port E에 연결하여 조이스틱의 방향 값을 Digital로 변환해줌<br />
    클록을 주어 TIMER 인터럽트를 발생. 주기는 1초로 설정<br />
    PWM은 Port B로 설정 후 LED가 출력되는 PORT F와 연결해 LED강도 조절
</h5>

![image](https://user-images.githubusercontent.com/66102708/147399953-e09ec88c-38b2-45b6-895c-8e8b4e784c34.png)

