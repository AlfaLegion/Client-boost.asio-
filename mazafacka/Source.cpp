#include<boost\asio.hpp>
#include<boost\system\error_code.hpp>
#include<boost\shared_ptr.hpp>
#include<boost\thread.hpp>
#include<mutex>
#include<locale>
#include<string>
#include<iostream>
#include<thread>
#include<condition_variable>
#include<fstream>
using namespace boost::asio;
using Socket = ip::tcp::socket;
using socket_ptr = boost::shared_ptr<Socket>;
//using socket_ptr = boost::shared_ptr<ip::tcp::socket>;
//void read_data_from_server(ip::tcp::socket& sock)
//{
//	while (true)
//	{
//		char data[512];
//		auto len = sock.read_some(buffer(data));
//		std::string str(data, len);
//		if (len > 0)
//			std::cout << data << std::endl;
//	}
//}
//int main()
//{
//	io_service service;
//	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 2002);
//	ip::tcp::socket sock(service);
//	try
//	{
//		sock.connect(ep);
//	}
//	catch ( boost::system::system_error e)
//	{
//		setlocale(LC_ALL, "Rus");
//		std::cout <<e.what()  << std::endl; 
//	}
//	std::thread th(read_data_from_server, std::ref(sock));
//	th.detach();
//	while (true)
//	{	
//		
//		std::string str;
//		std::cin >> str;
//		sock.write_some(buffer(str));
//	}
//	system("pause");
//
//}

class client
{
private:
	ip::tcp::socket socket;
	ip::tcp::endpoint ep;
	std::string adress;
	size_t nPort;

	std::string name;

	std::mutex mut;
public:
	client()=delete;
	client(io_service& io,const std::string& str) :socket(io),name(str){}
	size_t availabel()const  noexcept
	{
		return socket.available();
	}
	bool is_open()const noexcept
	{
		return socket.is_open();
	}
	void connect(const std::string& adr, unsigned short nP)
	{
		ep.address(ip::address::from_string(adr));
		ep.port(nP);
		socket.connect(ep);
		socket.write_some(buffer(name));
	}
	void write(const std::string& mes)
	{
		std::lock_guard<std::mutex>lg(mut);
		socket.write_some(buffer(mes));
	}
	std::string read(size_t sBuff)
	{
		std::unique_ptr<char>buff(new char[sBuff]);
		std::unique_lock<std::mutex>ul(mut, std::try_to_lock);
		size_t len=socket.read_some(buffer(buff.get(), sBuff));
		ul.unlock();
		return std::string(buff.get(), len);
	}
	void read_handler()// не закончана
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(50));
			if (socket.available())
			{
				std::cout << read(availabel()) << std::endl;
			}
		}
	}
	~client()
	{
		std::fstream fl("log.txt", std::ios_base::out);
		fl << "exit" << std::endl;
		write("#exit");
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		
	}
};



int main()
{
	
	setlocale(LC_ALL, "Rus");
	io_service service;
	std::string name;
	std::cin >> name;
	client cl(service,name);
	try
	{
		cl.connect("127.0.0.1", 2002);
	}
	catch (const boost::system::system_error& er)
	{
		std::cerr << er.what() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
		
	}
	std::thread th(&client::read_handler,&cl);
	th.detach();
	while (true)
	{
		if(!cl.is_open())
		{
			std::cout << "connection fail" << std::endl;
			break;
		}
		std::string str;
		std::getline(std::cin, str);
		try
		{
			cl.write(str);
		}
		catch (const boost::system::system_error& er)
		{
			std::cerr << er.code() << std::endl;
			system("pause");
			exit(EXIT_FAILURE);
		}
	}
	system("pause");
	return 0;
}