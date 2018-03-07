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

#include "qsi.h"
#include "cJSON.h"

#define UPLOAD_LOG_MODULE "cgi"
#define UPLOAD_LOG_PROC   "upload"

#define TEMP_BUF_MAX_LEN 51200
#define FILE_NAME_LEN 256
#define USER_NAME_LEN 256




/*
	���ܣ�
	������
	����ֵ��

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
 * @param filepath  (out)   �ļ����·��
 * @param file_name (out)   �ļ����ļ���
 * @param p_size    (out)   �ļ���С
 *
 * @returns
 *          0 succ, -1 fail
 */
int wx_recv_save_file(long len, char *user, char * filepath, char *filename, long *p_size)
{	
	char log[526] = {0};
	memset(log,0x00, 526);
    int ret = 0;
    
    char fullpath[2000] = {0};
     memset(fullpath,0x00, 2000);
   
     
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
	
    char content_text[TEMP_BUF_MAX_LEN] = {0}; //�ļ�ͷ����Ϣ
    char boundary[TEMP_BUF_MAX_LEN] = {0};     //�ֽ�����Ϣ

    //==========> ���ٴ���ļ��� �ڴ� <===========
    file_buf = (char *)malloc(len);
    memset(file_buf, 0x00, len);
    
    if (file_buf == NULL)
    {

        goto END;
    }

    ret2 = FCGI_fread(file_buf, 1, len, stdin); //�ӱ�׼����(web������)��ȡ����
    if(ret2 == 0)
    {    	
        ret = -1;
        goto END;
    }
    
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
        ret = -1;
        goto END;
    }

    //�����ֽ���
    strncpy(boundary, begin, p-begin);
    boundary[p-begin] = '\0';   //�ַ��������� �õ��ֽ��� 
	
	
    p += 2;//\r\n
    //�Ѿ�������p-begin�ĳ���
    len -= (p-begin);

    //get content text head
    begin = p;

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    p = strstr(begin, "\r\n");
    if(p == NULL)
    {
       ret = -1;
       goto END;
    }
    
    strncpy(content_text, begin, p-begin); //����Content-Disposition��һ������
    content_text[p-begin] = '\0';

    p += 2;//\r\n
    len -= (p-begin);

    //========================================��ȡ�ļ��ϴ���
    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    //                                ��
    q = begin;
    q = strstr(begin, "name=");
    

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n                                     ��
    q += strlen("name=");
    q++;    //������һ��"

    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    //                                          ��
    k = strchr(q, '"');
    strncpy(user, q, k-q);  //�����û���
    user[k-q] = '\0';
    
    FCGI_printf("user = %s\n", user);
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
 	
 	
 	p+=2;	
    begin = p;
    p = strstr(begin, "\r\n");
    p += 2;//\r\n
    len -= (p-begin);
  
    begin = p;  
    
       
    p = strstr(begin, "\r\n");
    p+=2;
    //Content-Disposition: form-data; name="file"; 
    len -= (p-begin);  
   	
   	//----------------------------455617322038579812696183
    begin = p;
    char *f, *e; 
    f = strstr(begin, "name=");
    f+= strlen("name=");
    f++;
    e = strchr(f, '"');
    
    char newfile[256];
    strncpy(newfile, f, e - f);
    FCGI_printf("newfile = %s\n", newfile);
    
    p = strstr(begin, "\r\n");      
    p+=2;  
    
    len -= (p-begin);
    
    begin = p;
    p = strstr(begin, "\r\n");
    p+=4;
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
    len -= (p - begin); 
 
    p = memstr(begin, len, boundary);
   
    if (p == NULL)
    {      
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
	
	
	 sprintf(fullpath, "%s/%s",filepath, filename );
    //begin---> file_len = (p-begin)

    //=====> ��ʱbegin-->p����ָ����������post���ļ�����������
    //======>������д���ļ���,�����ļ���Ҳ�Ǵ�post���ݽ�������  <===========		 
	  fd = open(fullpath, O_CREAT|O_WRONLY, 0644);	  	  
	  
	  if (fd < 0)
	  {
	      ret = -1;
	      goto END;
	  }
	
	  //ftruncate�Ὣ����fdָ�����ļ���С��Ϊ����lengthָ���Ĵ�С
	  ftruncate(fd, (p-begin));
	  FCGI_write(fd, begin, (p-begin));
	  FCGI_close(fd);
	
END:
    free(file_buf);
    return ret;
}


		

int main ()
{
	int count = 0;
	int flag = 0;
	
	long len;
    int ret = 0;	
    
    char filename[256] = {0}; //�ļ���
    char user[56] = {0};      //�ļ��ϴ���
    long size = 0;
    
	
	char get_filePath[20] = "filepath";
	char get_fileName[20] = "filename";
	
	
		
	while (FCGI_Accept() >= 0) 
	{
		FCGI_printf("Content-type: text/html; charset=UTF-8\r\n\r\n");
			   	 
		char *contentLength = getenv("CONTENT_LENGTH");	 			//��ȡ��������	
		char *querystring = getenv("QUERY_STRING"); 
		
		char filepath[256] = "/home/tbs/TBSNative/TBSUploadFile/file/image";
		
		if(contentLength != NULL)
		{
			len = strtol(contentLength, NULL, 10);	
		}
				
		char name[256] = "method";
		char *method = get_query_string(querystring, name);
		
		//char *filepath = get_query_string(querystring, get_filePath);		
		char *filename = get_query_string(querystring, get_fileName);
			
										
		if(strcasecmp(method, "wxupload") == 0)
		{
			FCGI_printf("filename = %s ", filename);
			//wx_recv_save_file(len, user, filepath, filename, &size);
		}						
						
	}
	return 0;
}