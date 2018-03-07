
/*
	���ܣ���ȡ�Ѿ��ϴ�������ļ��б�,���ļ�����MD5ֵ��
	������
		filePath 	(in)	�ļ�·��
		folderPath  (in)	�ļ���·��
	����ֵ���ɹ�0 ʧ�ܷ���-1
*/
int getChunkList(char *filePath, char *folderPath, FCGX_Request request);


/*
	���ܣ��ж��ļ����Ƿ���ڣ������ڴ���һ���ļ���	
	������
		filepath   (in)		�ļ���·��
		filename   (in)		�ļ�������	
	����ֵ���ɹ�����0 ʧ�ܷ���-1
*/
int folderIsExit(char *filepath, char *filename, FCGX_Request request);


/*
	���ܣ���ָ�����ȵ��ַ����в����ַ���
	������
		full_data   	(in)	 �����ַ���
		full_data_len   (in)     ���ҳ��ȳ���
		substr          (in)     Ҫ���ҵ��ַ���
	����ֵ���ɹ�����Ҫ���ҵ��ַ����ĳ�ʼλ�� ʧ�ܷ���NULL	
*/
char* memstr(char* full_data, int full_data_len, char* substr);


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
int recv_save_file(long len, char *user, char *filename, long *p_size, FCGX_Request request, char *filepath);


/*
	���ܣ���ȡ�ļ���С
	������
		path  (in)	�ļ�ȫ·��
	����ֵ���ɹ������ļ���С ʧ�ܷ���-1
*/
int getfilesize(char *path);

/*
	���ܣ�������ļ��ϲ�Ϊһ���ļ�, ͬʱɾ��Ƭ���ļ�
	������
		part_f_path  (in)	Ƭ���ļ�·��
		filepath     (in)	�ϲ����ļ�·��
		filename	 (in)	�ϲ����ļ���
	����ֵ�� �ɹ����� 0  ʧ�ܷ���-1

*/
int  merge(char *part_f_path , int part_num, char *filepath, char *filename, FCGX_Request request);


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
int write_log(char *logName, char * log, int len);


/**
 * ΢���ϴ�logo�ӿ�
 * @brief  �����ϴ���post���� ���浽������ʱ·��
 *         ͬʱ�õ��ļ��ϴ��ߡ��ļ����ơ��ļ���С
 *
 * @param len       	(in)    post���ݵĳ���
 * @param user      	(out)   �ļ��ϴ���
 * @param filepath  	(out)   �ļ����·��
 * @param file_name 	(out)   �ļ����ļ���
 * @param p_size    	(out)   �ļ���С
 *
 * @returns
 *          0 succ, -1 fail
 */
 
int wx_recv_save_file(long len, char *user, char * filepath, char *filename, long *p_size, FCGX_Request request);

/*
	���ܣ������ݲ������ݿ���
	������
		host      (in)	�˿�
		user	  (in)	�û���
		psword	  (in)	����
		dbname	  (in)	���ݿ���
		sql		  (in)	sql���
	����ֵ���ɹ����� 0 ʧ�ܷ���-1
*/
int mysql(char * host, char *user, char *psword, char *dbname, char *sql,int len, FCGX_Request request);