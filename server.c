#include "common.h"

sqlite3 *db;  //仅服务器使用
int maxfd = 0;
typedef void (*sighandler_t)(int);
void handler(int sig)
{
    printf("server close\n");
    sqlite3_close(db);
    for(int i=0;i<=maxfd;i++)
        close(i);
    exit(0);
}

int history_record(MSG *msg,char *words)
{
    char sql[900]={0},temp[150]={0};
    time_t t;
    struct tm *info = NULL;
    t = time(NULL);
    info = localtime(&t);
    sprintf(temp,"%04d-%02d-%02d %02d-%02d-%02d", info->tm_year+1900, info->tm_mon+1, info->tm_mday,\
                info->tm_hour, info->tm_min, info->tm_sec);//获取时间
    sprintf(sql,"insert into historyinfo values(\"%s\",\"%s\",\"%s\");",temp,msg->username,words);
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}
    return 0; 
}

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
    char words[500]={0};
	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
            sprintf(words,"%s %s 登陆成功",msg->usertype==0?"管理员":"普通用户",msg->username);
            history_record(msg,words);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[200] ={0};
    char words[500]={0};
    printf("%d    %s",msg->info.no,msg->info.addr);
    printf("1111\n");
    switch(msg->flags)
    {
        case 1:
            sprintf(sql,"update usrinfo set addr=\"%s\" where staffno=%d;",msg->info.addr,msg->info.no);
            sprintf(words,"%s %s 修改 %d 地址为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.addr);
            break;
        case 2:
            sprintf(sql,"update usrinfo set phone=\"%s\" where staffno=%d;",msg->info.phone,msg->info.no);
            sprintf(words,"%s %s 修改 %d 号码为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.phone);
            break;
        case 3:
            sprintf(sql,"update usrinfo set passwd=\"%s\" where staffno=%d;",msg->info.passwd,msg->info.no);
            sprintf(words,"%s %s 修改 %d 密码为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.passwd);
            break;
    }
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
        msg->flags = -1;
        strcpy(msg->recvmsg,errmsg);
        send(acceptfd,msg,sizeof(MSG),0);
	}else{
		msg->flags = 0;
        send(acceptfd,msg,sizeof(MSG),0);
        history_record(msg,words);
	}
    return 0; 
}

int process_user_query_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[DATALEN] = {0};
    char words[500]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn; 
    sprintf(sql,"select * from usrinfo where name=\"%s\";",msg->info.name);
    sprintf(words,"%s %s 检索用户 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.name);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);//错误
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"not found");
			send(acceptfd,msg,sizeof(MSG),0);//未检索到
		}else{
            for(int i=1;i<=nrow;i++){
                memset(&(msg->info),0,sizeof(msg->info));
                msg->info.no = atoi(result[i*ncolumn+0]);
                msg->info.usertype = atoi(result[i*ncolumn+1]);
                strcpy(msg->info.name,result[i*ncolumn+2]);
                strcpy(msg->info.passwd,result[i*ncolumn+3]);
                msg->info.age = atoi(result[i*ncolumn+4]);
                strcpy(msg->info.phone,result[i*ncolumn+5]);
                strcpy(msg->info.addr,result[i*ncolumn+6]);
                strcpy(msg->info.work,result[i*ncolumn+7]);
                strcpy(msg->info.date,result[i*ncolumn+8]);
                msg->info.level = atoi(result[i*ncolumn+9]);
                msg->info.salary = atof(result[i*ncolumn+10]);
                if(i==nrow){
                    strcpy(msg->recvmsg,"finished");
                }//判断最后一次
                send(acceptfd,msg,sizeof(MSG),0);
            }
		}
        history_record(msg,words);
	}
    sqlite3_free_table(result);
	return 0;	
}

