#include "common.h"

/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_admin_query(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    system("clear");
    int n;
	while(1){
		printf("*************************************************************\n");
		printf("******** 1：人名查找       2：查找所有         3：退出 ********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
                memset(&(msg->msgtype),0,sizeof(msg->msgtype));
                msg->msgtype = ADMIN_QUERY;
                strcpy(msg->recvmsg,"");
				printf("请输入要检索的人名：");
                scanf("%s",msg->info.name);
                //发送登陆请求
	            send(sockfd, msg, sizeof(MSG), 0);
	            //接受服务器响应
                recv(sockfd, msg, sizeof(MSG), 0);
                if(!strcmp(msg->recvmsg,"not found")){
                    printf("未检索到 %s\n",msg->info.name);
                    break;
                }
                printf("工号\t权限\t姓名\t密码\t年龄\t电话\t地址\t职位\t入职\t等级\t工资\n");
                printf("%d\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%.2f\n",msg->info.no,msg->info.usertype,\
                msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,\
                msg->info.date,msg->info.level,msg->info.salary);
				break;
			case 2:
                memset(&(msg->msgtype),0,sizeof(msg->msgtype));
                strcpy(msg->recvmsg,"");
                strcpy(msg->info.name,"");
                msg->msgtype = ADMIN_QUERY;
	            send(sockfd, msg, sizeof(MSG), 0);
                printf("工号\t权限\t姓名\t密码\t年龄\t电话\t地址\t职位\t入职\t等级\t工资\n");
                while(strcmp(msg->recvmsg,"finished") != 0){
                    memset(&(msg->info),0,sizeof(msg->info));
                    recv(sockfd, msg, sizeof(MSG), 0);
                    printf("%d\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%.2f\n",msg->info.no,msg->info.usertype,\
                    msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,\
                    msg->info.date,msg->info.level,msg->info.salary);
                }
				break;
			case 3:
                return;
			default:
				printf("您的输入有误，请重新输入\n"); 
		}
	}
}

/**************************************
 *函数名：admin_modification
 *参   数：消息结构体
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)//管理员修改
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    system("clear");
    int n = 0;
	char temp = 0;
	while(1){
        memset(&(msg->info),0,sizeof(msg->info));
        printf("请输入你要修改的职员工号：");
        scanf("%d",&(msg->info.no));
		printf("**********************请选择要修改的选项***********************\n");
		printf("******** 1：姓名   2：年龄    3：家庭住址     4：电话**********\n");
        printf("******** 5：职位   6：工资    7：入职年月     8：评级**********\n");
        printf("******** 9：密码   10：退出                         **********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
        msg->msgtype = ADMIN_MODIFY;
		switch(n)
		{
			case 1:
                printf("请输入新名字>>");
                scanf("%s",msg->info.name);   
                msg->flags = 1;
				break;
			case 2:
                printf("请输入新年龄>>");
                scanf("%d",&(msg->info.age));
                msg->flags = 2;
				break;
			case 3:
                printf("请输入新家庭地址>>");
                scanf("%s",msg->info.addr);
                msg->flags = 3;
                break;
            case 4:
                printf("请输入新电话号码>>");
                scanf("%s",msg->info.phone);
                msg->flags = 4;
				break;
			case 5:
                printf("请输入新职位>>");
                scanf("%s",msg->info.work);
                msg->flags = 5;
				break;
			case 6:
                printf("请输入新工资>>");
                scanf("%lf",&(msg->info.salary));
                msg->flags = 6;
                break;
			case 7:
                printf("请输入新入职年月>>");
                scanf("%s",msg->info.date);
                msg->flags = 7;
				break;
			case 8:
                printf("请输入新评级>>");
                scanf("%d",&(msg->info.level));
                msg->flags = 8;      
				break;
			case 9:
                printf("请输入新密码>>");
                scanf("%s",msg->info.passwd);
                msg->flags = 9;
                break;
            case 10:
				return;
			default:
				printf("您的输入有误，请重新输入\n"); 
		}
        send(sockfd, msg, sizeof(MSG), 0);
        recv(sockfd, msg, sizeof(MSG), 0);
        if(msg->flags==0){
            printf("修改成功,是否继续修改(Y/N)：\n");
			while(getchar()!=10);
			scanf("%c",&temp);
			while(getchar()!=10);
			if(temp == 'Y' || temp == 'y'){continue;}
			sleep(3);
			return;
		}
        if(msg->flags==-1)
            printf("修改失败\n");
	}
}

/**************************************
 *函数名：admin_adduser
 *参   数：消息结构体
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)//管理员添加用户
{		
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	while (1)
	{
		char temp = 0;
		system("clear");
		memset(&(msg->info),0,sizeof(msg->info));
		msg->msgtype = ADMIN_ADDUSER;
		printf("***************热烈欢迎新员工***************\n");
		printf("请输入工号：");
		scanf("%d",&(msg->info.no));
		while(getchar()!=10);
		printf("工号信息一旦录入无法更改，请确认您所输入的是否正确！(Y/N)：");
		scanf("%c",&temp);
		while(getchar()!=10);
		if(temp != 'Y' && temp != 'y'){return;}
		printf("请输入用户名：");
		scanf("%s",msg->info.name);
		printf("请输入用户密码：");
		scanf("%s",msg->info.passwd);
		printf("请输入年龄：");
		scanf("%d",&(msg->info.age));
		printf("请输入电话：");
		scanf("%s",msg->info.phone);
		printf("请输入家庭住址：");
		scanf("%s",msg->info.addr);
		printf("请输入职位：");
		scanf("%s",msg->info.work);
		printf("请输入入职日期：");
		scanf("%s",msg->info.date);
		printf("请输入评级(1~5,5为最高，新员工为 1)：");
		scanf("%d",&(msg->info.level));
		printf("请输入工资：");
		scanf("%lf",&(msg->info.salary));
		printf("是否为管理员：(Y/N)：");
		scanf("%c",&temp);
		while(getchar()!=10);
		if(temp!='Y' && temp!='y'){msg->info.usertype = 1;}
		else{msg->info.usertype = 0;}
		send(sockfd, msg, sizeof(MSG), 0);
        recv(sockfd, msg, sizeof(MSG), 0);
        if(msg->flags==0){
			printf("数据库修改成功!是否继续添加员工:(Y/N)：");
			scanf("%c",&temp);
			while(getchar()!=10);
			if(temp=='N' || temp=='n'){return;}
		}
        if(msg->flags==-1){
            printf("修改失败\n");
			printf("%s\n",msg->recvmsg);
			sleep(2);
		}
	}
	return;
}

/**************************************
 *函数名：admin_deluser
 *参   数：消息结构体
 *功   能：管理员删除用户
 ****************************************/
