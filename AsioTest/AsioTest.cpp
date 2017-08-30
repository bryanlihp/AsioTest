// AsioTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "boost\lexical_cast.hpp"
#include "boost\bind.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\thread.hpp"
#include "boost\asio.hpp"
#include "boost\asio\strand.hpp"

int GetWaitTime(int nMin, int nMax)
{
	// range_min <= random number < range_max
	int nWaitTime = (int)((double)rand() / (RAND_MAX + 1) * (nMax - nMin) + nMin);
	return nWaitTime;
}

void Test1()
{
	//test1_1 Hotfix applied
	boost::asio::io_service svc1_1;
	// up to this line
	svc1_1.run();
	std::cout << "Do you reckon this line displays?" << std::endl; // run returns immediately as there is no work to do
}
void Test2()
{
	boost::asio::io_service svc;
	boost::asio::io_service::work work(svc);
	svc.run();														// run will block forever because the work object		
	std::cout << "Do you reckon this line displays?" << std::endl;
}

void Test3()
{
	boost::asio::io_service svc_3; //hotfix test3 applied.
	for (int i = 0; i < 42; i++)
	{
		svc_3.poll();
		std::cout << "Counter" << i << std::endl;
	}
}

void Test4()
{
	//hotfix test4 applied.
	boost::asio::io_service svc;
	boost::asio::io_service::work work(svc);
	for (int i = 0; i < 42; i++)
	{
		svc.poll();
		std::cout << "Counter" << i << std::endl;
	}
}

void Test5()
{
	boost::shared_ptr<boost::asio::io_service> pService(new boost::asio::io_service());
	boost::shared_ptr<boost::asio::io_service::work> pWork(new boost::asio::io_service::work(*pService));
	pWork.reset(); // Destroy the work object
/*
	boost::asio::io_service::work *pWork = new boost::asio::io_service::work(svc);
	delete pWork;
	pWork = NULL;
*/
	pService->run();		// run won't block as the work object is destroyed.
	std::cout << "Do you reckon this line displays?" << std::endl;
}

boost::asio::io_service g_svc_6;
void WorkerThread_6()
{
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	g_svc_6.run();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
}


void Test6()
{
	boost::shared_ptr<boost::asio::io_service::work> pWork(new boost::asio::io_service::work(g_svc_6));
	std::cout << "Press [return] to exit.\n";
	boost::thread_group threads;
	for (int i = 0; i < 4; ++i)
	{
		threads.create_thread(WorkerThread_6); // starts two threads
	}
	std::cin.get();
	std::cout << "Main thread: stopping worker threads." << std::endl;
//	pWork.reset();
	g_svc_6.stop();  // this can also stop terminate the run() loop ( makes sense only when multiple threads)
	std::cout << "Main thread: waiting worker threads to finish." << std::endl;
	threads.join_all();
	std::cout << "Main thread finished." << std::endl;
}

int F2(int n,float &f)
{
	std::cout << "n: " << n << std::endl;
	std::cout << "f: " << f << std::endl;
	f = f*n;
	return n+1;
}

void Test7() 
{
	float f = 0.7f;
	int n = boost::bind<int>(&F2, 7, boost::ref(f))(); // using boost bind, this is equivalent to int n = F2(7,f) 

	std::cout << "Test7 returned: n=" << n <<", f="<< f << std::endl;
}


class MyClass
{
public:
	double F3(int &n, double f)
	{
		std::cout << "n: " << n << std::endl;
		std::cout << "f: " << f << std::endl;
		f = f*n;
		return f;
	}

	void Test8()
	{
		int n = 7;
		double f = 0.7f;
		f = boost::bind<double>(&MyClass::F3, this, boost::ref(n), f)();  // using boost bind, this is equivalent to int n = this->F3(n,f) 
		std::cout << "Test8 in object returned: n=" << n << ", f=" << f << std::endl;
	}
};

void Test8()
{
	int n = 7;
	double f = 0.7f;
	MyClass obj;
	f = boost::bind<double>(&MyClass::F3, &obj, boost::ref(n), f)();  // using boost bind, this is equivalent to int n = obj.F3(n,f) 
	std::cout << "Test8 returned: n=" << n << ", f=" << f << std::endl;
	obj.Test8();
}