int process_admin_modify_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[200] ={0};
    char words[500]={0};
    switch(msg->flags)
    {
    case 1:
        sprintf(sql,"update usrinfo set name=\"%s\" where staffno=%d;",msg->info.name,msg->info.no);
        sprintf(words,"%s %s 修改 %d 名字为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.name);
        break;
    case 2:
        sprintf(sql,"update usrinfo set age=%d where staffno=%d;",msg->info.age,msg->info.no);
        sprintf(words,"%s %s 修改 %d 年龄为 %d",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.age);
        break;
    case 3:
        sprintf(sql,"update usrinfo set addr=\"%s\" where staffno=%d;",msg->info.addr,msg->info.no);
        sprintf(words,"%s %s 修改 %d 地址为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.addr);
        break;
    case 4:
        sprintf(sql,"update usrinfo set phone=\"%s\" where staffno=%d;",msg->info.phone,msg->info.no);
        sprintf(words,"%s %s 修改 %d 号码为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.phone);
        break;
    case 5:
        sprintf(sql,"update usrinfo set work=\"%s\" where staffno=%d;",msg->info.work,msg->info.no);
        sprintf(words,"%s %s 修改 %d 工作为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.work);
        break;
    case 6:
        sprintf(sql,"update usrinfo set salary=%f where staffno=%d;",msg->info.salary,msg->info.no);
        sprintf(words,"%s %s 修改 %d 工资为 %lf",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.salary);
        break;
    case 7:
        sprintf(sql,"update usrinfo set date=\"%s\" where staffno=%d;",msg->info.date,msg->info.no);
        sprintf(words,"%s %s 修改 %d 入职为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.date);
        break;
    case 8:
        sprintf(sql,"update usrinfo set level=%d where staffno=%d;",msg->info.level,msg->info.no);
        sprintf(words,"%s %s 修改 %d 等级为 %d",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.level);
        break;
    case 9:
        sprintf(sql,"update usrinfo set passwd=\"%s\" where staffno=%d;",msg->info.passwd,msg->info.no);
        sprintf(words,"%s %s 修改 %d 密码为 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no,msg->info.passwd);
        break;
    }
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
        msg->flags = -1;
        strcpy(msg->recvmsg,errmsg);
        send(acceptfd,msg,sizeof(MSG),0);
	}else{
		msg->flags = 0;
        send(acceptfd,msg,sizeof(MSG),0);
        history_record(msg,words);
	}
    return 0; 
}

int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[900] ={0};
    char words[500]={0};
    sprintf(sql,"insert into usrinfo values(%d,%d,\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%lf);",msg->info.no,msg->info.usertype,\
    msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
    char *errmsg = NULL;
    sprintf(words,"%s %s 添加员工 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
        msg->flags = -1;
        strcpy(msg->recvmsg,errmsg);
        send(acceptfd,msg,sizeof(MSG),0);
	}else{
		msg->flags = 0;
        send(acceptfd,msg,sizeof(MSG),0);
        history_record(msg,words);
	}
    return 0; 
}

int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[200] ={0};
    char words[500]={0};
    sprintf(sql,"delete from usrinfo where staffno=%d;",msg->info.no);
    sprintf(words,"%s %s 删除员工 %d",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.no);
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
        msg->flags = -1;
        strcpy(msg->recvmsg,errmsg);
        send(acceptfd,msg,sizeof(MSG),0);
	}else{
		msg->flags = 0;
        send(acceptfd,msg,sizeof(MSG),0);
        history_record(msg,words);
	}
    return 0; 
}

int process_admin_query_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    //封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
    char words[500]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
    if(!strcmp(msg->info.name,"")){//全部显示
        sprintf(sql,"select * from usrinfo;");
        sprintf(words,"%s %s 检索所有员工",msg->usertype==0?"管理员":"普通用户",msg->username);
    }else{//名字检索
        sprintf(sql,"select * from usrinfo where name=\"%s\";",msg->info.name);
        sprintf(words,"%s %s 检索员工 %s",msg->usertype==0?"管理员":"普通用户",msg->username,msg->info.name);
    }
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);//错误
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"not found");
			send(acceptfd,msg,sizeof(MSG),0);//未检索到
		}else{
            for(int i=1;i<=nrow;i++){
                memset(&(msg->info),0,sizeof(msg->info));
                msg->info.no = atoi(result[i*ncolumn+0]);
                msg->info.usertype = atoi(result[i*ncolumn+1]);
                strcpy(msg->info.name,result[i*ncolumn+2]);
                strcpy(msg->info.passwd,result[i*ncolumn+3]);
                msg->info.age = atoi(result[i*ncolumn+4]);
                strcpy(msg->info.phone,result[i*ncolumn+5]);
                strcpy(msg->info.addr,result[i*ncolumn+6]);
                strcpy(msg->info.work,result[i*ncolumn+7]);
                strcpy(msg->info.date,result[i*ncolumn+8]);
                msg->info.level = atoi(result[i*ncolumn+9]);
                msg->info.salary = atof(result[i*ncolumn+10]);
                if(i==nrow){
                    strcpy(msg->recvmsg,"finished");
                }//判断最后一次
                send(acceptfd,msg,sizeof(MSG),0);
            }
		}
        sqlite3_free_table(result);
	}
    history_record(msg,words);
	return 0;	
}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[DATALEN] = {0};
    char words[500]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
    msg->flags=0;
    sprintf(sql,"select * from historyinfo;");
    sprintf(words,"%s %s 检索历史记录",msg->usertype==0?"管理员":"普通用户",msg->username);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);//错误
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"not found");
			send(acceptfd,msg,sizeof(MSG),0);//未检索到
		}else{
            for(int i=1;i<=nrow;i++){
                memset(msg->recvmsg,0,sizeof(msg->recvmsg));
                strncpy(msg->recvmsg,result[i*ncolumn+0],strlen(result[i*ncolumn+0]));
                strncat(msg->recvmsg," ",1);
                strncat(msg->recvmsg,result[i*ncolumn+1],strlen(result[i*ncolumn+1]));
                strncat(msg->recvmsg," ",1);
                strncat(msg->recvmsg,result[i*ncolumn+2],strlen(result[i*ncolumn+2]));
                if(i==nrow){
                    msg->flags=-1;
                }//判断最后一次
                send(acceptfd,msg,sizeof(MSG),0);
            }
		}
        history_record(msg,words);
	}
    sqlite3_free_table(result);
	return 0;
}

