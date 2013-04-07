// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <boost/bind.hpp> 
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/thread.hpp> 
#include <deque>
#include <boost/lexical_cast.hpp> 
#include <boost/date_time/posix_time/posix_time_types.hpp>


class SerialPort
{
public:
    SerialPort();
    ~SerialPort(void);

    // Open serial device. e.g. "COM1" in Windows or "/dev/ttyS0" in Linux
    bool open(std::string _device);

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn Serial::set_option()
    ///
    /// @brief  Set serial port opinions
    ///         parity:     0->none, 1->odd, 2->even
    ///         csize:      5 6 7 8
    ///         flow:       0->none, 1->software, 2->hardware
    ///         stop:       0->one,  1->onepointfive, 2->two
    ///////////////////////////////////////////////////////////////////////////////
    bool set_option(unsigned int baud, unsigned int parity = 0,
        unsigned int csize = 8, unsigned int flow = 0, unsigned int stop = 0);

    // Send a byte to serial port
    void send(const char msg)
    {
        io.post(boost::bind(&SerialPort::do_send, this, msg)); 
    }
    bool sendtest()
    {
        io.post(boost::bind(&SerialPort::do_send, this, 'a'));
        return true;
    }
    bool is_open()
    {
        return serial.is_open();
    }

    void close()
    {
        io.post(boost::bind(&SerialPort::do_close, this, boost::system::error_code()));
        m_thread.join();
        io.reset();
    }

    void recv_callback(boost::function<void (int,BYTE*)>);
    void err_callback(boost::function<void (int,BYTE*)>);



private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;

    std::string device;
    static const int max_buffer_length = 512;

    char recv_msg[max_buffer_length];
    std::deque<char> send_msg;

    boost::function<void (int,BYTE*)> m_recv_callback;
    boost::function<void (int,BYTE*)> m_err_callback;

    boost::thread m_thread;

    void recv_start(void);
    void recv_complete(const boost::system::error_code& error, size_t bytes_transferred);

    void do_send(const char msg);
    void send_start(void);
    void send_complete(const boost::system::error_code& error);

    void do_close(const boost::system::error_code& error);
};


#endif