typedef boost::asio::io_service asio_service;
typedef boost::asio::io_service::work asio_work;
typedef boost::shared_ptr<asio_service> asio_service_ptr;
typedef boost::shared_ptr<asio_work> asio_work_ptr;

void WorkerThread_9(asio_service_ptr pSvc)
{
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	pSvc->run();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
}

void Test9()
{
	asio_service_ptr pSvc(new boost::asio::io_service());
	asio_work_ptr pWork(new boost::asio::io_service::work(*pSvc));

	std::cout << "Press [return] to exit.\n";

	boost::thread_group threads;
	for (int i = 0; i < 4; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_9, pSvc)); // using bind and threads, note that thread function now takes one parameter instead void, which is required by create_thread
	}
	std::cin.get();
	pSvc->stop();
	threads.join_all();
}

boost::mutex global_stream_lock;
void stream_lock()
{
	global_stream_lock.lock();
}
void stream_unlock()
{
	global_stream_lock.unlock();
}

void WorkerThread_10(asio_service_ptr pSvc)
{
	stream_lock(); // lock cout is required to avoid output overlapping
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	pSvc->run();
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}

void Test10()
{

	asio_service_ptr pSvc(new boost::asio::io_service());
	asio_work_ptr pWork(new boost::asio::io_service::work(*pSvc));
	std::cout << "Press [return] to exit.\n"; // locking cout is not required as there is only one thread at this point
	
	boost::thread_group threads;
	for (int i = 0; i < 4; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_10, pSvc));
	}

	std::cin.get();
	pSvc->stop();
	threads.join_all();
}


void WorkerThread_11(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	pSvc->run();
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}

size_t Fib(size_t n)
{
	// simulating doing some work
	if (n <= 1)
	{
		return n;
	}
	boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

	return Fib(n - 1) + Fib(n - 2);
}

void CalculateFib(size_t n)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " now calculating fib(" << n << ")." << std::endl;
	stream_unlock();

	size_t f = Fib(n);
	boost::this_thread::sleep_for(boost::chrono::seconds(1));

	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " fib(" << n << ")=" << f << std::endl;
	stream_unlock();
}

void Test11()
{

	asio_service_ptr pSvc(new boost::asio::io_service());
	asio_work_ptr pWork(new boost::asio::io_service::work(*pSvc));
	std::cout << "Press [return] to exit.\n";

	int nThreads = boost::thread::hardware_concurrency(); // getting number of CPU cores
	
	// creating thread pool
	boost::thread_group threads;
	for (int i = 0; i < nThreads; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_11, pSvc)); 
	}
	
/*
	for (size_t n = 1; n < 10; n++)
	{
		pSvc->post(boost::bind(&CalculateFib, n)); // post 10 works to threads.
	}
*/
	pSvc->post(boost::bind(&CalculateFib, 4));
	pSvc->post(boost::bind(&CalculateFib, 2));
	pSvc->post(boost::bind(&CalculateFib, 4));
	pSvc->post(boost::bind(&CalculateFib, 8));
	pSvc->post(boost::bind(&CalculateFib, 3));
	pSvc->post(boost::bind(&CalculateFib, 6));
	pSvc->post(boost::bind(&CalculateFib, 1));
	pSvc->post(boost::bind(&CalculateFib, 2));
	pSvc->post(boost::bind(&CalculateFib, 10));
	pSvc->post(boost::bind(&CalculateFib, 7));

	stream_lock();
	std::cout << "Main thread: Destroying work object.\n";
	stream_unlock();
//	pSvc->stop();
//	pWork.reset(); // destroy work object
	stream_lock();
	std::cout << "Main thread: work object destroyed.\n";
	stream_unlock();
	std::cin.get();
	stream_lock();
	std::cout << "Main thread: waiting threads.\n";
	stream_unlock();
	threads.join_all();
	std::cout << "Main thread stopped.\n";
}

void WorkerThread_12(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	pSvc->run();
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}

void Dispatch(int x)
{
	stream_lock();
	std::cout << "[" << boost::this_thread::get_id() << "] " << __FUNCTION__ << " x = " << x << std::endl;
	stream_unlock();
}

