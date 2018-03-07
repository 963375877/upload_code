//�õ�ϰ���ǳɹ��Ļ���



#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pthread.h>
#include <fcgiapp.h>
#include <sys/time.h>  
#include <time.h>
#include <mysql/mysql.h>

#include "qsi.h"
#include "cJSON.h"

#define UPLOAD_LOG_MODULE "cgi"
#define UPLOAD_LOG_PROC   "upload"

#define TEMP_BUF_MAX_LEN 51200
#define FILE_NAME_LEN 256
#define USER_NAME_LEN 256

#define THREAD_COUNT 5  
static int counts[THREAD_COUNT];

#define _HOST_ "168.160.111.26"		//���ݿ����ڷ�����ip
#define _USER_ "tbs"  				//���ݿ��û�
#define _PASSWD_ "tbs"
#define _DBNAME_ "fileInfo"


//FCGX_Request request;
static pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER;


/*
	���ܣ���ȡ�Ѿ��ϴ�������ļ��б�,���ļ�����MD5ֵ��
	������
		filePath 	�ļ�·��
		folderPath  �ļ���·��
	����ֵ���ɹ�0 ʧ�ܷ���-1
*/
int getChunkList(char *filePath, char *folderPath, FCGX_Request request)
{
	/*
		��Ŀ¼�µ������ļ����һ���ļ��б����ص�ǰ��	
	*/
	char fullpath[256];
	memset(fullpath, 0x00, 256);
	sprintf(fullpath, "%s/%s", filePath, folderPath);
		
    //����һ���ļ�jSON����
    cJSON * allfile = cJSON_CreateArray();    
    cJSON * fullnode= cJSON_CreateObject();

    char * out = NULL;
    if(NULL == fullpath)
    {       
        FCGX_FPrintF(request.out,"%s", "enter path null!\n");
        return -1;
    }

    DIR *dirp = opendir(fullpath);	//��Ŀ¼
    if(dirp == NULL)
	{
        FCGX_FPrintF(request.out,"%s", "open dir err");
        return -1;
    }
        
	struct dirent * dentp = NULL;
	
	//��ȡĿ¼���ļ�
    while((dentp = readdir(dirp)))
    {	      	
    	if((strcmp(".", dentp->d_name)) == 0 || (strcmp("..", dentp->d_name)) == 0)
        {
        	continue;
        }
    	
        //����Ŀ¼�ļ�
        if(dentp->d_type == DT_REG)
    	{           
            struct stat filestruct;           
            char newfilepath[256];
            sprintf(newfilepath, "%s/%s", fullpath, dentp->d_name);
            stat(newfilepath, &filestruct);
            
            //����һ��jSON����
            cJSON *chunkList = NULL;            
            if(strcmp(folderPath,dentp->d_name ) == 0)
            {
            	chunkList = cJSON_CreateObject();
            	cJSON_AddStringToObject(chunkList, "filename", dentp->d_name);
            	FCGX_FPrintF(request.out,"%s", cJSON_Print(chunkList));
            	return 0;
            }
            else
            {
            	cJSON_AddItemToArray(allfile, chunkList = cJSON_CreateObject());
            	cJSON_AddStringToObject(chunkList, "filename", dentp->d_name); 
            	cJSON_AddNumberToObject(chunkList, "indexof", atoi(dentp->d_name));             	
            }                                                              	                      
       	}   
       	else
       	{
       		return 0;	    		
       	}      		       
    }	
    cJSON_AddItemToObject(fullnode, "chunkList", allfile);  	 	
    FCGX_FPrintF(request.out,"%s", cJSON_Print(fullnode));
	return 0;	
}

/*
	�ж��ļ����Ƿ���ڣ��ļ��в����ڴ����ļ��У�
	�ļ��д����ж�����������Ƿ���ȷ��
	����ļ����ھ���֯�ļ���json��ʽ���ظ�ǰ�ˣ�
	����ļ������ھͷ���Ŀ¼�µ������ļ����ظ�ǰ��
*/

/*
	���ܣ��ж��ļ����Ƿ���ڣ������ڴ���һ���ļ���
	
	������
		filepath   �ļ���·��
		filename   �ļ�������
	
	����ֵ���ɹ�����0 ʧ�ܷ���-1
*/

