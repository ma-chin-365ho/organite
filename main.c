#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define OS_TYPE_MACOS   (0)
#define OS_TYPE_M5STACK (1)
#define OS_TYPE          OS_TYPE_M5STACK

#if (OS_TYPE == OS_TYPE_MACOS)
#include <stdlib.h>
#elif (OS_TYPE == OS_TYPE_M5STACK)
#include <M5Stack.h>
#include "FS.h"
#include "SPIFFS.h"
#endif

#define PUNCHED_CARD_FILENM     ("punched_card.txt")
#define PUNCHED_CARD_LENGTH     (128)
#define PUNCHED_CARD_SOUND_NUM   (15)
#define PUNCHED_CARD_ON_MARK    ('O')
#define PUNCHED_CARD_OFF_MARK   ('-')
#define SOUND_DURATION_MS       (200)
#define SOUND_INTERVAL_MS       (40)
#define SOUND_FILE_DIR          ("wav")

#if (OS_TYPE == OS_TYPE_MACOS)
#elif (OS_TYPE == OS_TYPE_M5STACK)
#define FORMAT_SPIFFS_IF_FAILED true
#endif

unsigned char pc_str_ary[PUNCHED_CARD_SOUND_NUM][PUNCHED_CARD_LENGTH];

#if (OS_TYPE == OS_TYPE_MACOS)
#elif (OS_TYPE == OS_TYPE_M5STACK)
int g_len_i = -1;
#endif

int play_sound(unsigned char snd_id);
int play_punched_card ();
int load_punched_card ();

#if (OS_TYPE == OS_TYPE_MACOS)
int main(){

    if (load_punched_card() != 0) {
        return 1;
    }
    if (play_punched_card() != 0) {
        return 1;
    }
    return 0;
}
#elif (OS_TYPE == OS_TYPE_M5STACK)
void setup() {
  // Initialize the M5Stack object
  M5.begin();
  
  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();
  
  Serial.begin(115200);

  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  load_punched_card();

  M5.Lcd.printf("M5Stack Organite:\r\n");
}

void loop() {
  unsigned char snd_id;
  unsigned char sndnum_i = 0;

  if(M5.BtnA.wasPressed()) {
    M5.Lcd.printf("A wasPressed \r\n");
    g_len_i = 0;
    /* ここで呼ぶと鳴動が終わらない事象 */
    // play_punched_card();
#if (0)
    for (int i = 0; i < PUNCHED_CARD_SOUND_NUM; i++){
      for (int j = 0; j < PUNCHED_CARD_LENGTH; j++) {
        Serial.write(pc_str_ary[i][j]);
      }
      Serial.println(1);
    }
#endif
  }

  if (g_len_i >= 0) {
    snd_id = 0;
    for (sndnum_i = 0; sndnum_i < PUNCHED_CARD_SOUND_NUM; sndnum_i++)
    {
        if (pc_str_ary[sndnum_i][g_len_i] == PUNCHED_CARD_ON_MARK)
        {
            snd_id = PUNCHED_CARD_SOUND_NUM - sndnum_i;  /* 15 - 1 */
        }
    }

    play_sound(snd_id);

    g_len_i++;
    if (g_len_i >= PUNCHED_CARD_LENGTH)
    {
        g_len_i = -1;
    }
  }

  M5.update();
}

#endif

int play_sound(unsigned char snd_id) {

#if (OS_TYPE == OS_TYPE_MACOS)
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

    char str_cmd[128] = "afplay ";
    strcat(str_cmd, SOUND_FILE_DIR);
    strcat(str_cmd, "/");    
    strcat(str_cmd, snd_filenm_ary[snd_id]);

    printf(str_cmd);
    system(str_cmd);

#elif (OS_TYPE == OS_TYPE_M5STACK)

    uint16_t snd_freq;
    const uint16_t snd_freq_ary[PUNCHED_CARD_SOUND_NUM+1] = {
        0,
        261,
        293,
        329,
        349,
        391,
        440,
        493,
        523,
        587,
        659,
        698,
        783,
        880,
        987,
        1046
    };
    
    snd_freq = snd_freq_ary[snd_id];
    if (snd_freq == 0)
    {
        usleep(SOUND_DURATION_MS * 1000);
    }
    else
    {
        M5.Speaker.tone(snd_freq_ary[snd_id], SOUND_DURATION_MS);
    } 
    usleep(SOUND_DURATION_MS * 1000);
#endif

    return 0;
}

int play_punched_card () {
    unsigned int len_i = 0;
    unsigned char sndnum_i = 0;
    unsigned char snd_id = 0;
    int ret = 0;

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

        ret = play_sound(snd_id);
        if (ret != 0) {
            printf("[error] play sound error\n");
            return 1;
        }

        // wait  SOUND_INTERVAL_MS
        usleep(SOUND_INTERVAL_MS * 1000);
    }

    return 0;
}

int load_punched_card () {

#if (OS_TYPE == OS_TYPE_MACOS)
    FILE *fp;
    unsigned char sndnum_i = 0;
    unsigned char pc_str_buf[PUNCHED_CARD_LENGTH + 2]; // (改行コード分)+2

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

    return 0;
#elif (OS_TYPE == OS_TYPE_M5STACK)
    const char * path = "/punched_card.txt";
    unsigned char pc_str_buf[PUNCHED_CARD_LENGTH + 1]; // (改行コード分)+1 何故か+1でいい。
    unsigned int len_i = 0;
    unsigned char sndnum_i = 0;

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        return 1;
    }

    for (sndnum_i = 0; sndnum_i < PUNCHED_CARD_SOUND_NUM; sndnum_i++)
    {
        // Serial.write(file.read());
        for (len_i = 0; len_i < sizeof(pc_str_buf); len_i++)
        {
            if (file.available()) {
                pc_str_buf[len_i] = file.read();
            }
        }
        
        memcpy(pc_str_ary[sndnum_i], pc_str_buf, PUNCHED_CARD_LENGTH);
    }
#endif
}