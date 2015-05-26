#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

int main(void)
{
   //ByteArray to hold server response
   ByteArray res;
   
   //String to hold user input
   std::string input="";
   
   std::string output="";
   
    try
    {
       //Create and open a socket
       Socket socket("127.0.0.1", 2999);
       socket.Open();
       
       while (input!= "done") {
          
          //Obtain user input
          std::cout<<"Input a string: ";
          getline(std::cin, input);
          
          //convert message to send to server
          ByteArray bytes(input);
          
          //send input to server
          socket.Write(bytes);
          
          //wait for server response
          socket.Read(res);
          
          if (res.ToString()=="") {
             break;
          }
          
          if (res.ToString()=="DONE") {
             socket.Close();
             return 0;
          }
          
          //output server response
          std::cout<<"Server Response: "+res.ToString()<<std::endl;
       }
       std::cout<<"Client is closing"<<std::endl;
       socket.Close();
    }
   
    catch(...) {
        std::cout << "Could not open a connection to the server." << std::endl;
    }
}