int folderIsExit(char *filepath, char *filename, FCGX_Request request)
{		
	char fullpath[256];
	memset(fullpath, 0x00, sizeof(fullpath));
	
	sprintf(fullpath, "%s/%s", filepath, filename);
	
	if(access(fullpath, W_OK) == 0)
	{		
		//Ŀ¼��������д��Ȩ��	����Ŀ¼�������ļ���֯�ļ�����
		getChunkList(filepath, filename,request);	
			
	}
	else
	{
		//Ŀ¼�����ڻ�û��д��Ȩ��	
		 mkdir(fullpath, S_IRWXU|S_IRWXG|S_IRWXO); //0777
		 cJSON *fileinfo = cJSON_CreateObject();
         cJSON_AddStringToObject(fileinfo, "fileName", filename);       
		 FCGX_FPrintF(request.out,"%s", cJSON_Print(fileinfo));
	}	
	return 0;	
}


/*
	���ܣ���ָ�����ȵ��ַ����в����ַ���
	������
		(in)	full_data   		 �����ַ���
		(in)	full_data_len        ���ҳ��ȳ���
		(in)	substr               Ҫ���ҵ��ַ���
	����ֵ���ɹ�����Ҫ���ҵ��ַ����ĳ�ʼλ�� ʧ�ܷ���NULL	
*/
char* memstr(char* full_data, int full_data_len, char* substr)
{
    if (full_data == NULL || full_data_len <= 0 || substr == NULL)
    {
        return NULL;
    }

    if (*substr == '\0')
    {
        return NULL;
    }

    int sublen = strlen(substr);
    char* cur = full_data;
    int last_possible = full_data_len - sublen + 1;
    int i = 0;
    for (i = 0; i < last_possible; i++)
    {
        if (*cur == *substr)
        {
            if (memcmp(cur, substr, sublen) == 0)
            {
                // found
                return cur;
            }

        }
        cur++;
    }
    return NULL;
}



/**
 * @brief  �����ϴ���post���� ���浽������ʱ·��
 *         ͬʱ�õ��ļ��ϴ��ߡ��ļ����ơ��ļ���С
 *
 * @param len       (in)    post���ݵĳ���
 * @param user      (out)   �ļ��ϴ���
 * @param file_name (out)   �ļ����ļ���
 * @param p_size    (out)   �ļ���С
 *
 * @returns
 *          0 succ, -1 fail
 */
