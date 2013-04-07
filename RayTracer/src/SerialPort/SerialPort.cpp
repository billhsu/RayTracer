// Ray Tracer for 3D sound rendering
// Shipeng Xu 2013
// billhsu.x@gmail.com
// Shanghai University

#include "SerialPort.h"

#include <boost/thread.hpp> 
#include <boost/exception/all.hpp>
#include <boost/exception/get_error_info.hpp>

SerialPort::SerialPort(void) : io(), serial(io)
{
    
}


SerialPort::~SerialPort(void)
{
    m_thread.interrupt();
    close();
}

bool SerialPort::open(std::string _device)
{
    try
    {
        serial.open(_device);
    }
    catch (boost::exception& e)
    {

    }
    if(serial.is_open())device = _device;
    return serial.is_open();

}

bool SerialPort::set_option(unsigned int baud, unsigned int parity,
    unsigned int csize, unsigned int flow, unsigned int stop)
{
    if(!serial.is_open())   return false;

    boost::asio::serial_port_base::baud_rate opt_baud(baud);
    boost::asio::serial_port_base::parity opt_parity(
        static_cast<boost::asio::serial_port_base::parity::type>(parity));
    boost::asio::serial_port_base::character_size opt_csize(csize);
    boost::asio::serial_port_base::flow_control opt_flow(
        static_cast<boost::asio::serial_port_base::flow_control::type>(flow));
    boost::asio::serial_port_base::stop_bits opt_stop(
        static_cast<boost::asio::serial_port_base::stop_bits::type>(stop));

    serial.set_option(opt_baud);
    serial.set_option(opt_parity);
    serial.set_option(opt_csize);
    serial.set_option(opt_flow);
    serial.set_option(opt_stop);
    recv_start();

    m_thread = boost::thread(boost::bind(&boost::asio::io_service::run, &io));

    return true;
}

void SerialPort::recv_callback(boost::function<void (int,BYTE*)> callback)
{
    m_recv_callback = callback;
}

void SerialPort::err_callback(boost::function<void (int,BYTE*)> callback)
{
    m_err_callback = callback;
}

void SerialPort::recv_start(void) 
{ // Start an asynchronous read and call read_complete when it completes or fails 
    serial.async_read_some(boost::asio::buffer(recv_msg, max_buffer_length), 
        boost::bind(&SerialPort::recv_complete, 
        this, 
        boost::asio::placeholders::error, 
        boost::asio::placeholders::bytes_transferred)); 
}

void SerialPort::recv_complete(const boost::system::error_code& error, size_t bytes_transferred) 
{ // the asynchronous read operation has now completed or failed and returned an error 
    if (!error) 
    { // read completed, so process the data 
        //cout.write(recv_msg, bytes_transferred); // echo to standard output
        
        recv_start(); // start waiting for another asynchronous read again 
    } 
    else 
        do_close(error); 
} 

void SerialPort::do_send(const char msg) 
{
    bool write_in_progress = !send_msg.empty(); // is there anything currently being written? 
    send_msg.push_back(msg); // store in write buffer 
    if (!write_in_progress) // if nothing is currently being written, then start 
        send_start(); 
} 

void SerialPort::send_start(void)
{ // Start an asynchronous write and call write_complete when it completes or fails 
    boost::asio::async_write(serial, 
        boost::asio::buffer(&send_msg.front(), 1), 
        boost::bind(&SerialPort::send_complete, 
        this, 
        boost::asio::placeholders::error)); 
} 

void SerialPort::send_complete(const boost::system::error_code& error) 
{ // the asynchronous read operation has now completed or failed and returned an error 
    if (!error) 
    { // write completed, so send next write data 
        send_msg.pop_front(); // remove the completed data 
        if (!send_msg.empty()) // if there is anything left to be written 
            send_start(); // then start sending the next item in the buffer 
    } 
    else 
        do_close(error); 
} 

void SerialPort::do_close(const boost::system::error_code& error) 
{
    if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel() 
        return; // ignore it because the connection canceled the timer 

    serial.close(); 
}