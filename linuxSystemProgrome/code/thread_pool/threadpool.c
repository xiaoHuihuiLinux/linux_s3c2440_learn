/*��������в���itemtͷ�巨*/
/*ΪʲĪ�ú궨�岻�ú�������Ϊ��Ȼ���Ƕ�prev next�������ǽṹ�����Ͳ�һ����������*/

#define LL_ADD(item,list) do{ 				 				\
	item->prev = NULL;		  								\
	item->next = list;      				 				\
	if(list != NULL) list->prev = item;						\
	list = item;/*item��Ϊlist���µ�ͷ��ַ*/ 						\
}while(0)

/*ɾ��*/
#define LL_REMOVE(item,list) do{ 							\
	if(item->prev != NULL)  item->prev->next = item->next;  \
	if(item->next != NULL)  item->next->prev = item->prev;  \
	if(list == item) list = item->next;/*ɾ����item��ͷ�ڵ����*/ \
	item->prev = item->next = NULL;							\
}while(0)

/*�������
ÿһ��NJOB���൱����������е�ÿ������
*/
struct NJOB{
	void(*func)(void *arg);/*�ص�������ÿ���������Լ���������*/
	void *user_data;/*ÿ������ִ�еĲ���*/
	struct NJOB* prev;
	struct NJOB* next;
	/*���ĸ����Ծ͹�����һ������Ľ�㣬Ҳ�͹������������*/
}NJOB,*P_NJOB;
	
/*ִ�ж��У��൱���̳߳��е�һ�����̣߳�ÿ���̶߳�ȥ����������е��൱�ڲ����ٽ���Դ*/
typedef struct NWORKER {
	pthread_t id;/*�߳�id pthread_t�������ͱ�ʾ�߳�id*/
	int terminate;//״̬�ƾ�������߳��Ƿ���
	struct NMANAGER* pool;//�������ĸ�pool
	struct NWORKER *prev;
	struct NWORKER *next;
}NWORKER,*p_NWORKER;

/*�����߳�������*/
typedef struct NMANAGER {
	/*���ٽ���Դ�Ĵ��� ����*/ 
	pthread_mutex_t mtx;//pthread_mutex_t ������������
	/*���������Ϊ�յ������£��߳̾ͻ���Ϣ�����𣩡�
	ΪʲĪҪ�õ�������������Ϊ�����̵߳õ��������������з����������Ϊ����������ȴ���
	��������оͲ��ᱻ���Ϊ��Ч���������������Ҫ�������������ֲ�����ִ�е�����֮��
	�ͷ������ȴ�������������
	�����������÷����������������㣨������в�Ϊ�գ��߳̾�ȥ��������*/
	pthread_cond_t cond;//pthread_cond_t ���������������� �����緹��ģ�����û���˴򷹴����ڵȴ���������
	struct NJOB  * jobs;
	struct NWORKER *workes;
}nThreadPool,*P_nThreadPool;

