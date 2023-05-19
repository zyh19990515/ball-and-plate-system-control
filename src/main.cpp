#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include "wifi.h"
#define BAUD_RATE 115200
#define CHAR_BUF 128
#define ZERO_X 133
#define ZERO_Y 100

// #define YL        5    
// #define YH        6  
// #define XL        7  
// #define XH        8
Servo myservo1,myservo2;
int Flag_Stop=1,Flag_Show,Flag_Move;  //相关标志位
float Zero_X=2,Zero_Y=2,Target_X,Target_Y;  //X Y方向的目标值和控制量
int Position_X,Position_Y; //X Y方向的测量值
// float Balance_Kp=53,Balance_Kd=58;//PID参数
float Balance_Kp=550,Balance_Kd=100;
int cnt = 0;
char a;
float velocity = 0;
// float time = 0;
int last_pos = 5;
void(*resetFunc)(void)=0;



 void Control_servo(float velocity_x,float velocity_y)
{
    myservo1.write(90-velocity_x);        // X舵机转向的角度
    myservo2.write(90-velocity_y);        // Y舵机转向的角度
}

void getVelocity(int& cur_pos, int& last_pos, float& velocity){
  if(cur_pos!=last_pos){
    // velocity = 5/time;
  }
  else{
    velocity = 0;
  }
}

int balanceX(float Angle )
{  
   float  Differential,Bias,Balance_Ki=0.06;//定义差分变量和偏差
   static float Last_Bias,Integration,Balance_Integration,Flag_Target;  //上一次的偏差值
   int balance;//平衡的返回值
   Bias=(Angle-Zero_X);  //===求出平衡的角度中值 和机械相关  
   Differential=Bias-Last_Bias;  //求得偏差的变化率  
  if(++Flag_Target>20) //错频处理积分控制
  {
   Flag_Target=0;
   if(Flag_Stop==0) Integration+=Bias;  // 检测到小球且舵机使能则积分
   else Integration=0;//否则清零
   if(Integration<-200) Integration=-200; //积分限幅
   if(Integration>200)  Integration=200;  
   Balance_Integration=Integration*Balance_Ki;  //积分控制
  }   
   balance=Balance_Kp*Bias/500+Balance_Kd*Differential/50+Balance_Integration;   //===计算平衡控制的舵机PWM  PD控制   kp是P系数 kd是D系数 
   Last_Bias=Bias;  //保存上一次的偏差
   return balance;  //返回值
}

int balanceY(float Angle )
{  
 float  Differential,Bias,Balance_Ki=0.06;//定义差分变量和偏差
   static float Last_Bias,Integration,Balance_Integration,Flag_Target;  //上一次的偏差值
   int balance;//平衡的返回值
   Bias=(Angle-Zero_Y);  //===求出平衡的角度中值 和机械相关  
   Differential=Bias-Last_Bias;  //求得偏差的变化率  
  if(++Flag_Target>20) //错频处理积分控制
  {
   Flag_Target=0;
   if(Flag_Stop==0) Integration+=Bias;  // 检测到小球且舵机使能则积分
   else Integration=0;//否则清零
   if(Integration<-200) Integration=-200; //积分限幅
   if(Integration>200)  Integration=200;  
   Balance_Integration=Integration*Balance_Ki;  //积分控制
  }   
   balance=Balance_Kp*Bias/500+Balance_Kd*Differential/50+Balance_Integration;   //===计算平衡控制的舵机PWM  PD控制   kp是P系数 kd是D系数 
   Last_Bias=Bias;  //保存上一次的偏差
   return balance;  //返回值
}


void control(){  

  static uint8_t Max_Target=30;
  String mes = WiFiReceive();
  int pos_x, pos_y;
  int pos;
  //得到位置1-9
  if(mes.length()!=0){
    pos = mes[0]-'0';
  }
  else{
    pos = 5;
  }
  Serial.println(pos);
  //分解出x,y
  switch (pos)
  {
    case 1:
      pos_x = 0;
      pos_y = 0;
      break;
    case 2:
      pos_x = 0;
      pos_y = 1;
      break;
    case 3:
      pos_x = 0;
      pos_y = 2;
      break;
    case 4:
      pos_x = 1;
      pos_y = 0;
      break;
    case 5:
      pos_x = 1;
      pos_y = 1;
      break;
    case 6:
      pos_x = 1;
      pos_y = 2;
      break;
    case 7:
      pos_x = 2;
      pos_y = 0;
      break;
    case 8:
      pos_x = 2;
      pos_y = 1;
      break;
    case 9:
      pos_x = 2;
      pos_y = 2;
      break;
  
  default:
    break;
  }


  Position_X = pos_x;
  Position_Y = pos_y;
  Serial.print("x:");
  Serial.print(Position_X);
  Serial.print(" y:");
  Serial.println(Position_X);
  Target_X=-balanceX(Position_X);   //X方向的PID控制器
  Target_Y=-balanceY(Position_Y);   //Y方向的PID控制器
  // Serial.print("x:");
  // Serial.print(Target_X);
  // Serial.print(" y:");
  // Serial.println(Target_Y);
  if(Target_X<-Max_Target) Target_X=-Max_Target;  //X方向的舵机的控制最大角度
  if(Target_X>Max_Target)  Target_X=Max_Target;   //X方向的舵机的控制最大角度
  if(Target_Y<-Max_Target) Target_Y=-Max_Target;  //Y方向的舵机的控制最大角度
  if(Target_Y>Max_Target)  Target_Y=Max_Target;   //Y方向的舵机的控制最大角度
  delay(25);
  Control_servo(Target_X,Target_Y); 
  last_pos = pos;
 }



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  WiFiInit();
  while (!Serial)
  {
    delay(200);
  }

  myservo1.attach(10);           //初始化各个舵机
  myservo2.attach(9);            //初始化各个舵机
  myservo1.write(94);
  myservo2.write(88);
  Serial.println("setup finished");
  for(int i=0;i<5;i++){
    Serial.println(i);
    delay(1000);
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  
  // control();
    // String mes = WiFiReceive();
    // if(mes.length()!=0){
    //   Serial.println(mes);
    // }
    control();
    // time = millis()-time;
  // Serial.print("set_x:");
  // Serial.print(Zero_X);
  // Serial.print("    set_y:");
  // Serial.print(Zero_Y);
  // Serial.print("    PositionX:");
  // Serial.print(Position_X);
  // Serial.print("    PositionY");
  // Serial.println(Position_Y);
  
}