void Post(int x)
{
	stream_lock();
	std::cout << "[" << boost::this_thread::get_id() << "] " << __FUNCTION__ << " x = " << x << std::endl;
	stream_unlock();
}

void Run3(asio_service_ptr pSvc)
{
	for (int x = 0; x < 10; ++x)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] dispatching  " << x * 2 << std::endl;
		stream_unlock();
		pSvc->dispatch(boost::bind(&Dispatch, x * 2)); // will be executed immediately by this thread
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] posting  " << x * 2 + 1 << std::endl;
		stream_unlock();
		pSvc->post(boost::bind(&Post, x * 2 + 1));	  // posted to a queue and will be executed by a scheduled thread
//		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
}

void Test12()
{
	asio_service_ptr pSvc(new boost::asio::io_service());
	asio_work_ptr pWork(new boost::asio::io_service::work(*pSvc));
	std::cout << "Press [return] to exit.\n";
	boost::thread_group threads;
	//Creating thread pool
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_12, pSvc));
	}
	// post one work to asio
	pSvc->post(boost::bind(&Run3, pSvc));

	pWork.reset();
	//pSvc->stop()
	std::cin.get();
	stream_lock();
	std::cout << "Main thread: waiting threads.\n";
	stream_unlock();
	threads.join_all();
	std::cout << "Main thread stopped.\n";
}


void WorkerThread_13(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	pSvc->run();
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}

void PrintNum(int x)
{
	int nWait = GetWaitTime(1, 4);
	boost::this_thread::sleep_for(boost::chrono::seconds(nWait));
	std::cout << "[" << boost::this_thread::get_id() << "] x: " << x << std::endl;
}

void Test13()
{
	srand((unsigned)time(NULL));

	asio_service_ptr pSvc(new boost::asio::io_service());
	asio_work_ptr pWork(new boost::asio::io_service::work(*pSvc));
	std::cout << "Main Thread[" << boost::this_thread::get_id()<<"], the program will exit when all work has finished.\n";

	// 8 threads in thread pool
	boost::thread_group threads;
	for (int i = 0; i < 8; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_13, pSvc));
	}
	boost::asio::io_service::strand strand(*pSvc); // create a strand

/*
//  order of output depends on the order of the threads executing the works, there is no guarantee of the order of the result. 
//	even worse, cout is not locked i PrintNum(), the the output is overlapped.
	pSvc->post(boost::bind(&PrintNum, 1));
	pSvc->post(boost::bind(&PrintNum, 2));
	pSvc->post(boost::bind(&PrintNum, 3));
	pSvc->post(boost::bind(&PrintNum, 4));
	pSvc->post(boost::bind(&PrintNum, 5));
	pSvc->post(boost::bind(&PrintNum, 6));
*/

/*
	//	using strand.wrap guarantees that the every work that are passed to the service are executed serially but not the order of the works
	//	The content of a work is in the strand but the work is not.
	// The order of the result still depends on how threads executes the works but cout object ( the content of the work) will be used in order and 
	// the out put is not overlapped.

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 1)));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 2)));

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 3)));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 4)));

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 5)));
	pSvc->post(strand.wrap(boost::bind(&PrintNum, 6)));
*/

	// works are in a strand, and the order is guaranteed
	strand.post(boost::bind(&PrintNum, 1));
	strand.post(boost::bind(&PrintNum, 2));
	strand.post(boost::bind(&PrintNum, 3));
	strand.post(boost::bind(&PrintNum, 4));
	strand.post(boost::bind(&PrintNum, 5));
	strand.post(boost::bind(&PrintNum, 6));

	pWork.reset();

	stream_lock();
	std::cout << "Main thread: waiting threads.\n";
	stream_unlock();

	threads.join_all();

	stream_lock();
	std::cout << "Main thread stopped.\n";
	stream_unlock();
}

void WorkerThread_14(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	try
	{
		pSvc->run();
	}
	catch (std::exception &ex) // handles the exception that was thrown by RaiseError()
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << ex.what() << std::endl;
		stream_unlock();
	}
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}

void WorkerThread_15(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();
	
	boost::system::error_code ec;
	pSvc->run(ec);
	if (0!=ec)
	{
		global_stream_lock.lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << ec << std::endl;
		global_stream_lock.unlock();
	}

	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}


