//�õ�ϰ���ǳɹ��Ļ���

#include "UploadFile.h"

// char* getcgidata(FILE* fp, char* requestmethod)   
//{   
//    char* input;   
//    int len;   
//    int size = 1024;   
//    int i = 0;   
//      
//    if (!strcmp(requestmethod, "GET"))   
//    {   
//                 input = getenv("QUERY_STRING");   
//                 return input;   
//    }   
//    else if (!strcmp(requestmethod, "POST"))   
//    {   
//         len = atoi(getenv("CONTENT_LENGTH"));   
//         input = (char*)malloc(sizeof(char)*(size + 1));   
//           
//         if (len == 0)   
//         {   
//            input[0] = '\0';   
//            return input;   
//         }   
//           
//         while(1)   
//         {   
//            input[i] = (char)fgetc(fp);   
//            if (i == size)   
//            {   
//                 input[i+1] = '\0';   
//                 return input;   
//            }   
//              
//            --len;   
//            if (feof(fp) || (!(len)))   
//            {   
//                 i++;   
//                 input[i] = '\0';   
//                 return input;   
//            }   
//            i++;                 
//         }   
//    }   
//    return NULL;  
//
//}
//

//
//int main(int argc, char* argv[])
//{
//	char *input;   
//    char *req_method;   
//    char name[64];   
//    char pass[64];   
//    int i = 0;   
//    int j = 0;   
//      
//	//printf("Content-type: text/plain; charset=iso-8859-1\n\n");   
//    FCIG_printf("Content-type: text/html\n\n");   
//    FCGI_printf("The following is query reuslt:<br><br>");   
//
//    req_method = getenv("REQUEST_METHOD");   
//    input = getcgidata(stdin, req_method);   
//
//    // ���ǻ�ȡ��input�ַ������������µ���ʽ   
//    // Username="admin"&Password="aaaaa"   
//    // ����"Username="��"&Password="���ǹ̶���   
//    // ��"admin"��"aaaaa"���Ǳ仯�ģ�Ҳ������Ҫ��ȡ��   
//      
//    // ǰ��9���ַ���UserName=   
//    // ��"UserName="��"&"֮���������Ҫȡ�������û���   
//    for ( i = 9; i < (int)strlen(input); i++ )   
//    {   
//         if ( input[i] == '&' )   
//         {   
//                name[j] = '\0';   
//                break;   
//         }                                       
//         name[j++] = input[i];   
//    }   
//
//    // ǰ��9���ַ� + "&Password="10���ַ� + Username���ַ���   
//    // �����ǲ�Ҫ�ģ���ʡ�Ե���������   
//    for ( i = 19 + strlen(name), j = 0; i < (int)strlen(input); i++ )   
//    {   
//    	pass[j++] = input[i];   
//    } 
//    
//      
//    pass[j] = '\0';   
//    FCGI_printf("Your Username is %s<br>Your Password is %s<br> \n", name, pass);   
//      
//    return 0;   
//	
	
///
///int len = atoi(getenv("CONTENT_LENGTH"));

///char InputBuffer[4096] = {0};
///int i = 0; 
///int x;

///if(len < 0 || len >= MAX_CONTENT_LENGTH)
///	return;




///while( i < len )   
///{	 /*��stdin�еõ�Form����*/    
///
///	x = FCGI_fgetc(stdin);  
///	if( x == EOF )	 
///		break; 	
///	InputBuffer[i++] = x;	 
	///	return 0;
//}	 

//InputBuffer[i] = '/0';	 
//
//len = i; 
//
//
//m_content = InputBuffer;
//	
//
//	return 0;
//}



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


int write_log(char * logPath, char *logName, char * log, int len)
{
		//char logPath[] = "/home/tbs/TBSNative/TBSUploadFile/src/log";
		FCGI_FILE *fp = NULL;
		if(access(logPath, W_OK) != -1)
		{
			FCGI_fopen(logPath, "ab+");
			FCGI_fwrite(log, 1, len, fp);
			FCGI_fclose(fp);
		}
		else
		{			
			fp = FCGI_fopen(logPath, "wb+");
			FCGI_fwrite(log, 1, len, fp);
			FCGI_fclose(fp);				
		}
		return 0;
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
 
int recv_save_file(long len, char *user, char *filename, long *p_size)
{
	
	char log[526] = {0};
	memset(log,0x00, 526);
    int ret = 0;
    char *file_buf = NULL;
    char *begin = NULL;
    char *p, *q, *k;

    char content_text[TEMP_BUF_MAX_LEN] = {0}; //�ļ�ͷ����Ϣ
    char boundary[TEMP_BUF_MAX_LEN] = {0};     //�ֽ�����Ϣ

    //==========> ���ٴ���ļ��� �ڴ� <===========
    file_buf = (char *)malloc(len);
    
    if (file_buf == NULL)
    {
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "malloc error! file size is to big!!!!\n");
        return -1;
    }

    int ret2 = fread(file_buf, 1, len, stdin); //�ӱ�׼����(web������)��ȡ����
    if(ret2 == 0)
    {    	
        //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "fread(file_buf, 1, len, stdin) err\n");
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
    strncpy(content_text, begin, p-begin);
    content_text[p-begin] = '\0';
    //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"content_text: [%s]\n", content_text);

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

    //begin---> file_len = (p-begin)

    //=====> ��ʱbegin-->p����ָ����������post���ļ�����������
    //======>������д���ļ���,�����ļ���Ҳ�Ǵ�post���ݽ�������  <===========

    int fd = 0;
    fd = open(filename, O_CREAT|O_WRONLY, 0644);
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
    return ret;
}


int main()
{
    char filename[FILE_NAME_LEN] = {0}; //�ļ���
    char user[USER_NAME_LEN] = {0};   //�ļ��ϴ���
    long size;  //�ļ���С

    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        long len;
        int ret = 0;		
 		
        FCGI_printf("Content-type: text/html\r\n\r\n");
   		
        if (contentLength != NULL)
        {
            len = strtol(contentLength, NULL, 10); //�ַ���תlong�� ����atol
        }
        else
        {
            len = 0;
        }

        if (len <= 0)
        {
            FCGI_printf("No data from standard input\n");
            //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "len = 0, No data from standard input\n");
            ret = -1;
        }
        else
        {
            //===============> �õ��ϴ��ļ�  <============
            if (recv_save_file(len, user, filename, &size) < 0)
            {
                ret = -1;
                goto END;
            }
			//LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s�ɹ��ϴ�[%s, ��С��%ld, md5�룺%s]������\n", user, filename, size, md5);
       
END:
            memset(filename, 0, FILE_NAME_LEN);
            memset(user, 0, USER_NAME_LEN);

            char *out = NULL;
            //��ǰ�˷��أ��ϴ����
            /*
               �ϴ��ļ���
               �ɹ���{"code":"008"}
               ʧ�ܣ�{"code":"009"}
            */
            if(ret == 0) //�ɹ��ϴ�
            {
                //out = return_status("008");//common.h
            }
            else//�ϴ�ʧ��
            {
               // out = return_status("009");//common.h
            }

            if(out != NULL)
            {
                FCGI_printf(out); //��ǰ�˷�����Ϣ
                free(out);   //�ǵ��ͷ�
            }
        }
    } /* while */

    return 0;
}


