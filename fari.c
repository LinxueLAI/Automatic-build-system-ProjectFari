#include <stdlib.h>
#include <stdio.h>  
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <json-c/json.h>
#include <glob.h>
#include <fcntl.h>
struct list{
	char file[50];
	struct list *previous;
	struct list *next;
};
struct listCH{
	struct list *CH;
	int nb;
	struct listCH *previous;
	struct listCH *next;
};
char * HFileToCFile(char * stre){
	char *strToken;	
	strToken = strtok(stre,".h");
	strcat(strToken,".c");
	return strToken;
}
char * HFileToOFile(char * stre){
	char *strToken;	
	strToken = strtok(stre,".h");
	strcat(strToken,".o");
	return strToken;
}
char * CFileToOFile(char * stre){
	char *strToken;
	strToken = strtok(stre,".");
	strcat(strToken,".o");
	return strToken;
}
char* formatdate(char* strs,time_t val){
	strftime(strs,36,"%d.%m.%Y %H:%M:%S",localtime(&val));
	return strs;
}
struct list *getListFromLine(struct list *T,char *buff, int *nb,json_object ** jarray){
	struct list *P;
	struct list *element;
	char *strToken,*sep;
	json_object * jstring;
	while((strToken = strtok(NULL," ")) &&( strcmp(strToken,"\n") !=0 )){
		sep = strtok_r(strToken," \n",&strToken);
		(*nb)++;
		element =(struct list *)malloc(sizeof(struct list));
		strcpy(element->file,sep);
		jstring = json_object_new_string(element->file);
		json_object_array_add(*jarray,jstring);
		element->next =NULL;
		if(T==NULL){
			element->previous = NULL;
			T = element;
			element =NULL;
		}else{
			P = T;
			while(P->next != NULL){
				P = P->next;
			}
			P->next = element;
		}
	}
	return T;
}
time_t TraiterHFile(struct list * HT){
	struct list *T;
	struct stat sb,sbMin;
	char fileName[100],min[100],date[36];
	int t;
	T=HT;
	strcpy(min,T->file);
	stat(T->file,&sb);
	sbMin = sb;
	while(T!=NULL){
		if(access(T->file,F_OK) == -1){
			printf("le fichier %s n'existe pas",T->file);	
			exit(1);
		}
		stat(T->file,&sb);
		//printf("\nMax = %s and st_mtime = %s \n",min,formatdate(date,sbMin.st_mtime));
		//printf("\nH = %s and st_mtime = %s \n\n",T->file,formatdate(date,sb.st_mtime));
		t =sbMin.st_mtime - sb.st_mtime;
		if(t<0){
			strcpy(min,T->file);
			sbMin = sb;
		}
		T=T->next;
	}
	return sbMin.st_mtime;
}
int traiterCFile(char *Cfile,time_t stMtime){
	char Ofile[100];
	int toc,toh;
	struct stat sb,sbo;
	stat(Cfile,&sb);
	if(access(Cfile,F_OK) == -1){
		printf("le fichier %s n'existe pas\n",Cfile);
		exit(1);
	}else{
		strcpy(Ofile,Cfile);
		strcpy(Ofile,CFileToOFile(Ofile));
		
		stat(Ofile,&sbo);
		
		toc = sbo.st_mtime - sb.st_mtime;
		toh = sbo.st_mtime - stMtime;
		
		if((toc<0) || (toh<0) || (access(Ofile,F_OK) == -1)){
			printf("\ntraiterCfile :  il faut recompiler le fichier %s\n",Cfile);
			return 1;
		}else{
			return 0;
		}	
	}
	
}
int execCmdC(char **command,char *Cfile,char *Ofile,int Fnb,struct list *FT,json_object ** jarray,json_object ** error_msg,int *errr,int kk){
	int j,i;
	struct list * T;
	pid_t pid;
	json_object * jstring;
	int status ;
	char S[1000],SS[10000],sep[100],*strToken;
	int fd[2];
	pipe(fd);
	strcpy(command[0],"gcc");
	T = FT;
	for(i =1;i< 1+Fnb;i++){
		strcpy(command[i],T->file);
		T = T->next;
	}	
	strcpy(command[i],"-c");
	i++;
	strcpy(command[i],Cfile);
	i++;
	strcpy(command[i],"-o");
	i++;
	strcpy(command[i],Ofile);
	i++;
	command[i]=NULL;
	if((pid=fork())==0){
		close(fd[0]);
		dup2(fd[1],2);
		if(execvp(command[0],command) ==-1){
			
			close(fd[1]);
			close(fd[0]);
			exit(1);
		}
		close(fd[1]);
		close(fd[0]);
		exit(0);
	}else{
		waitpid(pid,&status,0);
	}
	strcpy(S,command[0]);
	for(j=1;j<5+Fnb;j++){
		strcat(S," ");
		strcat(S,command[j]);
	}
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) != 0){
			strcpy(sep,json_object_to_json_string(*error_msg));
			if(strcmp(sep,"\"\"")!=0){
				strToken = strtok(sep,"\"");
			}else{
				strToken =(char *)malloc(3*sizeof(char));
				strToken[0]='\"';
				strToken[1]='\"';
				strToken[2]='\0';
			}
			if(strcmp(strToken,"")!=34){
				strcpy(SS,strToken);
			}else{
				strcpy(sep,"");
				strcpy(SS,sep);
				strcat(SS," error while executing the commands : ");
			}
			strcat(SS,S);
			strcat(SS," , ");
			fprintf(stderr,"...............Erreur de compilation.\n"/*de %s\n",S*/);
			*error_msg = json_object_new_string(SS);
			(*errr)=1;
			if(kk==0){
				exit(0);
			}
			
		}else{
			
			printf("%s\n",S);
			printf("\n");


			jstring = json_object_new_string(S);
			json_object_array_add(*jarray,jstring);
		}
	}	
	return 1;
}
struct list *Globbing(struct list *CT,int *Cnb){
	struct list *P=CT,*E,*L;
	int i;
	int match_count;
	glob_t glob_buffer;
	while(P!=NULL){
		glob( P->file , 0 , NULL , &glob_buffer );
		match_count = glob_buffer.gl_pathc;
		strcpy(P->file,glob_buffer.gl_pathv[0]);		
		E = P;
		P= P->next;
		for (i=1; i < match_count; i++){
			L=(struct list *)malloc(sizeof(struct list));
			(*Cnb)++;
			strcpy(L->file,glob_buffer.gl_pathv[i]);
			L->next = NULL;
			E->next = L;
			L->previous = E;
			E = L;
		}
		E->next = P;
		globfree( &glob_buffer );
	}
	return CT;
}
time_t getMaxMtimeO(struct list *CT){
	struct list *T;
	struct stat sbo;
	char fileO[100];
	int too;
	time_t maxT;
	T=CT;
	strcpy(fileO,T->file);
	strcpy(fileO,CFileToOFile(fileO));
	stat(fileO,&sbo);
	maxT = sbo.st_mtime;
	while(T!=NULL){
		strcpy(fileO,T->file);
		strcpy(fileO,CFileToOFile(fileO));
		stat(fileO,&sbo);
		too = sbo.st_mtime - maxT;
		if(too>0){
			maxT = sbo.st_mtime;
		}
		T=T->next;
	}
	return maxT;
}
int execCmdJ(char **command,int Jnb,struct list *JT,json_object **jarray,json_object **  error_msg,int *errr,int kk){
	int j,i;
	struct list * T;
	pid_t pid;
	json_object * jstring;
	int status ;
	char S[1000],SS[1000],sep[100];
	int fd[2];
	pipe(fd);
	T = JT;
	strcpy(command[0],"javac");

	for(i =1;i< 1+Jnb;i++){
		strcpy(command[i],T->file);
		T = T->next;
	}
	command[i]=NULL;
	if((pid=fork())==0){
		close(fd[0]);
		dup2(fd[1],2);
		if(execvp(command[0],command) ==-1){
			exit(1);
			close(fd[1]);
			close(fd[0]);
		}
		close(fd[1]);
		close(fd[0]);
		exit(0);
	}else{
		waitpid(pid,&status,0);		
	}
	
	strcpy(S,command[0]);
	for(j=1;j<1+Jnb;j++){
		strcat(S," ");
		strcat(S,command[j]);
	}
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) != 0){
			strcpy(sep,json_object_to_json_string(*error_msg));
			strcpy(SS,"error while executing the command : ");
			strcat(SS,S);
			*error_msg = json_object_new_string(SS);
			(*errr)=1;
			if(kk==0){
				exit(0);
			}
		}else{
			printf("%s\n",S);
			jstring = json_object_new_string(S);
			json_object_array_add(*jarray,jstring);
		}
	}	
	return 1;
}
int execCmdE(char **command,int Cnb,struct list *CT,int Fnb,struct list *FT,int Bnb,struct list *BT,struct list *ET,json_object ** jlinking,json_object ** error_msg,int *errr,int kk){
	struct list *T;
	int i;
	char fileO[100];
	json_object * jstring;
	char S[1000],SS[1000];
	int status ;
	int fd[2];
	pipe(fd);
	strcpy(command[0],"gcc");
	T = FT;
	for(i =1;i< 1+Fnb;i++){
		strcpy(command[i],T->file);
		T = T->next;
	}
	strcpy(command[i],"-o");
	i++;
	strcpy(command[i],ET->file);
	T = CT;
	for(i =3+Fnb;i< 3+Fnb+Cnb;i++){
		strcpy(fileO,T->file);
		strcpy(fileO,CFileToOFile(fileO));
		strcpy(command[i],fileO);
		T = T->next;
	}
	T = BT;
	for(i =3+Fnb+Cnb;i< 3+Fnb+Cnb+Bnb;i++){
		strcpy(command[i],T->file);
		T = T->next;
	}
	command[i]=NULL;
	if(fork()==0){
		close(fd[0]);
		dup2(fd[1],2);
		if(execvp(command[0],command)==-1){
			close(fd[1]);
			close(fd[0]);
			exit(1);
		}
		close(fd[1]);
		close(fd[0]);
		exit(0);
	}else{
		wait(&status);		
	}
	strcpy(S,command[0]);
	for(i=1;i<3+Cnb+Bnb+Fnb;i++){
		strcat(S," ");
		strcat(S,command[i]);
	}
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) != 0){
			strcpy(SS,"error while executing the command : ");
			strcat(SS,S);
			*error_msg = json_object_new_string(SS);
			(*errr)=1;
			if(kk==0){
				exit(0);
			}
		}else{	
			printf("%s\n",S);
			json_object_object_add(*jlinking,"command",json_object_new_string(S));
		}
	}
	return 0;
}
struct listCH *appendTo(struct listCH *HTT,struct list *CH,int *nb,int *HTTnb){
	struct listCH *P;
	struct listCH *element;
	(*HTTnb)++;
	element =(struct listCH *)malloc(sizeof(struct listCH));
	element->CH = CH;
	element->nb = (*nb);
	element->next =NULL;
	if(HTT==NULL){
		element->previous = NULL;
		HTT = element;
		element =NULL;
	}else{
		P = HTT;
		while(P->next != NULL){
			P = P->next;
		}
		P->next = element;
	}
	return HTT;
}
void traiterCHFile(struct listCH *HTT,int nb,int *Fnb,struct list *FT,json_object **jarray,json_object **error_msg,int *errr,int kk){
	struct listCH *PTT;
	int i,toh;
	struct stat sb,sbo;
	char **command;
	char Ofile[100];
	char Cfile[100];
	PTT = HTT;
	while(PTT!=NULL){
		if((access((PTT->CH)->file,F_OK) == -1) || (access(((PTT->CH)->next)->file,F_OK) == -1)){
			fprintf(stderr,"le fichier %s ou %s n'existe pas\n",(PTT->CH)->file,((PTT->CH)->next)->file);
			exit(1);
		}else{
			strcpy(Cfile,(PTT->CH)->file);
			strcpy(Ofile,CFileToOFile(Cfile));
			stat(((PTT->CH)->next)->file,&sb);
			stat(Ofile,&sbo);
			toh = sbo.st_mtime - sb.st_mtime;
			if((toh<0) || (access(Ofile,F_OK) == -1)){
				printf("\ntraiterCfile :  il faut recompiler le fichier %s\n",(PTT->CH)->file);
				command = (char **)malloc((7+(*Fnb))*sizeof(char*));
				for(i =0;i<7+(*Fnb);i++){
					command[i]=(char *)malloc(
					(strlen(Ofile)+30)*sizeof(char)
					);
				}
				execCmdC(command,(PTT->CH)->file,Ofile,*Fnb,FT,jarray,error_msg,errr,kk);
			}
		}
		PTT= PTT->next;
	}
}
int main(int argc,char *argv[])
{
	char FILENAME[100],*line_buf = NULL,*strToken,min[100],* error;
	size_t line_buf_size = 0;
	int line_count = 0,HTTnb=0,nb=0,Enb=0,Fnb=0,Bnb=0,EnbL=0,Hnb=0,Cnb=0,t,i,Jnb=0,errr=0,kk=0;
	ssize_t line_size;
	struct list *CT=NULL,*HT=NULL,*FT=NULL,*BT=NULL,*ET=NULL,*P,*JT = NULL,*CH=NULL;
	struct listCH *HTT=NULL,*PTT;
	struct stat sbo;
	char date[36],js[10000];
	char **command;
	time_t stMtime,stMtimeO;
	FILE *fp;
	int jsonFile;
	json_object * jstring, *error_msg1, *error_msg2, *tmp ,* jobj,* jarraySources,* jarrayHeaders, * jarrayLibraries,* jarrayFlags,* jarrayJava,* jarrayExec, * jcompilation, * jarrayCommands, * jlinking,* JarrayJavaFiles;
	jobj            = json_object_new_object();
	jarraySources   = json_object_new_array();
	jarrayHeaders   = json_object_new_array();
	jarrayLibraries = json_object_new_array();
	jarrayFlags     = json_object_new_array();
	jarrayJava	= json_object_new_array();
	JarrayJavaFiles	= json_object_new_array();
	jarrayExec      = json_object_new_array();
	jcompilation    = json_object_new_object();
	jarrayCommands  = json_object_new_array();
	jlinking        = json_object_new_object();
	error_msg1      = json_object_new_string("");
	error_msg2      = json_object_new_string("");
	json_object_object_add(jlinking,"command",json_object_new_string(""));
	jsonFile = open("logs.json",O_WRONLY | O_CREAT | O_TRUNC,0644);
	if (-1 == (int)jsonFile)
	{
		fprintf(stderr, "Error Creating file '%s'\n", "logs.json");
		return EXIT_FAILURE;
	}
	if(argc == 2){
		if(strcmp(argv[1],"-k")==0){
			strcpy(FILENAME,"farifile");
			kk=1;
		}else{
			strcpy(FILENAME,argv[1]);
		}
	}else if(argc == 3){
		if(strcmp(argv[1],"-k")==0){
			strcpy(FILENAME,argv[2]);
			kk=1;
		}else if(strcmp(argv[2],"-k")==0){
			strcpy(FILENAME,argv[1]);
			kk=1;
		}else{
			fprintf(stderr,"error in syntax\n");
			exit(1);
		}
	}else if(argc == 1){
		strcpy(FILENAME,"farifile");
	}else{
		fprintf(stderr,"error in syntax\n");
		exit(1);
	}
	fp = fopen(FILENAME, "r");
	if ((!fp) || (access(FILENAME,F_OK) == -1))
	{
		fprintf(stderr, "Error opening file '%s'\n", FILENAME);
		return EXIT_FAILURE;
	}
	line_size = getline(&line_buf, &line_buf_size, fp);
	while (line_size >= 0)
	{
		line_count++;
		strToken = strtok(line_buf," ");
		if(strcmp(strToken,"E")==0){
			EnbL++;
			if(EnbL == 1){
				ET = getListFromLine(ET,line_buf,&Enb,&jarrayExec);
			}else{
				printf("\nil y a plus d'une ligne E\n");
				return 1;
			}		
		}else if(strcmp(strToken,"C")==0){
			CT = getListFromLine(CT,line_buf,&Cnb,&jarraySources);
		}else if(strcmp(strToken,"H")==0){
			HT = getListFromLine(HT,line_buf,&Hnb,&jarrayHeaders);
		}else if(strcmp(strToken,"B")==0){
			BT = getListFromLine(BT,line_buf,&Bnb,&jarrayLibraries);
		}else if(strcmp(strToken,"F")==0){
			FT = getListFromLine(FT,line_buf,&Fnb,&jarrayFlags);
		}else if(strcmp(strToken,"J")==0){
			JT = getListFromLine(JT,line_buf,&Jnb,&JarrayJavaFiles);
		}else if(strcmp(strToken,"CH")==0){
			CH = getListFromLine(CH,line_buf,&nb,&JarrayJavaFiles);
			HTT = appendTo(HTT,CH,&nb,&HTTnb);
			nb = 0;
			CH = NULL;
		}
		line_size = getline(&line_buf, &line_buf_size, fp);
	}
	if(Cnb != 0 ){
		if(Hnb > 0){
			HT = Globbing(HT,&Hnb);
			stMtime = TraiterHFile(HT);
		}else{
			stMtime = 0;
		}
		if(HTTnb!=0){
			traiterCHFile(HTT,HTTnb,&Fnb,FT,&jarrayCommands,&error_msg1,&errr,kk);
		}
		CT = Globbing(CT,&Cnb);
		P=CT;
		while(P!=NULL){
			strcpy(line_buf,P->file);
			if(traiterCFile(line_buf,stMtime)){
				command = (char **)malloc((7+Fnb)*sizeof(char*));
				for(i =0;i<7+Fnb;i++){
					command[i]=(char *)malloc(
					(strlen(line_buf)+30)*sizeof(char)
					);
				}
				strcpy(line_buf,CFileToOFile(line_buf));
				execCmdC(command,P->file,line_buf,Fnb,FT,&jarrayCommands,&error_msg1,&errr,kk);	
			}
			P=P->next;
		}
		stMtimeO = getMaxMtimeO(CT);
		stat(ET->file,&sbo);
		if(((stMtimeO-sbo.st_mtime)>0)||(access(ET->file,F_OK) == -1)){
			command = (char **)malloc((4+Cnb+Bnb+Fnb)*sizeof(char*));
			
			for(i =0;i<4+Cnb+Bnb+Fnb;i++){
				command[i]=(char *)malloc(
				(strlen(line_buf)+30)*sizeof(char)
				);
			}
			execCmdE(command,Cnb,CT,Fnb,FT,Bnb,BT,ET,&jlinking,&error_msg2,&errr,kk);
		}
	}else{
		if(Jnb != 0){
			P = JT;
			command = (char **)malloc((1+Jnb)*sizeof(char*));
			for(i =0;i<=2+Jnb;i++){
				strcpy(line_buf,P->file);
				command[i]=(char *)malloc(
				(strlen(line_buf)+30)*sizeof(char)
				);
			}
			execCmdJ(command,Jnb,JT,&jarrayJava,&error_msg1,&errr,kk);
			jarrayCommands = jarrayJava;
		}
	}
	printf("\n2--------------------\n");
	//Sources
	json_object_object_add(jobj,"sources",jarraySources);
	//Headers
	json_object_object_add(jobj,"headers",jarrayHeaders);
	//Libraries
	json_object_object_add(jobj,"libraries",jarrayLibraries);
	//Flags
	json_object_object_add(jobj,"flags",jarrayFlags);
	//executable_name
	tmp = json_object_array_get_idx(jarrayExec,0);
	strcpy(line_buf,json_object_to_json_string(tmp));
	jstring = json_object_new_string(strtok(line_buf,"\"") );
	json_object_object_add(jobj,"executable_name",jstring);
	//Compilation
	json_object_object_add(jcompilation,"commands",jarrayCommands);
	json_object_object_add(jcompilation,"error_msg",error_msg1);
	json_object_object_add(jobj,"compilation",jcompilation);
	//Linkings
	json_object_object_add(jlinking,"error_msg",error_msg2);
	json_object_object_add(jobj,"linking",jlinking);
	//fari_msg
	if(errr == 1){
		jstring = json_object_new_string("error in compilation");
	}else{
		jstring = json_object_new_string("compilation finished");
	}
	printf("\n3--------------------\n");
	json_object_object_add(jobj,"fari_msg",jstring);
	strcpy(js,json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
	printf("the json object created: %s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
	write(jsonFile,js,strlen(js));
	//fprintf(jsonFile,json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
	
	close(jsonFile);
	
	line_buf = NULL;
	fclose(fp);
	if(errr == 1){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}