int recv_save_file(long len, char *user, char *filename, long *p_size, FCGX_Request request, char *filepath)
{	
	char log[526] = {0};
	memset(log,0x00, 526);
    int ret = 0;
    char *file_buf = NULL;
    char *begin = NULL;
    char *p, *q, *k;
    
     int fd = 0;
	int ret2 = 0;
	
	char *heto= NULL;
	char *to = NULL;
		 
	int totallen = 0;
	char * toen = NULL;
	
	int num = 0;
	 
	int FragmentName = 0;	
	char FragmentNum[50];
	
	char FPATH[256] = {0};
	
    char content_text[TEMP_BUF_MAX_LEN] = {0}; 		//�ļ�ͷ����Ϣ
    char boundary[TEMP_BUF_MAX_LEN] = {0};     		//�ֽ�����Ϣ

    //==========> ���ٴ���ļ��� �ڴ� <===========
    file_buf = (char *)malloc(len);
    memset(file_buf,0x00, sizeof(file_buf));
    
    if (file_buf == NULL)
    {
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "malloc error! file size is to big!!!!\n");
        goto END;
    }
	  	
	  for(int i=0; i<len; i++)   //��ȡ��׼����
	  {
	  	file_buf[i] = FCGX_GetChar(request.in);	  		
	  }
	  	  	
	//ret2 = fread(file_buf, 1, len, stdin); //�ӱ�׼����(web������)��ȡ����
	//if(ret2 == 0)
	//{    	
	//	LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "fread(file_buf, 1, len, stdin) err\n");
	//	ret = -1;
	//	goto END;
	//}
  
    //===========> ��ʼ����ǰ�˷��͹�����post���ݸ�ʽ <============
    begin = file_buf;    //�ڴ����
    p = begin;

    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       �������ļ�����\r\n
       ------WebKitFormBoundary88asdgewtgewx
    */

    //get boundary �õ��ֽ���, ------WebKitFormBoundary88asdgewtgewx
    p = strstr(begin, "\r\n");
    if (p == NULL)
    {
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"wrong no boundary!\n");
        ret = -1;
        goto END;
    }

 		
    //�����ֽ���
    strncpy(boundary, begin, p-begin);
    boundary[p-begin] = '\0';   //�ַ���������
    //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"boundary: [%s]\n", boundary);
    //sprintf(boundary, "ret2 = %d\n", ret2);
       
    p += 2;//\r\n
    //�Ѿ�������p-begin�ĳ���
    len -= (p-begin);

    //get content text head
    begin = p;

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    p = strstr(begin, "\r\n");
    if(p == NULL)
    {
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"ERROR: get context text error, no filename?\n");
        ret = -1;
       goto END;
    }
    strncpy(content_text, begin, p-begin); 				//����Content-Disposition��һ������
    content_text[p-begin] = '\0';
    //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"content_text: [%s]\n", content_text);

    p += 2;//\r\n
    len -= (p-begin);

    //========================================��ȡ�ļ��ϴ���
    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n                                   ��
    q = begin;
    q = strstr(begin, "name=");
    

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n                                     ��
    q += strlen("name=");
    q++;    //������һ��"

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n                                          ��
    k = strchr(q, '"');
    strncpy(user, q, k-q);  //�����û���
    user[k-q] = '\0';
    
	//FCGI_printf("user = %s\n" , user);
    //ȥ��һ���ַ������ߵĿհ��ַ�
    //trim_space(user);   //util_cgi.h

    //========================================��ȡ�ļ�����
    //"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    //   ��
    
    begin = k;
    q = begin;
    q = strstr(begin, "filename=");

    //"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    //             ��
    q += strlen("filename=");
    q++;    //������һ��"

    //"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n                    ��
    k = strchr(q, '"');
    strncpy(filename, q, k-q);  //�����ļ���
    filename[k-q] = '\0';
 	 	
    begin = p;
    p = strstr(begin, "\r\n");
    p += 4;//\r\n\r\n
    len -= (p-begin);

    //��������ļ�����������
    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       �������ļ�����\r\n
       ------WebKitFormBoundary88asdgewtgewx
    */

    begin = p;
    
    //find file's end
    p = memstr(begin, len, boundary);//util_cgi.h�� ���ļ���β
    if (p == NULL)
    {
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "memstr(begin, len, boundary) error\n");
        ret = -1;
       	goto END;
    }
    else
    {
        p = p - 2;//\r\n
    }
	/*	
		------WebKitFormBoundaryqeJJMxl4wwu9T8YL
		Content-Disposition: form-data; name="total"

		295
		------WebKitFormBoundaryqeJJMxl4wwu9T8YL
		Content-Disposition: form-data; name="index"

		0	
	*/

	//��������ļ���� ��ȡ��Ƭ��
	 heto = p + 2;
	 to = NULL;
	 to = strstr(heto, "\r\n");
	 to += 4;
	 
	 totallen = 0;
	 toen = strstr(to, "\r\n");
	 	 
	 toen += 4;
	 to = toen;
	 toen = strstr(to, "\r\n");
	 
	 totallen = toen - to;
	 	 
	 memset(FragmentNum, 0x00, sizeof(FragmentNum));
	 strncpy(FragmentNum, to, toen - to);  				//��ȡƬ������
	
	 num = atoi(FragmentNum);
	 toen+= 2;
	 to = toen;
	 
	 toen = strstr(to, "\r\n");
	 toen += 2;
	 
	 to = toen;
	 toen = strstr(to, "\r\n");
	 
	 toen += 4;
	 to = toen;
	 toen = strstr(to, "\r\n");
	 	 
	 memset(FragmentNum, 0x00, sizeof(FragmentNum));
	 strncpy(FragmentNum, to, toen - to);  
	 FragmentName = atoi(FragmentNum);
	
	 
    //begin---> file_len = (p-begin)

    //=====> ��ʱbegin-->p����ָ����������post���ļ�����������
    //======>������д���ļ���,�����ļ���Ҳ�Ǵ�post���ݽ�������  <===========
	
	 
	  //fd = open(filename, O_CREAT|O_WRONLY, 0644);
	  
	  sprintf(FPATH, "%s/%s", filepath, FragmentNum);
	   	  
	  fd = open(FPATH, O_CREAT|O_WRONLY, 0644);
	  
	  if (fd < 0)
	  {
	      //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"open %s error\n", filename);
	      ret = -1;
	      goto END;
	  }
	
	  //ftruncate�Ὣ����fdָ�����ļ���С��Ϊ����lengthָ���Ĵ�С
	  ftruncate(fd, (p-begin));
	  write(fd, begin, (p-begin));
	  close(fd);
	
END:
    free(file_buf);
    FCGX_FFlush(request.in);
    return ret;
}

/*
	���ܣ���ȡ�ļ���С
	������
		path  �ļ�·��
	����ֵ���ɹ������ļ���С ʧ�ܷ���-1
*/
int getfilesize(char *path)
{
	FILE *pf = fopen(path, "rb");
	if (pf==NULL)
	{
		return -1;
	} 
	else
	{
		fseek(pf, 0, SEEK_END);
		int size = ftell(pf);
		fclose(pf);
		return size;
	}
}



