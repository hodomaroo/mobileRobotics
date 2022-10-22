/************************************************************************************************************/
/*                                                                                                          */
/*                                              MobileRobot.c                                               */
/*                                                                                                          */
/*                                                                                       2020. 1. 1.        */
/************************************************************************************************************/
#include "Interface.h"


#define cw wa = 0;
#define ar acf =cacf= 0;
#define set pos[0] = pos[1] = pos[2] = 0; gyro = rearGyro = 0;
#define V3 while(!Cmd(0,19));
#define SV1 while(!Cmd(0,18));
#define reset bar_1 = bar_2 = bar_3 = bar_4 = bar_5 = 0;
#define wait if(key) {SS(); while(SW2); while(!SW2);}
#define lcs lcd_clear_screen();
int key= 0, key_puck = 0;

int tmp;
#define swap(a,b) tmp = a, a = b, b = tmp;

int map[13] = {
	2,1,1,
	 3,0,
	0,3,0,
	 2,1,
	2,0,3
};
int node[13];
int bMap[6];

int link[13][8] = {
   {-1,-1,-1,-1,-1,3,1,-1},
   {-1,-1,0,3,-1,4,2,-1},
   {-1,-1,1,4,-1,-1,-1,-1},
   
   {-1,0,-1,5,-1,-1,-1,1},
   {-1,1,-1,-1,-1,7,-1,2},
   
   {-1,-1,-1,-1,-1,8,-1,3},
   {-1,-1,-1,-1,-1,-1,-1,-1},
   {-1,4,-1,9,-1,-1,-1,-1},
   
   {-1,5,-1,10,-1,11,-1,-1},
   {-1,-1,-1,11,-1,12,-1,7},
   
   {-1,-1,-1,-1,-1,-1,11,8},
   {-1,8,10,-1,-1,-1,12,9},
   {-1,9,11,-1,-1,-1,-1,-1}
};

int route[40]; //[0] : 회전 타입 / [1] : 회전 방향  [2] : 회전 횟수 
int g_route[40];
int seq_route[8],g_seq_route[8],seq_min;
int minMove = 30,flg = 0;
int fix[13] = {0};
int hist[40][13];
int curDp = 0;
int stage = 0;
int move_activation = 0;
int nowPos;
int call=  0;


int InHist(int *node, int dp){
   for(int i = 0  ;i < dp ; i++){
      for(int j = 0 ;  j < 13 ; j++){
         if(node[j] != hist[i][j]) break;
         if(j == 12) return 1;
      }
   }
   return 0;
}



void dis(int *arr){
	printf("\n%3d %3d %3d\n",arr[0],arr[1],arr[2]);
	printf("  %3d %3d\n",arr[3],arr[4]);
	printf("%3d %3d %3d\n",arr[5],arr[6],arr[7]);
	printf("  %3d %3d\n",arr[8],arr[9]);
	printf("%3d %3d %3d\n",arr[10],arr[11],arr[12]);
//	system("pause");
	//system("cls");
}

int seq[8] = {1,4,7,9,11,8,5,3};

int EdgeCheck(int *node){
   	if(node[0] > 0 && node[0] == node[2] && node[0] != node[6]){ 
      	fix[0] = fix[2] = 1;
      	return 1;
   	}
   	if(node[10] > 0 && node[10] == node[12] && node[10] != node[6]){ 
      	fix[10] = fix[12] = 1;
      	return 1;
   	}
   	return 0;
}

int Check(int *node){ //1차 정렬 확인 함수 
   	if(node[0]+node[2]+node[10]+node[12] != (6-map[6])*2) return 0;
   	if(node[0] <= 0 || node[2] <= 0 || node[10] <= 0 || node[12] <= 0) return 0;
   	if(node[0] != node[2]) return 0;
   
   	int pCnt = 0,pList[8];
   	for(int i = 0 ; i < 8 ; i++){
      	int col = node[seq[i]];
      	if((pCnt && col == pList[pCnt - 1]) || pCnt > 4) return 0;
      	if(col > 0) pList[pCnt++] = col;
   	}
   
   if(pList[pCnt-1] == pList[0]) return 0;
   
   return 1;
}

