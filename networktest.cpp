
#include <unistd.h>
#include "string.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <vector.h>
#define SERVER_PORT  12345

#define TRUE             1
#define FALSE            0

void 

int main (int argc, char *argv[])
{
   int    i, len, rc, on = 1;
   
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   char   buffer[80];
   struct sockaddr_in6 addr;
   fd_set              master_set, working_set;


   listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }


   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }
    rc = fcntl(listen_sd, F_SETFL, O_NONBLOCK);
   if (rc < 0)
   {
      perror("fcntl() failed");
      close(listen_sd);
      exit(-1);
   }
   memset(&addr, 0, sizeof(addr));
   addr.sin6_family      = AF_INET6;
   memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
   addr.sin6_port        = htons(SERVER_PORT);
   rc = bind(listen_sd,
             (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }


   rc = listen(listen_sd, 10);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }


   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);




   while (end_server == FALSE)
   {
      memcpy(&working_set, &master_set, sizeof(master_set));

      std::cout <<"Waiting on select()..." << std::endl;
      rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);

      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

      if (rc == 0)
      {
         std::cout <<"  select() timed out.  End program." <<  std::endl;
         break;
      }


      desc_ready = rc;
      for (i=3; i <= max_sd  &&  desc_ready > 0; ++i)
      {
        if (FD_ISSET(i, &working_set))
        {
            desc_ready -= 1;

            if (i == listen_sd)
            {
               std::cout <<"  Listening socket is readable" << std::endl;
               new_sd = 0;

                  new_sd = accept(listen_sd, NULL, NULL);
                              
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                  }
                  else
                  {
                     std::cout <<"  New incoming connection - " << new_sd << std::endl;
                     FD_SET(new_sd, &master_set);
                     if (new_sd > max_sd)
                        max_sd = new_sd;
                  }
                
            }

            else
            {
               std::cout <<"  Descriptor is readable " << i << std::endl;
               close_conn = FALSE;
               while (TRUE)
               {
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }

                  if (rc == 0)
                  {
                     std::cout <<"  Connection closed" << std::endl;
                     close_conn = TRUE;
                     break;
                  }

                  len = rc;
                  std::cout <<" bytes received " << len << std::endl;


                  rc = send(i, buffer, len, 0);
                  
                  if (rc < 0)
                  {
                     perror("  send() failed");
                     close_conn = TRUE;
                     break;
                  }

               }

               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
            } 
         }
      } 

   }
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
}