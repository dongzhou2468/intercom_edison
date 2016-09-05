/*
  A Minimal Capture Program
  This program opens an audio interface for capture, configures it for
  stereo, 16 bit, 44.1kHz, interleaved conventional read/write
  access. Then its reads a chunk of random data from it, and exits. It
  isn't meant to be a real program.
  From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html
  Fixes rate and buffer problems
  sudo apt-get install libasound2-dev
  gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int buffer_frames = 128;                                                                                        
unsigned int rate = 22050;                                                                                      
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
int err;

void initialize(snd_pcm_t *capture_handle, snd_pcm_hw_params_t *hw_params, int chann);

main (int argc, char *argv[])
{
  int i;
  char *buffer, *temp;

  FILE *recFile;
  recFile = fopen("record/alsa_record.raw", "wb");

  snd_pcm_t *capture_handle;                                                               
  snd_pcm_hw_params_t *hw_params_record;

  if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {          
    fprintf (stderr, "cannot open audio device for record: %s (%s)\n",                                  
             "default",                                                                            
             snd_strerror (err));                                                           
    exit (1);                                                                               
  }                                                                                         
  fprintf(stdout, "audio interface for record opened\n");
  initialize(capture_handle, hw_params_record, 2);

  buffer = (char *)malloc(128 * snd_pcm_format_width(format) / 8 * 2);		// 512
  fprintf(stdout, "buffer allocated, size: %d, %d\n", sizeof(buffer), sizeof(buffer[0]));
  //temp = malloc(128 * snd_pcm_format_width(format) / 8 * 2);

  snd_pcm_t *play_handle;
  snd_pcm_hw_params_t *hw_params_play;

  if ((err = snd_pcm_open (&play_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {                           
    fprintf (stderr, "cannot open audio device for playing: %s (%s)\n",                                                        
             "default",                                                                                             
             snd_strerror (err));                                                                                 
    exit (1);                                                                                                     
  }                                                                                                               
  fprintf(stdout, "audio interface for playing opened\n");                                                                    
  initialize(play_handle, hw_params_play, 2);

  /*
  snd_pcm_sw_params_t *m_sw_params;
  snd_pcm_sw_params_malloc(&m_sw_params);
  // gets buffersize for control
  snd_pcm_uframes_t chunksize=0;
  // setting software parameters
  if (( err= snd_pcm_sw_params_current(play_handle, m_sw_params))<0)
  {
    printf("audioplayer sw params current failed, errorcode = %s", snd_strerror(err));
    exit (1);
  }
  // start playing when one period has been written
  if (( err= snd_pcm_sw_params_set_start_threshold( play_handle, m_sw_params, chunksize ) )<0)
  {
    printf("audioplayer sw params start threshold failed, errorcode = %s", snd_strerror(err));
    exit (1);
  }
  snd_pcm_uframes_t boundary;
  if (( err= snd_pcm_sw_params_get_boundary( m_sw_params,&boundary))<0)
  {
    printf("audioplayer sw params get boundary failed, errorcode = %s", snd_strerror(err));
    exit (1);
  }
  // disable underrun reporting
  if (( err= snd_pcm_sw_params_set_stop_threshold(play_handle, m_sw_params, boundary ) )<0)
  {
    printf("audioplayer sw params stop threshold failed, errorcode = %s", snd_strerror(err));
    exit (1);
  }
  // play silence when there is an underrun
  if (( err= snd_pcm_sw_params_set_silence_size( play_handle, m_sw_params, boundary ) )<0)
  {
    printf("audioplayer sw params set silence size failed, errorcode = %s", snd_strerror(err));
    exit (1);
  }
  */

  for (i = 0; i < 5000; ++i) {
    printf("starting reading\n");
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
      printf ("read from audio interface failed %d\n", i);
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

    //memcpy(temp, buffer, 256);
    fwrite(buffer, sizeof(short) * 2, buffer_frames, recFile);
    
    printf("starting writing\n");
    if ((err = snd_pcm_writei (play_handle, buffer, buffer_frames)) != buffer_frames) {                         
      printf("write to audio interface failed %d\n", i);                                                
               //err, snd_strerror (err));                                                                          
      if(err == -EBADFD)
	printf("PCM is not in the right state\n");
      else if(err == -EPIPE)
	printf("an underrun occurred\n");
      else if(err == -ESTRPIPE)
	printf("a suspend event occurred\n");
      //exit (1);
      snd_pcm_prepare(play_handle);
    }
    
    /*fprintf(stdout, "read %d done\n", i);
    int j;
    for(j=0; j<sizeof(buffer); j++) {
    	printf("%02X:", buffer[j]);
    }
    printf("\n");*/
  }

  free(buffer);
  fclose(recFile);
  fprintf(stdout, "buffer freed\n");

  snd_pcm_close (capture_handle);
  snd_pcm_close (play_handle);
  fprintf(stdout, "audio interface closed\n");

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
