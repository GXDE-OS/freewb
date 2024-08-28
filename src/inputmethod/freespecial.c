#include "freespecial.h"
#include <ctype.h>
const char *strMatch[11]={"〇","一","二","三","四","五","六","七","八","九","十"};
const char *strTMatch[11]={"","","二","三","四","五","六","七","八","九",""};
const char *strCHWeeK[7]={"星期日","星期一","星期二","星期三","星期四","星期五","星期六"};
const char *strENWeek[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
const char *strDW[9] = {"","十","百","千","万","十","百","千","亿"};
const char *strMoneyDW[9] = {"","拾","佰","仟","万","拾","佰","仟","亿"};
const char *strMoney[11] = {"零","壹","贰","叁","肆","伍","陆","柒","捌","玖","拾"};
char* matchTime(char* str){
    time_t now;
	struct tm *timenow;
	time(&now);
	timenow = localtime(&now);
//	printf("Local Time is:%s\n",asctime(timenow));
	int iyear = timenow->tm_year+1900;
	char year[50] = {0};
    
	int imonth = timenow->tm_mon+1;
	char month[50] = {0};

	int iday=timenow->tm_mday;
	char day[50] = {0};

	
	int ihour = timenow->tm_hour;
	char hour[50] = {0};

	int iminute = timenow->tm_min;
	char minute[50] = {0};


	int isecond = timenow->tm_sec;
	char second[50] = {0};

	int iweekday = timenow->tm_wday;
    char weekday[50] = {0};
    int i=0;
	char *CH = (char*) malloc(200);
    memset(CH,0,200);
    char* outputStr = CH;
		while(*str){
			if(*str=='$'){
				str++;
                if(*str == '0')
                    str++;
				switch (*str){
					case 'Y':
                        i=0;
                        for(int i=1000;i>0;i=i/10){
                            strcat(year, strMatch[iyear/i]);
                            iyear = iyear%i;
                        }
                        while(year[i]){
                            *CH++ = year[i++];
                        }
                        break;
                    case 'y':
                        i=0;
                        sprintf(year,"%d",iyear);
                        while(year[i]){
                            *CH++ = year[i++];
                        }
                        break;
					case 'M':
                        i=0;
						if(*(str+1)=='I'){
                            if(iminute>=10){	
                                strcat(minute,strTMatch[iminute/10]);
                                strcat(minute,"十");
                            }
                            if(iminute%10)
                                strcat(minute,strMatch[iminute%10]);
                            if(iminute==0)
                                strcat(minute,"零");
        
							str++;	
							while(minute[i]){
								*CH++ = minute[i++];
							}	       
						}
						else{
                            if(imonth>=10)
                                strcat(month,"十");
                            if(imonth!=10)
                                strcat(month,strMatch[imonth%10]);   
							while(month[i]){
								*CH++ = month[i++];
							}	       
						}
						break;
                    case 'm':
                        i=0;
						if(*(str+1)=='i'){
                            sprintf(minute,"%d",iminute);
							str++;	
							while(minute[i]){
								*CH++ = minute[i++];
							}	       
						}
						else{
                            sprintf(month,"%d",imonth); 
							while(month[i]){
								*CH++ = month[i++];
							}	       
						}
						break;
					case 'D':
                        if(iday>=10){	
                            strcat(day,strTMatch[iday/10]);
                            strcat(day,"十");
                        }
                        if(iday%10)
                            strcat(day,strMatch[iday%10]);
                        i=0;
						while(day[i]){
							*CH++ = day[i++];
						}
						break;
                    case 'd':
                        i=0;
                        sprintf(day,"%d",iday);
                        while(day[i]){
                            *CH++ = day[i++];
                        }
                        break;                        
					case 'H':
                        i=0;
                        if(ihour>=10){	
                            strcat(hour,strTMatch[ihour/10]);
                            strcat(hour,"十");
                        }
                        if(ihour%10)
                            strcat(hour,strMatch[ihour%10]);
                        if(ihour==0)
                            strcat(hour,"零");
					        while(hour[i]){
					       		*CH++ = hour[i++];
					        }
                        break;
                    case 'h':
                        i=0;
                        sprintf(hour,"%d",ihour);
                        while(hour[i]){
                            *CH++ = hour[i++];
                        }
                        break; 
                                
					case 'S':
                        i=0;
                        if(isecond>=10){	
                            strcat(second,strTMatch[isecond/10]);
                            strcat(second,"十");
                        }
                        if(isecond%10)
                            strcat(second,strMatch[isecond%10]);
                        if(isecond==0)
                            strcat(second,"零");
                        while(second[i]){
                            *CH++ = second[i++];
                        }
                        break;
                    case 's':
                        i=0;
                        sprintf(second,"%d",isecond);
                        while(second[i]){
                            *CH++ = second[i++];
                        }
                        break;   
                    case 'W':
                        i=0;
                        strcat(weekday,strCHWeeK[iweekday]);
                        while(weekday[i]){
                            *CH++ = weekday[i++];
                        }
                        break;
                    case 'w':
                        i=0;
                        strcat(weekday,strENWeek[iweekday]);
                        while(weekday[i]){
                            *CH++ = weekday[i++];
                        }
                        break;                        
					default:
						return outputStr;
                        
				}
				str++;
			}
			else{
				*CH++ = *str++;
			}
		}
	return outputStr;
}
int isPuncKey(FcitxKeySym sym)
{
    if(sym == '<'){
        return Key_Angle_bracket;
    }
    else if(sym == '\''){
        return Key_Point_number;
    }
    else if(sym == '\"'){  
        return Key_quotation_marks;
    }
    else if(sym == '{' ){  
        return Key_big_parantheses;
    }
    else if(sym == '[' ){  
        return Key_Bracket;
    }
    else if(sym == '(' ){      
        return Key_Parentheses;
    }
    else
        return -1;
}
void matchPunc(char* str,int index,boolean puncState)
{
    const char* puncKey[6] = {"<>","\'\'","\"\"","{}","[]","()"};
    const char* puncKeyNew[6] = {"《》","‘’","“”","{}","[]","（）"};
    const char **punc;
    if(puncState)
        punc = puncKeyNew;
    else
        punc = puncKey;
    strcat(str,*(punc+index));
}
void switchTempKey(char *str,boolean puncState){
    const char* puncKey[4] = {";",".",",","\\"};
    const char* puncKeyNew[4] = {"；","。","，","、"};
    if(!puncState)
        return;
    for(int i=0;i<4;i++){
        if(!strcmp(str,puncKey[i])){
            strcpy(str,puncKeyNew[i]);
            return;
        }
    }   
}
char* matchNumber(char* str){
    if(!str || !*str)
        return NULL;
    int i=0;
	char *outputStr = (char*) malloc(100);
    memset(outputStr,0,100);
    char *temp = str;
    while(*temp){
        if(!isdigit(*temp)){
            free(outputStr);
            return NULL;
        }
        temp++;
        i++;
        if(i>9){
            free(outputStr);
            return NULL;            
        }       
    }
    while(*str){
        if(*str=='0' && *(str+1)!='0' && i!=5)
            strcat(outputStr,"零");
        else if(*str!='0')
            strcat(outputStr,strMatch[*str - '0']);
        if(*str!='0'|| i==5)
            strcat(outputStr,strDW[i-1]);
        str++;
        i--;
    }
	return outputStr;    
}
char* matchNumberSim(char* str){
    if(!str || !*str)
        return NULL;
    int i=0;
	char *outputStr = (char*) malloc(100);
    memset(outputStr,0,100);
    while(*str){
        if(!isdigit(*str)){
            free(outputStr);
            return NULL;
        }       
        strcat(outputStr,strMatch[*str - '0']);
        str++;
        i++;
        if(i>9){
            free(outputStr);
            return NULL;            
        }        
    }
	return outputStr;     
}
char* matchMoney(char* str){
    if(!str || !*str)
        return NULL;
    int i=0;
	char *outputStr = (char*) malloc(100);
    memset(outputStr,0,100);
    char *temp = str;
    while(*temp){
        if(*temp == '.')
            break;
        if(!isdigit(*temp)){
            free(outputStr);
            return NULL;
        }
        temp++;
        i++;
        if(i>9){
            free(outputStr);
            return NULL;            
        }
    }
    while(*str){
        if(*str == '.'){
            break;
        }
        if(*str!='0' || (*(str+1)!='0' && i!=5) )
            strcat(outputStr,strMoney[*str - '0']);
        if(*str!='0'||i==5)
            strcat(outputStr,strMoneyDW[i-1]);
        str++;
        i--;
    }
    if(*str=='.'&&!*(++str)){
        free(outputStr);
        return NULL;        
    }
    strcat(outputStr,"圆");
    while(*str){
        if(!isdigit(*str) || i>=2){
            free(outputStr);
            return NULL;
        }
        strcat(outputStr,strMoney[*str - '0']);
        if(i==0)
            strcat(outputStr,"角");
        else
            strcat(outputStr,"分");
        str++;
        i++;
    } 

    char *ptr = strstr(outputStr,"零分");
    if(ptr) *ptr='\0';
    
    int len = strlen(outputStr);
    int len2=strlen("圆");

    ptr = strstr(outputStr,"零角");
    if(ptr)
    {
        if( strcmp(outputStr+len-len2,"角")==0 )
            *(outputStr+len-len2)='\0';
        else
            strcpy(ptr,ptr+2*len2);
        len=strlen(outputStr);
    }
    
    ptr=strstr(outputStr,"零圆");
    if(ptr)
    {
        strcpy(ptr,ptr+len2);
        len=strlen(outputStr);
    }

    ptr = strstr(outputStr,"圆");
    if(*(ptr+len2)!='\0')
    {
        char *ptr1 = strstr(ptr+len2,"角");
        char *ptr2 = strstr(ptr+len2,"分");
        if( !ptr1 && ptr2)
        {
            char str[128];
            strcpy(str,ptr+len2);
            *ptr='\0';
            strcat(ptr,"圆零");
            strcat(ptr,str);
            len=strlen(outputStr);
        }
    }
//puts(outputStr);

    if( strcmp(outputStr+len-len2,"圆")==0||strcmp(outputStr+len-len2,"角")==0)
        strcat(outputStr,"整");
    
	return outputStr;     
}
char* matchtimeManul(char* str ,int flg){
    if(!str || !*str)
        return NULL;
    if(flg>2 || flg<0)
        return NULL;
    int i=0;
    boolean hasYear = true;
	char *outputStr = (char*) malloc(100);
    memset(outputStr,0,100);
    char *temp = str;
    while(*str){
        if(*str == '.'){
            str++;
            break;
        }
        if(!isdigit(*str)||i>=4 ){
            free(outputStr);
            return NULL;
        }
        if(flg)
            strncat(outputStr,str,1);
        else
            strcat(outputStr,strMatch[*str - '0']);
        str++;
        i++;
    }
    if(i==3){
        free(outputStr);
        return NULL;        
    }
    if(i==2){
        if(*temp - '0'>1 || (*temp -'0'==1 && *(temp+1) - '0'>2)){
            free(outputStr);
            return NULL;            
        }
        if(*temp >='0' &&!flg){
            memset(outputStr,0,100);
            i=0;
            while(*temp){
                if(*temp == '.'){
                    temp++;
                    break;
                }
                if(*temp != '0'){
                    if(i==0){
                        if(*temp =='1')
                            strcat(outputStr,"十");
                    }
                    else
                        strcat(outputStr,strMatch[*temp - '0']);
                }
                temp++;
                i++;
            }
            
        }
    }
    if(i!=4){
        hasYear = false;
        if(flg!=2){
            strcat(outputStr,"月");
        }
        else
            strcat(outputStr,"-");
    }
    else{
        hasYear = true;
        if(flg!=2){
            strcat(outputStr,"年");
        }
        else
            strcat(outputStr,"-");        
    }
    
    
    i=0;
    temp = str;
    while(*str){
        if(*str == '.'){
            break;
        }        
        if(!isdigit(*str) || i>=2){
            free(outputStr);
            return NULL;
        }
        if(flg)
            strncat(outputStr,str,1);
        else{
            if(i==0&&isdigit(*(str+1))){
                if(*str>'1')
                    strcat(outputStr,strMatch[*str - '0']);  
                if(*str>'0')
                strcat(outputStr,"十");
            }
            else if(*str>'0')
                strcat(outputStr,strMatch[*str - '0']);  
        }
        str++;
        i++;
    } 
    if(i==2){
        if(hasYear &&(*temp - '0'>1 || (*temp -'0'==1 && *(temp+1)-'0'>2))){
            free(outputStr);
            return NULL;            
        }
        else if(!hasYear &&(*temp - '0'>3 || (*temp -'0'==3 && *(temp+1)-'0'>1))){
            free(outputStr);
            return NULL;            
        }
    }   
    if(hasYear){
        if(flg!=2){
            strcat(outputStr,"月");
        }
        else
            strcat(outputStr,"-");
    }
    else{
        if(*str == '.' && !*(str+1)){
            if(flg!=2){
                strcat(outputStr,"日");
            }     
            return outputStr;
        }
        else{
            free(outputStr);
            return NULL;              
        }
    }
    temp = ++str;
    i=0;
    while(*str){
        if(*str == '.'){
            break;
        }        
        if(!isdigit(*str) || i>=2){
            free(outputStr);
            return NULL;
        }
        if(flg)
            strncat(outputStr,str,1);
        else{
            if(i==0&&isdigit(*(str+1))){
                if(*str>'1')
                    strcat(outputStr,strMatch[*str - '0']);  
                if(*str>'0')
                strcat(outputStr,"十");
            }
            else if(*str>'0')
                strcat(outputStr,strMatch[*str - '0']);   
        }
        str++;
        i++;
    } 
    if(i==0||*str){
        free(outputStr);
        return NULL;          
    }
    if(i==2){
        if(*temp - '0'>3 || (*temp -'0'==3 && *(temp+1)-'0'>1)){
            free(outputStr);
            return NULL;            
        }
    }   
    if(flg!=2)
        strcat(outputStr,"日");
	return outputStr;       
}
