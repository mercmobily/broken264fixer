/*
 * convert weird *.264 file to mp4 and wav
 * -raspo666 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* WAV */
typedef struct wav_header
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth; // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header;

/**
    Main
 **/
int main(int ac, char **argValue)
{
    FILE *rawFile,*videoFile,*audioFile;
    FILE *videoTSFile,*audioTSFile;

    int firstVideoTS = -1;
    int firstAudioTS = -1;


    wav_header ahead;

    char *pFullBuffer,*p, ccode[5], *pFilename, *pString;
    int count,bufsiz,abytes;
    int len = 0;

    if(ac != 2)
    {
        exit(printf("use %s <file> \n",argValue[0]));
    }

    if(!(rawFile = fopen(argValue[1],"rb")))
    {
        exit(printf("cannot open %s for reading.\n",argValue[1]));
    }

    /* try to allocate a pFullBuffer size of infile */
    fseek(rawFile,0,SEEK_END);
    bufsiz = ftell(rawFile);
    rewind(rawFile);
    if(!(pFullBuffer = ( char * ) malloc(bufsiz)))
    {
        exit(printf("unable to allocate %d bytes\n",bufsiz));
    }
    p = pFullBuffer;

    /* open output */


    if(!(pFilename = (char*) malloc(strlen(argValue[1])+5)))
    {
        exit(printf("no mem for strings\n"));
    }

    //    Input Raw file
    pString = strstr(argValue[1],".264");
    if(pString != NULL)
    {
        *pString = '\0';
    }
    pString = argValue[1];

    //    Video File
    sprintf(pFilename,"%s.h264",pString);
    if(!(videoFile = fopen(pFilename,"wb")))
    {
        exit(printf("cannot open %s for writing.\n",argValue[2]));
    }

    //    Video Timestamp
    sprintf(pFilename,"%s.video.ts.txt",pString);
    if(!(videoTSFile = fopen(pFilename,"w")))
    {
        exit(printf("cannot open %s for writing.\n",argValue[2]));
    }
    fprintf ( videoTSFile, "# timestamp format v2\n" );

    //    Wav File
    sprintf(pFilename,"%s.wav",pString);
    if(!(audioFile = fopen(pFilename,"wb")))
    {
        exit(printf("cannot open %s for writing.\n",pFilename));
    }

    //    Wav Timestamp File
    sprintf(pFilename,"%s.audio.ts.txt",pString);
    if(!(audioTSFile = fopen(pFilename,"w")))
    {
        exit(printf("cannot open %s for writing.\n",pFilename));
    }
    fprintf ( audioTSFile, "# timestamp format v2\n" );

    fwrite(&ahead,sizeof(wav_header),1,audioFile);

    /* get data */
    count = fread(pFullBuffer,1,bufsiz,rawFile);
    abytes = 0;
    if(count != bufsiz)
    {
        printf("could only read %d of %d bytes\n",count,bufsiz);
        exit(0);
    }

    /* Throw first 0x10 bytes of garbage/fileheader plus first videoheader */
    // HXVS
    memset(ccode,0,sizeof(ccode));
    memcpy(ccode, p, 4);
    if( ! strncmp((char *)ccode,"HXVS",4) == 0 )
    {
        exit(printf("No HXVS.\n"));
    }
    p    += 4;

    //    ??
    p    += 4;

    // Duration
    int duration;
    memcpy(&duration, p, sizeof(int));
    printf("Duration %d\n", duration);

    p    += 4;

    //    ??
    p     += 4;

    while(p - pFullBuffer < bufsiz)
    {
        //    Code
        memset(ccode,0,sizeof(ccode));
        memcpy(ccode, p, 4);
        p += 4;

        //    The Length
        memcpy(&len, p, sizeof(int));
        p += 4;

        //
        if( strncmp((char *)ccode,"HXAF",4) == 0 )
        {
            //     Timestamp
            int ts;
            memcpy(&ts,p,sizeof(int));
            if ( firstAudioTS == -1 )
            {
                firstAudioTS = ts;
            }
            fprintf ( audioTSFile, "%d\n", ts - firstAudioTS );
            p += 4;

            //    ??
            p += 4;

            //    Audio
            p += 4; // {0x0001, 0x5000} whatever that means, it must go, it's no audio
            len -= 4;
            // printf("code: HXAF audio  %d bytes\n",len);
            fwrite(p,1,len,audioFile);
            p += len;
            abytes += len;
            continue;
        }
        if( strncmp((char *)ccode,"HXVF",4) == 0 )
        {
            //    Timestamp
            int ts;
            memcpy(&ts,p,sizeof(int));
            if ( firstVideoTS == -1 )
            {
                firstVideoTS = ts;
            }

            int NalUnit = p [ 12 ] & 0x1f;
            //    Do not set TimeStampp for SPS...
            if ( NalUnit != 0x6 && NalUnit != 0x7 && NalUnit != 0x8 )
            {
                fprintf ( videoTSFile, "%d\n", ts - firstVideoTS );
            }
            p += 4;

            //    ??
            p += 4;

            //
            fwrite(p,1,len,videoFile);
            p += len;
            continue;
        }
        else if(!(strncmp((char *)ccode,"HXFI",4)))
        {
            printf("HXFI End if File\n");
            break; /* some sort of table follows */
        }
        else
        {
            printf("Unknown Code\n");
        }
    }

    /* wav header */

    strncpy(ahead.riff_header,"RIFF",4);
    strncpy(ahead.wave_header,"WAVE",4);
    strncpy(ahead.fmt_header,"fmt ",4);
    strncpy(ahead.data_header,"data",4);

    ahead.fmt_chunk_size = 16;
    ahead.audio_format = 0x6;
    ahead.num_channels = 1;
    ahead.sample_rate = 8000;
    ahead.byte_rate = 8000; // 16 ??
    ahead.sample_alignment = 2;
    ahead.bit_depth = 16;
    ahead.data_bytes = abytes;
    ahead.wav_size = abytes + sizeof(wav_header) - 8;
    fseek(audioFile,0,SEEK_SET);
    fwrite(&ahead,sizeof(wav_header),1,audioFile);

    free(pFullBuffer);
    fclose(rawFile);
    fclose(videoFile);
    fclose(audioFile);
    fclose(videoTSFile);
    fclose(audioTSFile);
    exit(0);

} 
