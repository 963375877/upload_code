#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


#define UPLOAD_LOG_MODULE "cgi"
#define UPLOAD_LOG_PROC   "upload"

#define TEMP_BUF_MAX_LEN 512
#define FILE_NAME_LEN 256
#define USER_NAME_LEN 256



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
int recv_save_file(long len, char *user, char *filename, long *p_size);


char* memstr(char* full_data, int full_data_len, char* substr);

/*
 *@brief  ���ַ���д�뵽ָ����log�ļ�
 *         
 *
 * @param log       (in)    Ҫд���ַ���
 * @param len      	(in)   	Ҫд�ַ����ĳ���
 * @param logPath   (in)    log�ļ�����·��
 * @param logName   (in)    log�ļ���
 *
 * @returns
 *          0 succ, -1 fail
 */
int write_log(char * logPath, char *logName, char * log, int len);