void do_admin_deluser(int sockfd,MSG *msg)//管理员删除用户
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	while (1)
	{
		char temp = 0;
		system("clear");
		memset(&(msg->info),0,sizeof(msg->info));
		msg->msgtype = ADMIN_DELUSER;
		printf("请输入要删除的用户工号：");
		scanf("%d",&(msg->info.no));
		
		send(sockfd, msg, sizeof(MSG), 0);
        recv(sockfd, msg, sizeof(MSG), 0);
        if(msg->flags==0){
			printf("用户 %d 删除成功!是否继续添加员工:(Y/N)：",msg->info.no);
			while(getchar()!=10);
			scanf("%c",&temp);
			while(getchar()!=10);
			if(temp=='N' || temp=='n'){return;}
		}
        if(msg->flags==-1){
            printf("修改 %d 失败\n",msg->info.no);
			printf("%s\n",msg->recvmsg);
			sleep(2);
		}
	}
	return;
}

/**************************************
 *函数名：do_history
 *参   数：消息结构体
 *功   能：查看历史记录
 ****************************************/
void do_admin_history (int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_HISTORY;
	msg->flags = 0;
	send(sockfd, msg, sizeof(MSG), 0);
	printf("时间\t员工\t操作\n");
	while(msg->flags != -1){
		memset(msg->recvmsg,0,sizeof(msg->recvmsg));
		recv(sockfd, msg, sizeof(MSG), 0);
		printf("%s\n",msg->recvmsg);
	}
	return;
}