//��װΪsdk
void *thread_cb(void *arg)//�̵߳Ļص� ��woker��������(Ҳ���Ǵ�ģ�͵İ�����������)
{
	struct NWORKER* worker = (struct NWORKER*)arg;
	while(1)//����terminate�Ƴ�while1 
	{	
		pthread_mutex_lock(&worker->pool->mtx);//�����ȴ�ǰ����
		while(worker->pool->jobs == NULL)//�������Ϊ�� Ҳ��������Ϊ��
		{
			if(worker->terminate == 1) break;//����1�˳�
			
			pthread_cond_wait(&worker->pool->cond,&worker->pool->mtx);// �����ȴ�����̵ȴ�������������
		}
		if(worker->terminate == 1)//�˳���Ȧwhile
		{
			//δ����״̬���߳������������ǰ�ȫ�ġ����������һ����������״̬���߳����������ٲ�
			pthread_mutex_unlock(&worker->pool->mtx);//����
			break;
		}
		/*�൱�ڴ����еȴ��Ŀͻ��н���һ���ˣ�Ȼ��������ľ���ҵ��*/
		//1 �ӵȴ��Ķ������ó�����
		struct NJOB *job = worker->pool->jobs;//��һ���������
		if(job != NULL)
		{
			LL_REMOVE(job, worker->pool->jobs);//�������Լ��Ƴ��ˣ��൱���Ƴ�ͷ�ڵ㣩
		}
		pthread_mutex_unlock(&worker->pool->mtx);//����
		job->func(job);//2 �������ҵ��
	}
	free(worker);
	pthread_exit(NULL);//����
	//pthread_cancel(id);//���Ե����ⲿ�ĺ���ͨ��id������Դ
}
/*�����̳߳�*/
//��osд��������з���ֵ����Ȼû��������״̬
int nThreadPoolCreate(nThreadPool *pool,int numberworks)//�̸߳���numberworks
{
	int i  =0;
	if(numberworks < 1) numberworks =1;//���ٴ���һ��
	if(pool == NULL) return -1;//ʧ��
	memset(pool,0,sizeof(nThreadPool));
	/*�Խṹ�����һ�����ݳ�ʼ����ֵ������memcpy*/
	//��ʼ���߳���pthread_mutex_init(&mtx, NULL);ͬ��Ҳ���Բ�ȡ����ķ���
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;//pthread_mutex_t Ӧ����������������
	memcpy(pool->mtx,&blank_mutex,sizeof(pthred_mutex_t));
	
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;//pthread_cond_t Ӧ����������������������
	memcpy(pool->cond,&blank_cond,sizeof(pthread_cond_t));//memcpy ���ڴ��ֱֵ�ӿ�������strcpy���ǵ����鲻Ϊ0���ϵĿ���
	//works
	for(i= 0; i < numberworks;i++)
	{	
		struct NWORKER* worker = (struct NWORKER*)malloc(sizeof(struct NWORKER));
		if(NULL == worker)
		{
			perror("malloc");
			return -2;
		}
		memset(worker,0,sizeof(struct NWORKER));//malloc�������������һ��Ҫ��0
		worker->pool = pool;
		int ret = pthread_create(&worker->id,NULL,thread_cb,worker);//�����߳� id ���� �ص����� ����(����ص�����)
		if(ret)//0 �ɹ�
		{
			perror("pthread_create");//ʧ��
			free(worker);
			return -3;
		}
		LL_ADD(worker, pool->workes);//��������ڵ���빤������
	}
}
/*�����̳߳�*/
int nThreadPoolDestory(nThreadPool *pool)//�����̳߳�
{
	struct	NWORKER *worker = NULL;
	for(worker = pool->workes;worker != NULL;worker = worker->next)
	{
		worker->terminate = 1;//����Ҫ����
	}
	pthread_mutex_lock(&pool->mtx);
	pthread_cond_broadcast(&pool->cond);//�㲥
	pthread_mutex_unlock(&pool->mtx);
}
/*�����������һ������Ҳ�������̳߳�������Ҫ����һ���ź�֪ͨ�̣߳���Ϊ�̶߳�Ҫ���������ȴ�*/
int nThreadPoolPushTask(nThreadPool * pool,struct NJOB*job)
{/*���̵߳�����ɾ�����ж�Ҫ����*/
	pthread_mutex_lock(&pool->mtx);//��������������������õģ�Ҳ����˵������������ʱ���ڱ������߳�ռ�ã� ��ô pthread_mutex_lock() ���û���뵽��������ŶӶ�����
	LL_ADD(job, pool->jobs);
	pthread_cond_signal(pool->cond);// ֪ͨ�ȴ������������ϵ��߳�
	pthread_mutex_unlock(&pool->mtx);//����
}

// 0 --1000����
#if 1
int main()
{
	int i;
	nThreadPool *pool;
	NJOB *job;
	for(i =0;i< 1000;i++)
	{
		
	}
	nThreadPoolCreate(pool,6);
	nThreadPoolPushTask(pool,job);
}
#endif