int process_client_quit_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char words[500]={0};
    if(msg->flags==0)
        sprintf(words,"%s %s 退出登录",msg->usertype==0?"管理员":"普通用户",msg->username);
    else
        sprintf(words,"客户端退出");
    history_record(msg,words);
}

int process_client_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    memset(msg->recvmsg,0,sizeof(msg->recvmsg));
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}
}

int updateMaxfd(int maxfd,fd_set readfds)
{
    for(int i=maxfd;i>=0;i--){
        if(FD_ISSET(i,&readfds)){
            return i;
        }
    }
    return 0;
}

int main(int argc, const char *argv[])
{
	//判断命令行参数	
	if(argc < 3){
        ERR_MSG("Parameter error");
        printf("need IP PORT\n");
        return -1;
    }
	//打开数据库
	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}
    char sql[200] = "create table if not exists usrinfo(staffno int primary key,usertype int,name char,passwd char,age int,phone char,addr char,work char,date char,level int,salary float);";
	//执行数据库操作
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}
    char sql1[200] = "create table if not exists historyinfo(time char,name char,words char);";
	//执行数据库操作
	if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create historyinfo table success.\n");
	}
	//socket->填充->绑定->监听->等待连接->数据交互->关闭

	//创建网络通信的套接字
    int sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd<0){
        ERR_MSG("socket");
        return -1;
    }
    //捕获2)信号
    sighandler_t s = signal(2, handler);
    if(SIG_ERR == s)
    {
        perror("signal");
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

	//绑定网络套接字和网络结构体
	if(bind(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0){
		ERR_MSG("bind");
		return -1;
	}
	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sfd,10) < 0){
		ERR_MSG("listen");
		return -1;
	}
	//通过select实现并发
    int acceptfd = 0;
    int recvbytes = 0;
    fd_set readfds,tempfds;//创建一个读集合
    FD_ZERO(&readfds);
    FD_ZERO(&tempfds);//清空读集合
    FD_SET(0,&readfds);//写事件
    FD_SET(sfd,&readfds);//socket

    maxfd = sfd;//设置最大文件描述符
    int retval = 0;
    struct sockaddr_in cin;
    socklen_t addrlen = sizeof(cin);
    MSG msg;

	while(1){
        tempfds = readfds;//重要，必须每次重新赋值
		retval =select(maxfd+1,&tempfds,NULL,NULL,NULL);
        char temp[20]={0};
		//判断是否是集合里关注的事件
        for(int i=0;i<=maxfd;i++){
            if(FD_ISSET(i,&tempfds) == 0){
                continue;
            }
            if(i == 0){
                scanf("%s",temp);
                while (getchar()!=10){};
                if((!strcmp(temp,"q"))||(!strcmp(temp,"quit"))||(!strcmp(temp,"exit"))){
                    printf("server close\n");
                    sqlite3_close(db);
                    for(int i=0;i<=maxfd;i++)
                        close(i);
                    exit(0);
                }
            }else if(i == sfd){
                //数据交互 
                acceptfd = accept(sfd,(struct sockaddr*)&cin,&addrlen);
                if(acceptfd == -1){
                    printf("acceptfd failed.\n");
                    exit(-1);
                }
                printf("%s:%d  connect success\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
                FD_SET(acceptfd,&readfds);//将newfd放入到读集合中
                maxfd = acceptfd>maxfd?acceptfd:maxfd;//更新maxfd
            }else{
                memset(&msg,0,sizeof(msg));//清零
                recvbytes = recv(i,&msg,sizeof(msg),0);
                if(recvbytes == -1){
                    printf("recv failed.\n");
                    continue;
                }else if(recvbytes == 0){
                    printf("%s: %d close connection\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
                    close(i);
                    FD_CLR(i, &readfds);  //删除集合中的i
                    maxfd = updateMaxfd(maxfd,readfds);
                }else{
                    process_client_request(i,&msg);
                }
            }
        }
	}
    sqlite3_close(db);
	close(sfd);

	return 0;
}