/*
	���ܣ�������ļ��ϲ�Ϊһ���ļ�, ͬʱɾ��Ƭ���ļ�
	������
		part_f_path  Ƭ���ļ�·��
		filepath     �ϲ����ļ�·��
		filename	 �ϲ����ļ���
	����ֵ�� �ɹ����� 0  ʧ�ܷ���-1

*/
int  merge(char *part_f_path , int part_num, char *filepath, char *filename, FCGX_Request request)
{
	char newpath[part_num][600]; 				//����·��	
	for(int i=0; i<part_num; i++)
	{
		memset(newpath[i], 0x00, sizeof(newpath[i]));			
	}
	
	char fullpath[500] = {0};
	memset(fullpath, 0x00, sizeof(fullpath));
	sprintf(fullpath, "%s/%s",filepath, filename);
	for (int i = 0; i < part_num; i++)
	{
		sprintf(newpath[i], "%s/%d", part_f_path, i);
	}	
		
	FILE *pfw = fopen(fullpath, "wb");
	for (int i = 0; i < part_num; i++)
	{
		int length = getfilesize(newpath[i]);
		if (length != -1)
		{
			FILE *pfr = fopen(newpath[i], "rb"); //��ȡ
			for (int j = 0; j < length; j++)
            {
				char ch = fgetc(pfr);
				fputc(ch, pfw);   				//д��
            }
			fclose(pfr);
			remove(newpath[i]);
		}
	}
	fclose(pfw);
	return 0;
}


/*
	���ܣ�дlog�ļ�
	������
		logPath		log�ļ�����·��
		logName		log�ļ���
		log			Ҫд������
		len			Ҫд�����ݳ���
		level       ����ȼ�
			#define DBG_INFOR       0x01  // call information  
			#define DBG_WARNING     0x02  // paramters invalid,    
			#define DBG_ERROR       0x04  // process error, leading to one call fails  
			#define DBG_CRITICAL    0x08  // process error, leading to voip process can't run exactly or exit  
	����ֵ���ɹ�����0 ʧ�ܷ���-1

*/

int write_log(char *logName, char * log, int len)
{
		char logPath[500];
		memset(logPath,0x00, sizeof(logPath));
		sprintf(logPath, "/home/tbs/TBSNative/TBSUploadFile/log/%s", logName);
		        	
    	time_t timer;     //time_t����long int ����
		struct tm *tblock;
		timer = time(NULL);
		tblock = localtime(&timer);
		FCGI_FILE *fp = NULL;	
		char time[500];
		memset(time,0x00, sizeof(time));
		sprintf(time, "%s", asctime(tblock));
		if(access(logPath, W_OK) != -1)
		{
			char n[] = "//n";
			FCGI_fopen(logPath, "ab+");	
			FCGI_fwrite(time, 1, strlen(time), fp);	
			FCGI_fwrite(log, 1, len, fp);
			FCGI_fwrite(n, 1, strlen(n), fp);	
			FCGI_fclose(fp);
		}
		else
		{		
			char n[] = "//n";	
			fp = FCGI_fopen(logPath, "wb+");
			FCGI_fwrite(time, 1, strlen(time), fp);	
			FCGI_fwrite(log, 1, len, fp);
			FCGI_fwrite(n, 1, strlen(n), fp);	
			FCGI_fclose(fp);				
		}
		return 0;
}
	