/**************************************
 *函数名：admin_menu
 *参   数：套接字、消息结构体
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
    //printf("------------%s-----------%d.\n",__func__,__LINE__);
    int n;
	while(1){
		printf("*************************************************************\n");
		printf("******** 1：查询         2：修改           3：添加用户 ********\n");
        printf("******** 4：删除用户     5：查询历史记录    6：退出     ********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
                do_admin_query(sockfd,msg);
				break;
			case 2:
                do_admin_modification(sockfd,msg);
				break;
			case 3:
				do_admin_adduser(sockfd,msg);
                break;
			case 4:
				do_admin_deluser(sockfd,msg);
				break;
			case 5:
				do_admin_history(sockfd,msg);
				break;
			case 6:
				msg->msgtype = QUIT;
				msg->flags = 0;
				if(send(sockfd, msg, sizeof(MSG), 0)<0){
					return;
				}
				return;
			default:
				printf("您的输入有误，请重新输入\n"); 
		}
	}
}

/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_user_query(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	memset(&(msg->msgtype),0,sizeof(msg->msgtype));
	msg->msgtype = USER_QUERY;
	strcpy(msg->recvmsg,"");
	printf("请输入要检索的名字：");
	scanf("%s",msg->info.name);
	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	if(!strcmp(msg->recvmsg,"not found")){
		printf("未检索到 %s\n",msg->info.name);
		sleep(2);
		return;
	}
	printf("工号\t权限\t姓名\t密码\t年龄\t电话\t地址\t职位\t入职\t等级\t工资\n");
	printf("%d\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%.2f\n",msg->info.no,msg->info.usertype,\
	msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,\
	msg->info.date,msg->info.level,msg->info.salary);
	return;
}

/**************************************
 *函数名：do_modification
 *参   数：消息结构体
 *功   能：修改
 ****************************************/
void do_user_modification(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    int n;
	memset(&(msg->info),0,sizeof(msg->info));
	printf("请输入你要修改的职员工号：");
	scanf("%d",&(msg->info.no));
	printf("***********请输入要修改的选项(其他信息亲请联系管理员)***********\n");
	printf("******** 1：家庭住址    2：电话    3：密码    4：退出**********\n");
	printf("*************************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();
	msg->msgtype = USER_MODIFY;
	switch(n)
	{
		case 1:
			printf("请输入新家庭地址>>");
			scanf("%s",msg->info.addr);
			msg->flags = 1;
			break;
		case 2:
			printf("请输入新电话号码>>");
			scanf("%s",msg->info.phone);
			msg->flags = 2;
			break;
		case 3:
			printf("请输入新密码>>");
			scanf("%s",msg->info.passwd);
			msg->flags = 3;
			break;
		case 4:
			return;
		default:
			printf("您的输入有误，请重新输入\n"); 
	}
	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	if(msg->flags==0)
		printf("修改成功\n");
	if(msg->flags==-1){
		printf("修改失败\n");
	}
	sleep(2);
}

/**************************************
 *函数名：user_menu
 *参   数：消息结构体
 *功   能：管理员菜单
 ****************************************/
void user_menu(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	while(1){
		printf("*************************************************************\n");
		printf("********** 1：查询         2：修改           3：退出 **********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
		switch(n)
		{
			case 1:
                do_user_query(sockfd,msg);
				break;
			case 2:
                do_user_modification(sockfd,msg);
				break;
			case 3:
				msg->msgtype = QUIT;
				msg->flags = 0;
				if(send(sockfd, msg, sizeof(MSG), 0)<0){
					return;
				}
				return;
			default:
				printf("您的输入有误，请重新输入\n"); 
		}
	}
}

int admin_or_user_login(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	//输入用户名和密码
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();

	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0){
		if(msg->usertype == ADMIN){
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER){
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}
	return 0;
}


/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sockfd)
{	
	int n;
	MSG msg;
	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
		switch(n){
			case 1:
				msg.msgtype  = ADMIN_LOGIN;
				msg.usertype = ADMIN;
				break;
			case 2:
				msg.msgtype =  USER_LOGIN;
				msg.usertype = USER;
				break;
			case 3:
				msg.msgtype = QUIT;
				msg.flags = -1;
				if(send(sockfd, &msg, sizeof(MSG), 0)<0){
					perror("do_login send");
					return -1;
				}
				close(sockfd);
				exit(0);
			default:
				printf("您的输入有误，请重新输入\n"); 
		}
		admin_or_user_login(sockfd,&msg);
	}
}

int main(int argc, const char *argv[])
{
    if(argc < 3){
        ERR_MSG(" Parameter error");
        printf("need IP PORT\n");
        return -1;
    }
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 

	//创建网络通信的套接字
    int sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd<0){
        ERR_MSG("socket");
        return -1;
    }
    //允许端口快速重用
    int reuse = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))<0){
        ERR_MSG("setsockopt");
        return -1;
    }
	//填充网络结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[2]));//需要转成数字
    sin.sin_addr.s_addr = inet_addr(argv[1]);

	//连接服务器
	if(connect(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0){
		ERR_MSG("connect");
		return -1;
	}
    printf("connect success\n");
	do_login(sfd);
	close(sfd);
	return 0;
}
