#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <pthread.h>
#include <vector>

class Connection:public Thread{
public:
   //Game Socket
   Socket socket;
   Socket otherSocket;
   
   //vector list of cards
   std::vector<std::string> cards;
   
   Connection(Socket const &socket, Socket const &otherSocket): socket(socket), otherSocket(otherSocket) {}
   
   void shuffleCards() {
      //Create deck of cards
      std::string suits[4] = {"♥","♦","♠","♣"};
      //clear existing cards
      cards.clear();
      //Populate the cards
      for (int i(1); i<=13; i++) {
         for (int j(0); j<4; j++) {
            std::string number = std::to_string(i);
            number.append(suits[j]);
            cards.push_back(number);
         }
      }
      
      //Shuffle cards using built-in random generator:
      std::random_shuffle(cards.begin(), cards.end());
   }
 
   long ThreadMain(void) {
      
      //Create String messages
      std::string yourTurn = "yourTurn,It is your turn!,\n";
      ByteArray yourTurnMsg(yourTurn);
      std::string roundOver = "roundOver,Round over,\n";
      ByteArray roundOverMsg(roundOver);
      
      //Used for dealer decision and deck shuffling
      std::srand(std::time(0));
      
      //Create cards for all players
      std::string dealerCard1, dealerCard2, clientCard1, clientCard2, client2Card1, client2Card2;
      
      shuffleCards();
      
      //Set player and dealer cards from shuffled deck
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      dealerCard1 = cards.back(); //take card off top
      cards.pop_back();           //Remove card from deck
      
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      dealerCard2 = cards.back();
      cards.pop_back();
      
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      clientCard1 = cards.back();
      cards.pop_back();
      
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      clientCard2 = cards.back();
      cards.pop_back();
      
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      client2Card1 = cards.back();
      cards.pop_back();
      
      //check deck for cards
      if (cards.size() < 2) {
         shuffleCards();
      }
      client2Card2 = cards.back();
      cards.pop_back();
      
      //Create string to send to the client telling them the dealer and client cards
      std::string gameStartMessage1 = "";
      gameStartMessage1.append("Cards");
      gameStartMessage1.append(",");
      gameStartMessage1.append(dealerCard2);
      gameStartMessage1.append(",");
      gameStartMessage1.append(clientCard1);
      gameStartMessage1.append(",");
      gameStartMessage1.append(clientCard2);
      gameStartMessage1.append(",\n");
      
      //Send starting cards to the second player
      std::string gameStartMessage2 = "";
      gameStartMessage2.append("Cards");
      gameStartMessage2.append(",");
      gameStartMessage2.append(dealerCard2);
      gameStartMessage2.append(",");
      gameStartMessage2.append(client2Card1);
      gameStartMessage2.append(",");
      gameStartMessage2.append(client2Card2);
      gameStartMessage2.append(",\n");
      
      //Send starting cards
      ByteArray startMsg(gameStartMessage1);
      socket.Write(startMsg);
      
      //Send starting cards to second player
      ByteArray startMsg2(gameStartMessage2);
      otherSocket.Write(startMsg2);
      
      //"Think" about who goes first for a second
      Sleep(1000);
      
      //Tell client it is its turn
      socket.Write(yourTurnMsg);
      
      //ByteArray to hold received message from client
      ByteArray recievedMsg;
      
      while(1) {
         //Wait for a message from client
         try {
            socket.Read(recievedMsg);
         }
         catch(...) {
            //Cannot read from socket, client could be down, break
            break;
         }
         //Convert message from client to string
         std::string req = recievedMsg.ToString();
         if (req == "done") {
            std::string serverFailed = "failed, The other player quit.,\n";
            ByteArray serverFailedMsg(serverFailed);
            try {
               otherSocket.Write(serverFailedMsg);
               otherSocket.Close();
            }
            catch (...) {
               std::cout<<"Client two has already exited."<<std::endl;
            }
            return 0;
         }
        if(req == "hit") {
            std::string hitCard = "hit,";
           
           //check deck for cards
           if (cards.size() < 2) {
              shuffleCards();
           }
            hitCard.append(cards.back());
            cards.pop_back();
            hitCard.append(",\n");
            ByteArray hitMsg(hitCard);
            socket.Write(hitMsg);
         }
         else if (req=="pass") {
            socket.Write(roundOverMsg);
         }
         else {
            std::istringstream ss(req);
            std::string token;
            std::getline(ss, token, ',');
            
            if (token=="pass") {
               //Allow other player to play here//
               //Tell client it is its turn
               otherSocket.Write(yourTurnMsg);
               
               while (1) {
                  try {
                     otherSocket.Read(recievedMsg);
                  }
                  catch(...) {
                     //Cannot read from socket, client could be down, break
                     break;
                  }
                  std::string req2 = recievedMsg.ToString();
                  std::string serverFailed = "failed, Other player has quit,\n";
                  ByteArray serverFailedMsg(serverFailed);
                  if (req2=="done") {
                     std::string serverFailed = "failed, The other player quit.,\n";
                     ByteArray serverFailedMsg(serverFailed);
                     try {
                        socket.Write(serverFailedMsg);
                        socket.Close();
                     }
                     catch (...) {
                        std::cout<<"Client two has already exited."<<std::endl;
                     }
                     return 0;
                  }
                  else if(req2 == "hit") {
                     std::string hitCard = "hit,";
                     
                     //check deck for cards
                     if (cards.size() < 2) {
                        shuffleCards();
                     }
                     hitCard.append(cards.back());
                     cards.pop_back();
                     hitCard.append(",\n");
                     ByteArray hitMsg(hitCard);
                     otherSocket.Write(hitMsg);
                  }
                  else {
                     std::istringstream ss(req2);
                     std::string token;
                     std::getline(ss, token, ',');
                     if (token=="pass") {
                        //Get dealer card values
                        char firstCharDeal1 = dealerCard1.front();
                        int dealerCard1Value = (int)firstCharDeal1-'0';
                        char firstCharDeal2 = dealerCard2.front();
                        int dealerCard2Value = (int)firstCharDeal2-'0';
                        int dealerCardTotal = dealerCard1Value + dealerCard2Value;
                        
                        //Decide whether dealer should hit or stick
                        while (1) {
                           //hit dealer
                           if ( rand() % 2 ) {
                              
                              //check deck for cards
                              if (cards.size() < 2) {
                                 shuffleCards();
                              }
                              std::string dealerHitCard = cards.back();
                              cards.pop_back();
                              char cardChar = dealerHitCard.front();
                              int hitCardVal = (int)cardChar-'0';
                              dealerCardTotal += hitCardVal;
                              if (dealerCardTotal > 21) {
                                 break;
                              }
                           }
                           else if (dealerCardTotal < 8){
                              
                              //check deck for cards
                              if (cards.size() < 2) {
                                 shuffleCards();
                              }
                              std::string dealerHitCard = cards.back();
                              cards.pop_back();
                              char cardChar = dealerHitCard.front();
                              int hitCardVal = (int)cardChar-'0';
                              dealerCardTotal += hitCardVal;
                              if (dealerCardTotal > 21) {
                                 break;
                              }
                           }
                           //dealer sticks
                           else {
                              break;
                           }
                        }
                        
                        //Once both players have finished
                        std::string clientScore = req.substr (5,req.size());
                        std::string client2Score = req2.substr (5,req.size());
                        int clientScoreValue = std::stoi (clientScore);
                        int client2ScoreValue = std::stoi (client2Score);
                        
                        //Send Client 1 results
                        if (clientScoreValue < 21 && dealerCardTotal < 21) {
                           
                           if (clientScoreValue > dealerCardTotal) {
                              std::string sendWinner = "winner,";
                              sendWinner.append(std::to_string(dealerCardTotal));
                              sendWinner.append(",\n");
                              ByteArray sendWinnerMsg(sendWinner);
                              socket.Write(sendWinnerMsg);
                           }
                           else if (clientScoreValue < dealerCardTotal) {
                              std::string sendLoser = "loser,";
                              sendLoser.append(std::to_string(dealerCardTotal));
                              sendLoser.append(",\n");
                              ByteArray sendLoserMsg(sendLoser);
                              socket.Write(sendLoserMsg);
                           }
                        }
                        else if (clientScoreValue > 21 && dealerCardTotal < 21) {
                           std::string sendLoser = "loser,";
                           sendLoser.append(std::to_string(dealerCardTotal));
                           sendLoser.append(",\n");
                           ByteArray sendLoserMsg(sendLoser);
                           socket.Write(sendLoserMsg);
                        }
                        else  {
                           std::string sendWinner = "winner,";
                           sendWinner.append(std::to_string(dealerCardTotal));
                           sendWinner.append(",\n");
                           ByteArray sendWinnerMsg(sendWinner);
                           socket.Write(sendWinnerMsg);
                        }
                        
                        //Send Client 2 results
                        if (client2ScoreValue < 21 && dealerCardTotal < 21) {
                           
                           if (client2ScoreValue > dealerCardTotal) {
                              std::string sendWinner = "winner,";
                              sendWinner.append(std::to_string(dealerCardTotal));
                              sendWinner.append(",\n");
                              ByteArray sendWinnerMsg(sendWinner);
                              otherSocket.Write(sendWinnerMsg);
                           }
                           else if (client2ScoreValue < dealerCardTotal) {
                              std::string sendLoser = "loser,";
                              sendLoser.append(std::to_string(dealerCardTotal));
                              sendLoser.append(",\n");
                              ByteArray sendLoserMsg(sendLoser);
                              otherSocket.Write(sendLoserMsg);
                           }
                        }
                        else if (client2ScoreValue > 21 && dealerCardTotal < 21) {
                           std::string sendLoser = "loser,";
                           sendLoser.append(std::to_string(dealerCardTotal));
                           sendLoser.append(",\n");
                           ByteArray sendLoserMsg(sendLoser);
                           otherSocket.Write(sendLoserMsg);
                        }
                        else  {
                           std::string sendWinner = "winner,";
                           sendWinner.append(std::to_string(dealerCardTotal));
                           sendWinner.append(",\n");
                           ByteArray sendWinnerMsg(sendWinner);
                           otherSocket.Write(sendWinnerMsg);
                        }
                        //End of winner declaration
                        
                        
                     }
                  }
               }
               
            }
         }
      }
   return 200;
}

~Connection() {
   //Inform client that server has shut down
   std::string serverFailed = "failed, The server has quit.,\n";
   ByteArray serverFailedMsg(serverFailed);
   try {
      socket.Write(serverFailedMsg);
   }
   catch (...) {
      std::cout<<"Client one has already exited."<<std::endl;
   }
   try {
      otherSocket.Write(serverFailedMsg);
   }
   catch (...) {
      std::cout<<"Client two has already exited."<<std::endl;
   }
   std::cout<<"A connection has been terminated successfully."<<std::endl;
}
};