/*
	�̴߳�����
*/
static void *doit(void *a)
{
    int rc, i, thread_id;
    thread_id = (int)((long)a);
    pid_t pid = getpid();
    char *server_name;   
    
    int count = 0;
	int flag = 0;
	
	long len;
    int ret = 0;	
    
    char filename[256] = {0}; //�ļ���
    char user[56] = {0};      //�ļ��ϴ���
    long size = 0;
    	
	char get_filePath[20] = "filepath";
	char get_fileName[20] = "filename";
	char get_fileMd5Value[56] = "fileMd5Value";
	char get_chunks[56] = "chunks";
		
	FCGX_Request request;		
    FCGX_InitRequest(&request, 0, 0); //��ʼ��request
	
    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_mutex_t counts_mutex = PTHREAD_MUTEX_INITIALIZER;

        /* Some platforms require accept() serialization, some don't.. */        
        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request); 
       	pthread_mutex_unlock(&accept_mutex);
        if (rc < 0)
            break;
            		
        server_name = FCGX_GetParam("SERVER_NAME", request.envp); 						//�ӻ��������л�ȡSERVER_NAME��                              
        FCGX_FPrintF(request.out,"Content-type: text/html; charset=UTF-8\r\n\r\n");				   	 
		char *contentLength = FCGX_GetParam("CONTENT_LENGTH", request.envp);	 		//��ȡ��������	
		char *querystring = FCGX_GetParam("QUERY_STRING", request.envp);							
		char name[256] = "method";
		
		char *method = get_query_string(querystring, name);								
		char *filename = get_query_string(querystring, get_fileName);
		char *fileMd5Value = get_query_string(querystring, get_fileMd5Value);	
		char *chunks = 	get_query_string(querystring, get_chunks);	
													
		if(strcasecmp(method, "check") == 0)
		{	
			char filepath[256] = "../file";																				 									
			folderIsExit(filepath, fileMd5Value,request);
		}				
		else if(strcasecmp(method, "merge") == 0)
		{		
			 char filepath[600] ={0};	
			 int int_chunks  = atoi(chunks);	
	         sprintf(filepath, "../file/%s", fileMd5Value);	         
	         merge(filepath , int_chunks, filepath, filename, request);		               	
		}					
		else if(strcasecmp(method, "upload") == 0)
		{									
			if (contentLength != NULL)
	        {	        	
	        	time_t timer;     							//time_t����long int ����
      			struct tm *tblock;
      			timer = time(NULL);
      			tblock = localtime(&timer);
	        	
	        	len = strtol(contentLength, NULL, 10); 		//�ַ���תlong�� ����atol
	        	cJSON *fileinfo = cJSON_CreateObject();
	        	cJSON_AddStringToObject(fileinfo, "fileName", filename);
	        	cJSON_AddStringToObject(fileinfo, "fileMd5Value", fileMd5Value);
	        	cJSON_AddNumberToObject(fileinfo, "len", len);
	        	cJSON_AddStringToObject(fileinfo, "time", asctime(tblock));
	        	FCGX_FPrintF(request.out,"%s", cJSON_Print(fileinfo));	
	        	
				//1. init 
			    MYSQL*mysql = mysql_init(NULL);

			    if(mysql == NULL)
			    {
			    	char err[] = "init err";
			    	write_log(FCGX_GetParam("SERVER_NAME",request.envp),err,strlen(err));
			        exit(1);
			    }	
			    	
			    //2. real_connect
			    mysql = mysql_real_connect(mysql,_HOST_,_USER_,_PASSWD_,_DBNAME_,0,NULL,0);	
			    		    
			    if(mysql == NULL)
			    {
			        //printf("connect err\n");
			        exit(1);
			    }			    
			    const char * csname = "utf8";
			     char rSql[500]={0};
			     
			    mysql_set_character_set(mysql, csname);			   
			    sprintf(rSql,"insert into fileTable values('%s','%s', '%s', '%ld')", filename,fileMd5Value,asctime(tblock),len );

			    if(mysql_query(mysql,rSql) != 0)
			    {
			    	//printf("mysql_query err\n");
			    	exit(1);
			    }
			    
			    //3. close
			    mysql_close(mysql);	       	        	        		        		          
	        }
	        else
	        {
	            len = 0;
	        }						
			if (len <= 0)
	        {
	            FCGI_printf("No data from standard input\n");            
	            ret = -1;
	        }
	        else
	        {	    
	        	char filepath[600] ={0};	
	         	sprintf(filepath, "../file/%s", fileMd5Value);	         	        		        		    
	            if (recv_save_file(len, user, filename, &size, request, filepath) < 0)
	            {	            	
	                ret = -1;
	                return 0;
	            }		                      	                        	         	            
			}				
		}  				 				          
        pthread_mutex_lock(&counts_mutex);
       	++counts[thread_id];
        pthread_mutex_unlock(&counts_mutex);    
        FCGX_Finish_r(&request);
    }
    
    return NULL;
}

int main()
{
	long i;  
		
    pthread_t id[THREAD_COUNT];    
    FCGX_Init();  
  	
    for (i = 1; i < THREAD_COUNT; i++)  
    {
    	pthread_create(&id[i], NULL, doit, (void*)i);  
        pthread_detach(id[i]);  
    }    	  
  	doit(0); 	
  		  		  		
     	//FCGI_printf("%s", "GHGGGGHGHGHGH");   
    	//for (i = 1; i < THREAD_COUNT; i++)      	
        //		pthread_join(id[i], NULL); 
                             
	return 0;	
}