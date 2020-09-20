#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define OS_TYPE_MACOS   (0)
#define OS_TYPE_M5STACK (1)
#define OS_TYPE          OS_TYPE_MACOS

#if (OS_TYPE == OS_TYPE_MACOS)
#include <stdlib.h>
#endif


#define PUNCHED_CARD_FILENM     ("punched_card.txt")
#define PUNCHED_CARD_LENGTH     (128)
#define PUNCHED_CARD_SOUND_NUM   (15)
#define PUNCHED_CARD_ON_MARK    ('O')
#define PUNCHED_CARD_OFF_MARK   ('-')
#define SOUND_INTERVAL_MS       (40)
#define SOUND_FILE_DIR          ("wav")

/* low -> high */
char snd_filenm_ary[PUNCHED_CARD_SOUND_NUM+1][6+1] = {
    "nn.wav",
    "c3.wav",
    "d3.wav",
    "e3.wav",
    "f3.wav",
    "g3.wav",
    "a4.wav",
    "b4.wav",
    "c4.wav",
    "d4.wav",
    "e4.wav",
    "f4.wav",
    "g4.wav",
    "a5.wav",
    "b5.wav",
    "c5.wav"
};

int play_sound(char * filename);


int main(){

    FILE *fp;
    unsigned char pc_str_ary[PUNCHED_CARD_SOUND_NUM][PUNCHED_CARD_LENGTH];
    unsigned char pc_str_buf[PUNCHED_CARD_LENGTH + 2]; // (改行コード分)+2
    unsigned int len_i = 0;
    unsigned char sndnum_i = 0;
    unsigned char snd_id = 0;
    int ret = 0;

    fp = fopen(PUNCHED_CARD_FILENM,"r");
    if(fp==NULL)
    {
        printf("[error] file open error\n");
        return 1;
    }

    for (sndnum_i = 0; sndnum_i < PUNCHED_CARD_SOUND_NUM; sndnum_i++)
    {
        if ( fgets(pc_str_buf, sizeof(pc_str_buf), fp) == NULL )
        {
            printf("[error] file format error\n");
            return 1;
        }
        else
        {
            // printf("%s¥n", pc_str_buf);
            memcpy(pc_str_ary[sndnum_i], pc_str_buf, PUNCHED_CARD_LENGTH);
        }
    }
    fclose(fp);

    for (len_i = 0; len_i < PUNCHED_CARD_LENGTH; len_i++)
    {
        snd_id = 0;
        for (sndnum_i = 0; sndnum_i < PUNCHED_CARD_SOUND_NUM; sndnum_i++)
        {
            if (pc_str_ary[sndnum_i][len_i] == PUNCHED_CARD_ON_MARK)
            {
                snd_id = PUNCHED_CARD_SOUND_NUM - sndnum_i;  /* 15 - 1 */
            }
        }

        // play wav
        // printf("%d temp %d ontei ", len_i, snd_id);

        ret = play_sound(snd_filenm_ary[snd_id]);
        if (ret != 0) {
            printf("[error] play sound error\n");
            return 1;
        }

        // wait  SOUND_INTERVAL_MS
        usleep(SOUND_INTERVAL_MS * 1000);
    }

    return 0;
}

int play_sound(char * filename) {

#if (OS_TYPE == OS_TYPE_MACOS)
    char str_cmd[128] = "afplay ";
    strcat(str_cmd, SOUND_FILE_DIR);
    strcat(str_cmd, "/");    
    strcat(str_cmd, filename);
    system(str_cmd);
#else
    /* undefined */
#endif

}