void RaiseError(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread [" << boost::this_thread::get_id() << "] " << __FUNCTION__ << std::endl;
	stream_unlock();
	pSvc->post(boost::bind(&RaiseError, pSvc));  // raise another exception, this will effectively kill all threads in thread pool.
	throw(std::runtime_error("Oops!"));
}

void Test14()
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	stream_lock();
	std::cout << "Main Thread[" << boost::this_thread::get_id() << "], the program will exit when all work has finished.\n";
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 3; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_14, pSvc));
	}

	pSvc->post(boost::bind(&RaiseError, pSvc)); // raise an std run time exception
	// although we do not kill the work, all threads still quits because of exception
	threads.join_all();
}


void Test15()
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	stream_lock();
	std::cout << "Main Thread[" << boost::this_thread::get_id() << "], the program will exit when all work has finished.\n";
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(&WorkerThread_15, pSvc));
	}

	pSvc->post(boost::bind(&RaiseError, pSvc));
	threads.join_all();
}

void WorkerThread_16(asio_service_ptr pSvc)
{
	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " started." << std::endl;
	stream_unlock();

	while(true)
	{
		try
		{
			boost::system::error_code ec;
			pSvc->run(ec);
			if (0 != ec)
			{
				stream_lock();
				std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << std::endl;
				stream_unlock();
			}
			break;  // Thread exits when run exists
		}
		catch (std::exception &ex)
		{
			global_stream_lock.lock();
			std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << ex.what() << std::endl;
			global_stream_lock.unlock();
			// continue work after exception handling
		}
	}

	stream_lock();
	std::cout << "Thread " << boost::this_thread::get_id() << " finished." << std::endl;
	stream_unlock();
}


typedef void(*WorkThreadFun)(asio_service_ptr pSvc);
void Test(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	stream_lock();
	std::cout << "Main Thread[" << boost::this_thread::get_id() << "], the program will exit when all work has finished.\n";
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}

	pSvc->post(boost::bind(&RaiseError, pSvc));
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	pSvc->stop();
	threads.join_all();
}

void TimerHandler(const boost::system::error_code &ec)
{
	if (0!=ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << std::endl;
		stream_unlock();
	}
	else
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] TimerHandler " << std::endl;
		stream_unlock();
	}
}

typedef boost::asio::deadline_timer asio_timer;
typedef boost::shared_ptr<boost::asio::deadline_timer> asio_timer_ptr;

void TimerHandlerEx(const boost::system::error_code &ec, asio_timer_ptr pTimer)
{
	
	if (0 != ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << ", Message=" << ec.message() << std::endl;
		stream_unlock();
	}
	else
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] TimerHandlerEx " << std::endl;
		stream_unlock();
		// reset time to expire 200 ms later
		pTimer->expires_from_now(boost::posix_time::milliseconds(200));
		pTimer->async_wait(boost::bind(&TimerHandlerEx, _1, pTimer));
	}
}

void Test17(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	stream_lock();
	std::cout << "Main thread[" << boost::this_thread::get_id() << "] Press [return] to exit." << std::endl;
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}

	//Create a timer that expires 2 seconds from now
	boost::asio::deadline_timer timer(*pSvc);
	timer.expires_from_now(boost::posix_time::seconds(2));
	// wait asynchronously
	timer.async_wait(TimerHandler);

	std::cin.get();
	pSvc->stop();
	threads.join_all();
}


void Test18(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	stream_lock();
	std::cout << "Main thread[" << boost::this_thread::get_id() << "] Press [return] to exit." << std::endl;
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}

	asio_timer_ptr pTimer(new asio_timer(*pSvc));
	pTimer->expires_from_now(boost::posix_time::seconds(5));
	pTimer->async_wait(boost::bind(TimerHandlerEx, _1, pTimer));

	std::cin.get();

	pSvc->stop();
	threads.join_all();
}

typedef boost::asio::io_service::strand asio_strand;
typedef boost::shared_ptr<asio_strand> asio_strand_ptr;

