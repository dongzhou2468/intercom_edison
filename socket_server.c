/*
  intel edison speaks to intercom app
  install alsa-lib-dev and configure default pcm
  compile: gcc -o socket_server socket_server.c -lasound -lpthread
  run: ./socket_server $app_ip
*/
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

#include <pthread.h>
#include <unistd.h>

int buffer_frames = 1024 * 4;		//origin 128, better to be multiple of 8?                                                                                       
unsigned int rate = 44100;                                                                                      
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
int err;
volatile int read_i = 0;

void initialize(snd_pcm_t *capture_handle, snd_pcm_hw_params_t *hw_params, int chann);

struct dest_ip
{
  char *ip;
};

void capture_t(void *argv)
{
  char *buffer;
  snd_pcm_t *capture_handle;                                                           
  snd_pcm_hw_params_t *hw_params_record;                                                 
                                                                                         
  if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device for record: %s (%s)\n",
             "default",                                  
             snd_strerror (err));                        
    exit (1);                                                                         
  }                                                                                      
  fprintf(stdout, "audio interface for record opened\n");                                
  initialize(capture_handle, hw_params_record, 1);

  buffer = (char *)malloc(2048);
  fprintf(stdout, "buffer allocated, size: %d, %d\n", sizeof(buffer), sizeof(buffer[0]));

  struct dest_ip *myip = (struct dest_ip *)argv;

  struct sockaddr_in server_addr; 
  bzero(&server_addr, sizeof(server_addr)); 
  server_addr.sin_family = AF_INET; 
  server_addr.sin_addr.s_addr = inet_addr(myip->ip); 
  server_addr.sin_port = htons(4142); 
  
  int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(client_socket_fd < 0) 
  { 
    printf("Create Socket Failed...\n"); 
    exit(1); 
  }

  while(read_i++ < 1500)
  {
    //printf("starting reading\n");                                                                             
    if ((err = snd_pcm_readi (capture_handle, buffer, 1024)) != 1024) {                     
      printf ("read from audio interface failed %d\n", read_i);                                                    
               //err, snd_strerror (err));                                                                    
      if(err == -EBADFD)                                                                                      
        printf("PCM is not in the right state\n");                                                            
      else if(err == -EPIPE)                                                                                  
        printf("an underrun occurred\n");                                                                     
      else if(err == -ESTRPIPE)                                                                               
        printf("a suspend event occurred\n");                                                                 
      //exit (1);                                                                                             
      snd_pcm_prepare(capture_handle);
    }
    if(sendto(client_socket_fd, buffer, 2048, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    { 
      printf("socket send failed...\n"); 
      exit(1); 
    } 
  }
  free(buffer);
   
}

main (int argc, char *argv[])
{
  int i;
  unsigned char *temp;

  FILE *recFile;
  recFile = fopen("record/socket_server.raw", "wb");

  temp = (unsigned char *)malloc(buffer_frames * 2);

  snd_pcm_t *play_handle;
  snd_pcm_hw_params_t *hw_params_play;

  if ((err = snd_pcm_open (&play_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {                           
    fprintf (stderr, "cannot open audio device for playing: %s (%s)\n",                                                        
             "default",                                                                                             
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "audio interface for playing opened\n");                                                                    
  initialize(play_handle, hw_params_play, 1);

  /* UDP Socket */
  struct sockaddr_in server_addr; 
  bzero(&server_addr, sizeof(server_addr)); 
  server_addr.sin_family = AF_INET; 
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_addr.sin_port = htons(4142); 
  
  int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(server_socket_fd == -1) 
  { 
    printf("Create Socket Failed...\n"); 
    exit(1); 
  } 
  if(-1 == (bind(server_socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)))) 
  { 
    printf("Server Bind Failed...\n"); 
    exit(1); 
  } 

  struct sockaddr_in client_addr; 
  socklen_t client_addr_length = sizeof(client_addr); 

  struct dest_ip myip;
  myip.ip = argv[1];
  pthread_t capture_thread;                                                                               
  recvfrom(server_socket_fd, temp, buffer_frames * 2, 0, (struct sockaddr*)&client_addr, &client_addr_length);
  if(pthread_create(&capture_thread, NULL, (void *)capture_t, (void *)&myip) != 0)                                 
      printf("create capture thread failed...\n");

  i = 0;
  while(i++ < 1500)
  {
    //printf("waiting for data...\n");
    if(recvfrom(server_socket_fd, temp, buffer_frames * 2, 0, (struct sockaddr*)&client_addr, &client_addr_length) == -1)
      printf("receiving failed...\n");
    //fwrite(temp, sizeof(temp[0]), buffer_frames * 2, recFile);

    //stop
    if(atoi(temp) == 1010)
    {
      printf("stop received!\n");
      read_i = 1500;
      i = 1500;
      sleep(1);
      sendto(server_socket_fd, temp, 4, 0, (struct sockaddr*)&client_addr, client_addr_length);
    }

    //printf("starting writing\n");                                               
    if ((err = snd_pcm_writei (play_handle, temp, buffer_frames)) != buffer_frames) {
      printf("write to audio interface failed %d\n", i);                        
               //err, snd_strerror (err));                                      
      /*if(err == -EBADFD)                                                        
        printf("PCM is not in the right state\n");                              
      else if(err == -EPIPE)                                                    
        printf("an underrun occurred\n");                                       
      else if(err == -ESTRPIPE)                                                 
        printf("a suspend event occurred\n");*/                                  
      //exit (1);                                                               
      snd_pcm_prepare(play_handle);
    }
  }

  pthread_join(capture_thread, NULL);

  fclose(recFile);
  fprintf(stdout, "buffer freed\n");

  //snd_pcm_close (capture_handle);
  snd_pcm_close (play_handle);
  fprintf(stdout, "audio interface closed\n");

  free(temp);                                                                   
  close(server_socket_fd);
  exit (0);
}

void initialize(snd_pcm_t *capture_handle, snd_pcm_hw_params_t *hw_params, int chann)
{
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {                                                        
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",                                       
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params allocated\n");                                                                       
                                                                                                                  
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {                                            
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",                                     
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params initialized\n");                                                                     
                                                                                                                  
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {      
    fprintf (stderr, "cannot set access type (%s)\n",                                                             
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params access setted\n");                                                                   
                                                                                                                  
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {                             
    fprintf (stderr, "cannot set sample format (%s)\n",                                                           
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params format setted\n");                                                                   
                                                                                                                  
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {                        
    fprintf (stderr, "cannot set sample rate (%s)\n",                                                             
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params rate setted\n");                                                                     
                                                                                                                  
  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, chann)) < 0) {                                
    fprintf (stderr, "cannot set channel count (%s)\n",                                                           
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params channels setted\n");

  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {                                                
    fprintf (stderr, "cannot set parameters (%s)\n",                                                              
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "hw_params setted\n");

  snd_pcm_hw_params_free (hw_params);                                                                             
  fprintf(stdout, "hw_params freed\n");

  if ((err = snd_pcm_prepare (capture_handle)) < 0) {                                                             
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",                                             
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "audio interface prepared\n");  
}