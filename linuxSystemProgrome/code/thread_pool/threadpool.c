/*往任务队列插入itemt头插法*/
/*为什莫用宏定义不用函数，因为虽然都是对prev next操作但是结构体类型不一样（讲究）*/

#define LL_ADD(item,list) do{ 				 				\
	item->prev = NULL;		  								\
	item->next = list;      				 				\
	if(list != NULL) list->prev = item;						\
	list = item;/*item成为list的新的头地址*/ 						\
}while(0)

/*删除*/
#define LL_REMOVE(item,list) do{ 							\
	if(item->prev != NULL)  item->prev->next = item->next;  \
	if(item->next != NULL)  item->next->prev = item->prev;  \
	if(list == item) list = item->next;/*删除的item是头节点情况*/ \
	item->prev = item->next = NULL;							\
}while(0)

/*任务队列
每一个NJOB就相当于任务队列中的每个任务
*/
struct NJOB{
	void(*func)(void *arg);/*回调函数，每个任务都有自己的任务函数*/
	void *user_data;/*每个任务执行的参数*/
	struct NJOB* prev;
	struct NJOB* next;
	/*这四个属性就构成了一个链表的结点，也就构成了任务队列*/
}NJOB,*P_NJOB;
	
/*执行队列，相当于线程池中的一个个线程，每个线程都去操作任务队列的相当于操作临界资源*/
typedef struct NWORKER {
	pthread_t id;/*线程id pthread_t数据类型表示线程id*/
	int terminate;//状态牌决定这个线程是否工作
	struct NMANAGER* pool;//操作的哪个pool
	struct NWORKER *prev;
	struct NWORKER *next;
}NWORKER,*p_NWORKER;

/*管理线程与任务*/
typedef struct NMANAGER {
	/*对临界资源的处理 加锁*/ 
	pthread_mutex_t mtx;//pthread_mutex_t 锁的数据类型
	/*当任务队列为空的条件下，线程就会休息（挂起）。
	为什莫要用到条件变量是因为假如线程得到锁后区操作队列发现任务队列为空则会阻塞等待，
	则任务队列就不会被填充为有效，造成死锁。所以要用条件变量发现不满足执行的条件之后
	释放锁，等待在条件变量上
	条件变量的用法：当条件变量满足（任务队列不为空）线程就去操作队列*/
	pthread_cond_t cond;//pthread_cond_t 条件变量数据类型 就例如饭堂模型如果没有人打饭大妈在等待条件满足
	struct NJOB  * jobs;
	struct NWORKER *workes;
}nThreadPool,*P_nThreadPool;

//封装为sdk
void *thread_cb(void *arg)//线程的回调 传woker参数进来(也就是打饭模型的阿姨做的事情)
{
	struct NWORKER* worker = (struct NWORKER*)arg;
	while(1)//引入terminate推出while1 
	{	
		pthread_mutex_lock(&worker->pool->mtx);//条件等待前加锁
		while(worker->pool->jobs == NULL)//任务队列为空 也就是条件为假
		{
			if(worker->terminate == 1) break;//等于1退出
			
			pthread_cond_wait(&worker->pool->cond,&worker->pool->mtx);// 条件等待令进程等待在条件变量上
		}
		if(worker->terminate == 1)//退出外圈while
		{
			//未锁定状态的线程锁进行销毁是安全的。尽量避免对一个处于锁定状态的线程锁进行销毁操
			pthread_mutex_unlock(&worker->pool->mtx);//解锁
			break;
		}
		/*相当于从银行等待的客户中叫来一个人，然后办理他的具体业务*/
		//1 从等待的队列中拿出数据
		struct NJOB *job = worker->pool->jobs;//拿一个任务出来
		if(job != NULL)
		{
			LL_REMOVE(job, worker->pool->jobs);//这样把自己移除了（相当于移除头节点）
		}
		pthread_mutex_unlock(&worker->pool->mtx);//解锁
		job->func(job);//2 办理具体业务
	}
	free(worker);
	pthread_exit(NULL);//解锁
	//pthread_cancel(id);//可以调用外部的函数通过id回收资源
}
/*创建线程池*/
//在os写函数最好有返回值，不然没法判它的状态
int nThreadPoolCreate(nThreadPool *pool,int numberworks)//线程个数numberworks
{
	int i  =0;
	if(numberworks < 1) numberworks =1;//至少创建一个
	if(pool == NULL) return -1;//失败
	memset(pool,0,sizeof(nThreadPool));
	/*对结构体或者一块数据初始化赋值可以用memcpy*/
	//初始化线程锁pthread_mutex_init(&mtx, NULL);同样也可以采取下面的方法
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;//pthread_mutex_t 应该是锁的数据类型
	memcpy(pool->mtx,&blank_mutex,sizeof(pthred_mutex_t));
	
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;//pthread_cond_t 应该是条件变量的数据类型
	memcpy(pool->cond,&blank_cond,sizeof(pthread_cond_t));//memcpy 将内存的值直接拷贝但是strcpy就是当数组不为0不断的拷贝
	//works
	for(i= 0; i < numberworks;i++)
	{	
		struct NWORKER* worker = (struct NWORKER*)malloc(sizeof(struct NWORKER));
		if(NULL == worker)
		{
			perror("malloc");
			return -2;
		}
		memset(worker,0,sizeof(struct NWORKER));//malloc出来是脏的区域一定要清0
		worker->pool = pool;
		int ret = pthread_create(&worker->id,NULL,thread_cb,worker);//创建线程 id 属性 回调函数 参数(传入回调函数)
		if(ret)//0 成功
		{
			perror("pthread_create");//失败
			free(worker);
			return -3;
		}
		LL_ADD(worker, pool->workes);//构成链表节点加入工作队列
	}
}
/*销毁线程池*/
int nThreadPoolDestory(nThreadPool *pool)//销毁线程池
{
	struct	NWORKER *worker = NULL;
	for(worker = pool->workes;worker != NULL;worker = worker->next)
	{
		worker->terminate = 1;//不需要加锁
	}
	pthread_mutex_lock(&pool->mtx);
	pthread_cond_broadcast(&pool->cond);//广播
	pthread_mutex_unlock(&pool->mtx);
}
/*往任务队列仍一个任务（也就是往线程池仍任务）要发送一个信号通知线程，因为线程都要满足条件等待*/
int nThreadPoolPushTask(nThreadPool * pool,struct NJOB*job)
{/*多线程的增加删除队列都要加锁*/
	pthread_mutex_lock(&pool->mtx);//加锁这个操作是阻塞调用的，也就是说，如果这个锁此时正在被其它线程占用， 那么 pthread_mutex_lock() 调用会进入到这个锁的排队队列中
	LL_ADD(job, pool->jobs);
	pthread_cond_signal(pool->cond);// 通知等待在条件变量上的线程
	pthread_mutex_unlock(&pool->mtx);//加锁
}

// 0 --1000基数
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