void TimerHandlerEx1(const boost::system::error_code &ec, asio_timer_ptr pTimer, asio_strand_ptr pStrand)
{

	if (0 != ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << std::endl;
		stream_unlock();
	}
	else
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] TimerHandlerEx " << std::endl;
		stream_unlock();
		pTimer->expires_from_now(boost::posix_time::milliseconds(200));
		pTimer->async_wait(pStrand->wrap(boost::bind(&TimerHandlerEx1, _1, pTimer,pStrand)));
		//pTimer->async_wait(boost::bind(&TimerHandlerEx, _1, pTimer));
	}
}

void TimerHandlerEx_NoStrand(const boost::system::error_code &ec, asio_timer_ptr pTimer)
{

	if (0 != ec)
	{
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << std::endl;
	}
	else
	{
		std::cout << "[" << boost::this_thread::get_id() << "] TimerHandlerEx " << std::endl;
		pTimer->expires_from_now(boost::posix_time::milliseconds(100));
		pTimer->async_wait(boost::bind(&TimerHandlerEx_NoStrand, _1, pTimer));
	}
}



void Test19(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	asio_strand_ptr pStrand(new asio_strand(*pSvc));

	stream_lock();
	std::cout << "Main thread[" << boost::this_thread::get_id() << "] Press [return] to exit." << std::endl;
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 10; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}

	boost::this_thread::sleep_for(boost::chrono::seconds(1));

	pStrand->post(boost::bind(&PrintNum, 1));
	pStrand->post(boost::bind(&PrintNum, 2));
	pStrand->post(boost::bind(&PrintNum, 3));
	pStrand->post(boost::bind(&PrintNum, 4));
	pStrand->post(boost::bind(&PrintNum, 5));
	pStrand->post(boost::bind(&PrintNum, 6));

	asio_timer_ptr pTimer(new asio_timer(*pSvc));
	pTimer->expires_from_now(boost::posix_time::milliseconds(100));
//	pTimer->async_wait(pStrand->wrap(boost::bind(TimerHandlerEx1, _1, pTimer,pStrand)));
	pTimer->async_wait(boost::bind(TimerHandlerEx_NoStrand, _1, pTimer));
//	pTimer->async_wait(boost::bind(&TimerHandlerEx, _1, pTimer));
	std::cin.get();

	pSvc->stop();
	threads.join_all();
}

typedef boost::asio::ip::tcp::socket asio_tcp_socket;
typedef boost::shared_ptr<asio_tcp_socket> asio_tcp_socket_ptr;
typedef boost::asio::ip::tcp::resolver asio_tcp_resolver;
typedef boost::shared_ptr<asio_tcp_resolver> asio_tcp_resolver_ptr;

void OnConnected(const boost::system::error_code &ec, asio_tcp_socket_ptr pSocket)
{
	if (ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << std::endl;
		stream_unlock();
	}
	else
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Connected!" << std::endl;
		stream_unlock();
	}
}

void OnResolved(const boost::system::error_code &ec, asio_tcp_resolver::iterator it, asio_tcp_socket_ptr pSocket)
{
	if (ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec  << ", Message: " << ec.message() << std::endl;
		stream_unlock();
	}
	else
	{
		boost::asio::ip::tcp::endpoint endpoint = *it;
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Address resolved, connecting to" << endpoint << std::endl;
		stream_unlock();
		pSocket->async_connect(endpoint, boost::bind(OnConnected, _1, pSocket));
	}
}
void OnAccept(const boost::system::error_code &ec, asio_tcp_socket_ptr pSocket)
{
	if (ec)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Error: " << ec << ", Message: " << ec.message() << std::endl;
		stream_unlock();
	}
	else
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Accepted!" << std::endl;
		stream_unlock();
	}
}

void NetworkClientTest(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	asio_strand_ptr pStrand(new asio_strand(*pSvc));

	stream_lock();
	std::cout << "Main thread[" << boost::this_thread::get_id() << "] Press [return] to exit." << std::endl;
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}
	boost::this_thread::sleep_for(boost::chrono::seconds(1));
	asio_tcp_socket_ptr pSocket(new asio_tcp_socket(*pSvc));
	asio_tcp_resolver_ptr pResolver(new asio_tcp_resolver(*pSvc));
	try
	{
		std::string s = boost::lexical_cast<std::string>(80);
		asio_tcp_resolver::query query("vsburdev01", "http");
		pResolver->async_resolve(query,boost::bind(OnResolved,_1,_2,pSocket));
	}
	catch (std::exception & ex)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << ex.what() << std::endl;
		stream_unlock();
	}
	std::cin.get();
	pSvc->stop();
	threads.join_all();
}