class ServerThread: public Thread {
private:
   //List to hold a reference to all threads
   std::vector<Connection*> threads;
   
   //Socket that will be listening
   SocketServer* socket;
   
public:
   //Constructor
   ServerThread() {}
   
   long ThreadMain(void) {
      
      //Semaphore to wait for a second connection
      //sem_t* startSem =
      
      //socketserver to listen for a connection attempt
      socket = new SocketServer(2999);
      
      try
      {
         //Stay open for client connections
         while (1) {
            //Block for a connection attempt
            Socket openSocket = socket->Accept();
            std::string otherTurn = "waiting,Waiting for the other player...,\n";
            ByteArray otherTurnMsg(otherTurn);
            openSocket.Write(otherTurnMsg);
            Socket otherSocket = socket->Accept();
            
            //Once a connection has been attempted, open a new thread to accept the connection
            Connection* newThread = new Connection(openSocket, otherSocket);
            
            //add reference to list of thread pointers
            threads.push_back(newThread);
            
            //Start the new connection
            newThread->Start();
         }
      }
      catch(...)
      {
         std::cout << "The server has failed." << std::endl;
      }
      
      return 200;
   }
   
   ~ServerThread() {
      try {
         //delete all threads
         socket->Shutdown();
         //delete socket;
         for (int i(0); i<threads.size(); i++) {
            delete threads[i];
         }
         std::cout<<"The server thread has been deleted, server ended."<<std::endl;
      }
      catch(...) {
         
      }
   }
};

int main(void) {
   ServerThread server;
   server.Start();
   std::string input ="";
   while (input!="done") {
      getline(std::cin, input);
   }
}