void Sort(int lp, int p,int dp, int *node){
	call ++;
	if(flg || dp >= curDp) return;
	if(InHist(node,dp)) return;
	
	route[dp] = p;
	
	memcpy(hist[dp],node,sizeof(hist[dp]));
	
	if(stage == 0 && EdgeCheck(node) || (stage == 1 && Check(node))){
		flg = 1;
		memcpy(g_route,route,sizeof(route));
		curDp= dp;
		return;
	}
	
	
	for(int i = 0 ; i < 8 ; i++){
		int np = link[p][i];
		if(np < 0 || np == lp) continue;
		if(fix[np]) continue;
		
		
		swap(node[p],node[np]);
		Sort(p,np,dp+1,node);
		swap(node[p],node[np]);
	}
}



int main(void)
{

    Interface_init();

	write_gyro(0x1E, 1); // 지자기 OFF
	write_gyro(0x1E, 5); // 자이로 500dps	
	write_gyro(0x1E, 20); // 능동적 진동성분제거 OFF

	write_gyro(0x1E, 16); // 자이로 보정

	Camera_init();
	Setting(11);
	TM(50);
	V3

	_delay_ms(500);

	set

	LED_ON(3);
	_delay_ms(100);
	LED_OFF(3);

	while(1)
	{		
	    if(SW1)
		{
			key--;
			LED_ON(3);
			_delay_ms(100);
			LED_OFF(3);

		}
	    if(SW3)
		{
			key++;
			LED_ON(3);
			_delay_ms(100);
			LED_OFF(3);

		}
		
		
	    if(SW2)
		{
			lcd_clear_screen();
			LED_ON(3);
			_delay_ms(100);
			LED_OFF(3);

			switch(key)
			{

case 0:
case 1:


stage = 0;
for(int i = 0,val = 0; i < 13 ; i++){
	if(!map[i]) map[i] = val--;
}

for(int i = 10 ; i < 12 ; i++)
	if(map[i] <= 0) nowPos = i;
	

printf("%d",nowPos);
memset(hist,0,sizeof(hist));

for(int i = 0 ; i < 40 ; i+=3){
	curDp = i;
	flg = 0;
	Sort(-1,nowPos,0,map);
	if(flg) break;
}



printf("flg : %d\n",flg);
printf("min : %d\n",curDp);
dis(map);

for(int i = 0 ; i < curDp ; i++){
	//printf("%d -> %d",g_route[i],g_route[i+1]);
	swap(map[g_route[i]],map[g_route[i+1]]);
	//dis(map);
}

memset(hist,0,sizeof(hist));
nowPos= g_route[curDp];
stage = 1;
for(int i = 0 ; i < 40 ; i+=3){
	curDp = i;
	flg = 0;
	Sort(-1,nowPos,0,map);
	if(flg) break;
}

printf("flg : %d\n",flg);
printf("min : %d\n",curDp);
dis(map);
for(int i = 0 ; i < curDp ; i++){
	//printf("%d -> %d",g_route[i],g_route[i+1]);
	swap(map[g_route[i]],map[g_route[i+1]]);
	//dis(map);
}

LED_ON(3);
_delay_ms(1000);
LED_OFF(3);



while(!SW2);
while(SW2);

			}
		}

		if(key){
			lcd(0,0,"%03d %03d %03d %03d %03d",g_psd[2],g_psd[1],g_psd[0],g_psd[8],g_psd[7]);
			lcd(1,0,"%03d %03d     %03d %03d",g_psd[3],g_psd[4],g_psd[5],g_psd[6]);
		}
		else{
			lcd(0,0,"%03d %03d %03d %03d %03d",psd[2],psd[1],psd[0],psd[8],psd[7]);
			lcd(1,0,"%03d %03d     %03d %03d",psd[3],psd[4],psd[5],psd[6]);
		}
		key_puck = abs(key) %9;

		get_gyro();
		lcd(2,0,"C%d X%3d Y%3d S%2d",key_puck,Cmd(key_puck,102),Cmd(key_puck,103),Cmd(key_puck,104));		
		lcd(3,0,"%d%d%d%d%d| G%04d",IR_4,IR_1,IR_2,IR_3,IR_5,(int)gyro);
		lcd(3,16,"k:%02d",key);
	}
}