typedef boost::asio::ip::tcp::acceptor asio_tcp_acceptor;
typedef boost::shared_ptr<asio_tcp_acceptor> asio_tcp_acceptor_ptr;


void NetworkServerTest(WorkThreadFun pfnWorkerThread)
{
	asio_service_ptr pSvc(new asio_service());
	asio_work_ptr pWork(new asio_work(*pSvc));
	asio_strand_ptr pStrand(new asio_strand(*pSvc));

	stream_lock();
	std::cout << "Main thread[" << boost::this_thread::get_id() << "] Press [return] to exit." << std::endl;
	stream_unlock();

	boost::thread_group threads;
	for (int i = 0; i < 2; ++i)
	{
		threads.create_thread(boost::bind(pfnWorkerThread, pSvc));
	}

	asio_tcp_acceptor_ptr pAcceptor(new asio_tcp_acceptor(*pSvc));
	asio_tcp_socket_ptr pSocket(new asio_tcp_socket(*pSvc));

	try
	{
		asio_tcp_resolver_ptr pResolver(new asio_tcp_resolver(*pSvc));
//		std::string s = boost::lexical_cast<std::string>(7777);
		asio_tcp_resolver::query query("localhost", "7777");
		boost::asio::ip::tcp::endpoint ep = *pResolver->resolve(query);
		pAcceptor->open(ep.protocol());
		pAcceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
		pAcceptor->bind(ep);
		pAcceptor->listen(boost::asio::socket_base::max_connections);
		pAcceptor->async_accept(*pSocket, boost::bind(OnAccept, _1, pSocket));
		stream_lock();
		std::cout << "Listening on: " << ep << std::endl;
		stream_unlock();
	}
	catch (std::exception & ex)
	{
		stream_lock();
		std::cout << "[" << boost::this_thread::get_id() << "] Exception: " << ex.what() << std::endl;
		stream_unlock();
	}
	std::cin.get();

	// use err code version not in try ... catch to ensure all both the shutdown and close functions are called.
	boost::system::error_code ec;
	pAcceptor->close(ec);

	pSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	pSocket->close(ec);


	pSvc->stop();
	threads.join_all();
}

class A : std::enable_shared_from_this<A>
{
public:
	virtual std::string GetName()
	{
		return std::string("A");
	}
	std::shared_ptr<A> GetPtr()
	{
		return shared_from_this();
	}
	int a;

};

class B : public A
{
public:
	virtual std::string GetName()
	{
		return std::string("B");
	}
	int b;
};

void Test20()
{
	std::shared_ptr<A> pa(new B);
	std::shared_ptr<A> pb = pa->GetPtr();
	std::string name1 = pa->GetName();
	std::string name2 = pb->GetName();

	std::shared_ptr<B> pa1(new B);
	std::shared_ptr<B> pb1 = std::dynamic_pointer_cast<B>(pa1->GetPtr());

	std::string name3 = pa1->GetName();
	std::string name4 = pb1->GetName();

	int i = 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	stream_lock();
	std::cout << "Main thread ID=" << boost::this_thread::get_id() << std::endl;
	stream_unlock();
	Test1();	//feature1 added
//	Test2();
	Test3();
//	Test4();
//	Test5();
//	Test6();
//	Test7();
//	Test8();
//	Test9();
//	Test10();
//	Test11();
//	Test12();
//	Test13();  // strands
//	Test14();  // exception, no runtime error
//	Test15();  // error code, run time exception
//	Test(&WorkerThread_16); // handle runtime exception and go back to work, no premature thread stopping
//	Test17(&WorkerThread_16);	//Timer
//	Test18(&WorkerThread_16);	// Recurring timer
//	Test19(&WorkerThread_16);	// Timer and strand
//	NetworkClientTest(&WorkerThread_16);
//	NetworkServerTest(&WorkerThread_16);
//	Test20();
	return 0;
}

